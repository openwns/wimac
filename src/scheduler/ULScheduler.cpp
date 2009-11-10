/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * email: info@openwns.org
 * www: http://www.openwns.org
 * _____________________________________________________________________________
 *
 * openWNS is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 2 as published by the
 * Free Software Foundation;
 *
 * openWNS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <WIMAC/scheduler/ULScheduler.hpp>
#include <iomanip>

#include <WNS/ldk/fcf/FrameBuilder.hpp>

#include <WIMAC/StationManager.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/Classifier.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/frame/ULMapCollector.hpp>
#include <WIMAC/parameter/PHY.hpp>


STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::scheduler::SSULScheduler,
    wimac::scheduler::Interface,
    "wimac.frame.SSULScheduler",
    wimac::FUConfigCreator );

using namespace wimac;
using namespace wimac::scheduler;


struct UplinkBasicConnection :
    public std::unary_function<ConnectionIdentifier::Ptr, bool>
{
    explicit UplinkBasicConnection(int myID): myID_( myID) {}
    bool operator()(const ConnectionIdentifier::Ptr& ptr)
    {
        return ( ptr->connectionType_ == wimac::ConnectionIdentifier::Basic
                 && ptr->subscriberStation_ == myID_);
    }
private:
    int myID_;
};


SSULScheduler::SSULScheduler( wns::ldk::FunctionalUnit* parent, const wns::pyconfig::View& config) :
    wns::Cloneable<SSULScheduler>(),
    scheduler::PDUWatchProvider(parent->getFUN()),
    currentBurstStart_ ( 0.0 ),
    currentBurstEnd_ ( 0.0 ),
    currentPhyModePtr (),
    ulMapRetriever_(0),
    ulMapRetrieverName_( config.get<std::string>("ulMapRetrieverName") ),
    accepting_ ( false ),
    component_(0),
    phyUser_(0),
    connectionManager_(0),
    receptor_(0)
{
}

void SSULScheduler::setFUN(wns::ldk::fun::FUN* fun)
{

    assureType(fun->getLayer(), wimac::Component*);
    component_ = dynamic_cast<wimac::Component*>(fun->getLayer());

    ulMapRetriever_ =
        fun->findFriend<wimac::frame::ULMapCollector*>(ulMapRetrieverName_);
    assure( ulMapRetriever_, "ulmapcollector not of type ULMapCollector");

    friends_.classifier_ =
        fun->findFriend<wimac::ConnectionClassifier*>("classifier");
    assure( friends_.classifier_, "classifier not of type ConnectionClassifier");

    phyUser_ =
        fun->findFriend<wimac::PhyUser*>("phyUser");
    assure( phyUser_, "PhyUser not of type wimac::PhyUser");

    connectionManager_ =
        fun->getLayer()->getManagementService<service::ConnectionManager>("connectionManager");
    assure( connectionManager_, "Layer must have connection manager" );

}

void SSULScheduler::deliverSchedule(wns::ldk::Connector* connector)
{
    LOG_INFO( "ul-scheduler in ", component_->getName(), " woke up" );
    currentBurstStart_ =
        ulMapRetriever_->getBurstStart();

    currentBurstEnd_ =
        ulMapRetriever_->getBurstEnd();

    currentPhyModePtr =
        (ulMapRetriever_->getPhyMode());
    assureNotNull(currentPhyModePtr.getPtr());

    phaseStartTime_ =
        wns::simulator::getEventScheduler()->getTime();


    LOG_INFO( component_->getName(), " has ",
              ulMapRetriever_->getBurstEnd() - ulMapRetriever_->getBurstStart(),
              "s to schedule starting at offset ", ulMapRetriever_->getBurstStart(),
              " using PhyMode ", currentPhyModePtr->getString());

    accepting_ = true;

    receptor_->wakeup();

    accepting_ = false;

    LOG_INFO( component_->getName(), " scheduled ",
              compounds_.size(), " compounds" );

    //mark first PDU for measuring in PhyUser
    if(!compounds_.empty())
    {
        wns::ldk::CompoundPtr pdu = *compounds_.begin();
        wimac::PhyUserCommand* phyCommand = dynamic_cast<wimac::PhyUserCommand*>(
            component_->getFUN()->getProxy()->getCommand( pdu->getCommandPool(), phyUser_ ) );
        phyCommand->peer.measureInterference_ = true;
    }

    while ( !compounds_.empty() )
    {
        connector->
            getAcceptor( compounds_.front() )->sendData( compounds_.front() );
        compounds_.pop_front();
    }
}

wns::simulator::Time SSULScheduler::getCurrentDuration() const
{
    return ulMapRetriever_->getULPhaseDuration();
}

bool SSULScheduler::doIsAccepting( const wns::ldk::CompoundPtr& compound ) const
{
    if (!accepting_)
        return false;

    if (!ulMapRetriever_->hasUplinkBurst())
        return false;

    wns::simulator::Time compoundDuration =
        getCompoundDuration( compound ) -  Utilities::getComputationalAccuracyFactor();
#ifndef NDEBUG
    wns::simulator::Time burstDuration = ulMapRetriever_->getBurstEnd() - ulMapRetriever_->getBurstStart();
    assure( compoundDuration <= burstDuration, component_->getName()
            << "SSULScheduler::doIsAccepting: Compound duration (size) isn't schedulable,"
            << " because it overruns the assigned slot duration!"
            << " CompoundDuration = " << std::setprecision(15) << compoundDuration
            << " BurstDuration = " << std::setprecision(15) << burstDuration
            << " difference is: " << compoundDuration - burstDuration );
#endif
    if ( (compoundDuration <= currentBurstEnd_ - currentBurstStart_) )
        return true;

    return false;
}

void SSULScheduler::schedule( const wns::ldk::CompoundPtr& compound )
{
    wns::ldk::ClassifierCommand* clcom =
        friends_.classifier_->getCommand( compound->getCommandPool() );

    Component::StationID destinationID = 0;


    ConnectionIdentifier::Ptr ci;
    ci = connectionManager_->getConnectionWithID( clcom->peer.id );
    assure(ci, "SSULScheduler::doSendData: Get no ConnectionIdentifier from ConnectionManager for that CID!\n");

    if( component_->getStationType() == wns::service::dll::StationTypes::AP() )
    {
        destinationID = ci->subscriberStation_;
    }
    else if( component_->getStationType() == wns::service::dll::StationTypes::UT()
             || component_->getStationType() == wns::service::dll::StationTypes::RUT()
             || component_->getStationType() == wns::service::dll::StationTypes::FRS())
    {
        destinationID = ci->baseStation_;
    }
    else
    {
        assure(0, "unknown station");
    }

    OmniUnicastPhyAccessFunc* func = new OmniUnicastPhyAccessFunc;

    wns::node::Interface* destination =
        TheStationManager::getInstance()->getStationByID( destinationID )->getNode();

    func->destination_ = destination;

    wns::simulator::Time duration = getCompoundDuration( compound );


    func->transmissionStart_ =
        //currentBurstStart_ + Utilities::getComputationalAccuracyFactor() + phaseStartTime_;
        currentBurstStart_ + phaseStartTime_;
    func->transmissionStop_ =
        //currentBurstStart_ + duration + phaseStartTime_;
        currentBurstStart_ + duration -  Utilities::getComputationalAccuracyFactor() + phaseStartTime_;
    currentBurstStart_ += duration;
    assureNotNull(currentPhyModePtr.getPtr());
    func->phyMode_ = currentPhyModePtr;

    PhyUserCommand* phyUserCommand =
        phyUser_->activateCommand( compound->getCommandPool() );

    phyUserCommand->local.pAFunc_.reset( func );
    phyUserCommand->peer.destination_ = destination;
    phyUserCommand->peer.cellID_ = component_->getCellID();
    phyUserCommand->peer.source_ = component_->getNode();
    assureNotNull(currentPhyModePtr.getPtr());
    phyUserCommand->peer.phyModePtr = currentPhyModePtr;
    phyUserCommand->peer.estimatedCandI_ = ulMapRetriever_->getEstimatedCandI();
    phyUserCommand->magic.sourceComponent_ = component_;
    LOG_INFO("setting destination of compound to: ",
             TheStationManager::getInstance()->
             getStationByID( destinationID )->getName() );

    this->pduWatch( compound );  // Watch for special compounds to inform its observer

    compounds_.push_back( compound );
}

wns::simulator::Time
SSULScheduler::getCompoundDuration(const wns::ldk::CompoundPtr& compound) const
{
    assureNotNull(currentPhyModePtr.getPtr());

    double dataRate =
        currentPhyModePtr->getDataRate();
    Bit compoundSize =
        compound->getLengthInBits();// + getFrameBuilder()->getOpcodeSize();

    return compoundSize / dataRate;
}
