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

#include <WIMAC/frame/DataCollector.hpp>

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/Utilities.hpp>

#include <boost/bind.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::frame::DataCollector,
    wns::ldk::FunctionalUnit,
    "wimac.frame.DataCollector",
    wns::ldk::FUNConfigCreator );

using namespace wimac::frame;


DataCollector::DataCollector(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::fcf::CompoundCollector(config),
    wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>(fun)
{
    if (!config.isNone("txScheduler"))
    {
        std::string txSchedulerName = config.get<std::string>("txScheduler.__plugin__");
        wns::pyconfig::View txSchedulerConfig
            ( config.getView("txScheduler") );
        wimac::scheduler::SchedulerCreator* txSchedulerCreator
            ( wimac::scheduler::SchedulerFactory::creator(txSchedulerName) );
        txScheduler.reset(txSchedulerCreator->create(this, txSchedulerConfig));
    }

    if (!config.isNone("rxScheduler"))
    {
        std::string rxSchedulerName = config.get<std::string>("rxScheduler.__plugin__");
        wns::pyconfig::View rxSchedulerConfig
            ( config.getView("rxScheduler") );
        wimac::scheduler::SchedulerCreator* rxSchedulerCreator
            ( wimac::scheduler::SchedulerFactory::creator(rxSchedulerName) );
        rxScheduler.reset(rxSchedulerCreator->create(this, rxSchedulerConfig));
    }
}

DataCollector::DataCollector(const DataCollector& rhs) :
    wns::ldk::fcf::CompoundCollectorInterface(rhs),
    wns::ldk::CompoundHandlerInterface<FunctionalUnit>(rhs),
    wns::ldk::CommandTypeSpecifierInterface(rhs),
    wns::ldk::HasReceptorInterface(rhs),
    wns::ldk::HasConnectorInterface(rhs),
    wns::ldk::HasDelivererInterface(rhs),
    wns::CloneableInterface(rhs),
    wns::IOutputStreamable(rhs),
    wns::PythonicOutput(rhs),
    wns::ldk::FunctionalUnit(rhs),
    wns::ldk::fcf::CompoundCollector(rhs),
    wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>(rhs),
    wns::ldk::HasConnector<wns::ldk::SingleConnector>(rhs),
    wns::ldk::HasReceptor<wns::ldk::SingleReceptor>(rhs),
    wns::ldk::HasDeliverer<wns::ldk::SingleDeliverer>(rhs),
    wns::Cloneable<wimac::frame::DataCollector>(rhs),
    wns::events::CanTimeout(rhs)
{
    txScheduler.reset(dynamic_cast<wimac::scheduler::Interface*>
                      (dynamic_cast<wns::CloneableInterface*>
                       (rhs.txScheduler.get())->clone()));

    rxScheduler.reset(dynamic_cast<wimac::scheduler::Interface*>
                      (dynamic_cast<wns::CloneableInterface*>
                       (rhs.rxScheduler.get())->clone()));
}

void
DataCollector::onFUNCreated()
{
    if (txScheduler.get())
    {
        txScheduler->setFUN(getFUN());
        txScheduler->setReceptor(getReceptor());
    }
    if (rxScheduler.get())
    {
        rxScheduler->setFUN(getFUN());
        rxScheduler->setReceptor(getReceptor());
    }

    phyUser_ = getFUN()->findFriend<wimac::PhyUser*>("phyUser");
    assure( phyUser_, "PhyUser is not of type wimac::PhyUser");
}

void
DataCollector::doOnData(const wns::ldk::CompoundPtr& compound)
{
    assure(phyUser_, "PhyUser unknown");
    wimac::PhyUserCommand* phyCommand = 
        phyUser_->getCommand(compound->getCommandPool());

    assure(phyCommand, "Cannot extract phyCommand");

    if(phyCommand->magic.schedulingTimeSlot != NULL)
    {    
        rxScheduler->getHARQ()->onTimeSlotReceived(phyCommand->magic.schedulingTimeSlot,
            wns::scheduler::harq::HARQInterface::TimeSlotInfo(
            phyCommand->magic.rxMeasurement,
            0.0,
            phyCommand->local.pAFunc_->subBand_));
    
        if(deliverReceivedEvent != wns::events::scheduler::IEventPtr())
        {
            deliverReceivedEvent = wns::simulator::getEventScheduler()->scheduleDelay(
                boost::bind(&DataCollector::deliverReceived, this), 
                    Utilities::getComputationalAccuracyFactor());
        }
    }
    else
    {
        getDeliverer()->getAcceptor(compound)->onData(compound);
    }
}

void
DataCollector::deliverReceived()
{
    deliverReceivedEvent = wns::events::scheduler::IEventPtr();

    wns::scheduler::harq::HARQInterface::DecodeStatusContainer compounds;
    compounds = rxScheduler->getHARQ()->decode();

    wns::scheduler::harq::HARQInterface::DecodeStatusContainer::iterator it;

    for (it = compounds.begin(); it!=compounds.end();++it)
    {
        if(it->first->harq.successfullyDecoded)
        {
            wns::scheduler::PhysicalResourceBlockVector::iterator itPRB;
            for(itPRB = it->first->physicalResources.begin();
                itPRB != it->first->physicalResources.end();
                ++it)
            {
                wns::scheduler::ScheduledCompoundsList::const_iterator compoundIt;
                // Iterate over all contained compounds
                for (compoundIt = itPRB->scheduledCompoundsBegin();
                    compoundIt != itPRB->scheduledCompoundsEnd();
                    ++compoundIt)
                {
                    getDeliverer()->getAcceptor(compoundIt->compoundPtr)
                        ->onData(compoundIt->compoundPtr);
                }
            }
        }
        it->first->physicalResources.clear();
    }
}

void
DataCollector::doSendData(const wns::ldk::CompoundPtr& compound)
{
    if ( rxScheduler.get() && rxScheduler->isAccepting(compound) )
        rxScheduler->schedule(compound);
    else if ( txScheduler.get() && txScheduler->isAccepting(compound) )
        txScheduler->schedule(compound);
    else
        throw wns::Exception("No scheduler accepts the compound as requested");
}

void
DataCollector::doStartCollection(int)
{
    wimac::scheduler::Interface* sched;
    sched = getCurrentScheduler();
    if(sched != NULL)
    {
        getCurrentScheduler()->setDuration(getMaximumDuration());
        getCurrentScheduler()->startScheduling();
    }
}

void
DataCollector::doStart(int)
{
    wimac::scheduler::Interface* sched;
    sched = getCurrentScheduler();
    if(sched != NULL)
        getCurrentScheduler()->deliverSchedule(getConnector());
    setTimeout( getMaximumDuration() );
}

wns::simulator::Time
DataCollector::getCurrentDuration() const
{
    return getCurrentScheduler()->getDuration();
}

bool
DataCollector::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
    return (txScheduler.get() && txScheduler->isAccepting(compound))
        || ( rxScheduler.get() && rxScheduler->isAccepting(compound) );
}

void
DataCollector::onTimeout()
{
    getFrameBuilder()->finishedPhase( this );
}

void
DataCollector::finishCollection()
{
    wimac::scheduler::Interface* sched;
    sched = getCurrentScheduler();
    if(sched != NULL)
        getCurrentScheduler()->finishScheduling();
}

wimac::scheduler::Interface*
DataCollector::getCurrentScheduler() const
{
    switch (getMode())
    {
    case Sending:
        return txScheduler.get();
    case Receiving:
        return rxScheduler.get();
    default:
        throw wns::Exception("Unknown activation mode in DataCollector");
    }
}


