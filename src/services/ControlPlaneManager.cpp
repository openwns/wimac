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


/**
 * @file
 * @author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/services/ControlPlaneManager.hpp>

#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/utils.hpp>

#include <WIMAC/PhyUser.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/services/FUReseter.hpp>
#include <WIMAC/services/scanningStrategy/Plain.hpp>

using namespace wimac::service;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::ControlPlaneManagerSS,
    wns::ldk::ManagementServiceInterface,
    "wimac.services.ControlPlaneManagerSS",
    wns::ldk::MSRConfigCreator);


ControlPlaneManagerSS::ControlPlaneManagerSS( wns::ldk::ManagementServiceRegistry* msr,
                                              const wns::pyconfig::View& config ) :
    ManagementService( msr ),
    VersusInterface(),
    AssociatingCallBackInterface(),
    DissociatingCallBackInterface(),
    EventSubject(getMSR()->getLayer()->getName()),
    state_(Dissociated),
    qosCategory_(ConnectionIdentifier::NoQoS),
    ackBaseStations_(),
    scanningStrategy_(NULL),
    startTimeHO_(0),
    handover_(false),
    failure_(0.0),
    probeAssociatedToContextProvider_("MAC.UTAssocitedToAP", 0),
    probeQoSCategoryContextProvider_("MAC.UTQoSCategory", 0),
    associating_(new service::Associating(
                     dynamic_cast<Component*>( getMSR()->getLayer() ),
                     this, config) ),
    dissociating_(new service::Dissociating(
                      dynamic_cast<Component*>( getMSR()->getLayer() ),
                      this, config) ),
    strategyHandover_(NULL),
    strategyBestStation_(NULL),
    configScanningStrategyInitial_(config.getView("scanningStrategyInitial")),
    configScanningStrategyMain_(config.getView("scanningStrategyMain")),
    stationsToScan_(scanningStrategy::Interface::Stations()),
    friends_()
{
    // Create Handover Strategy
    strategyHandover_ = handoverStrategy::Factory::creator(
        config.getView("strategyHandover").get<std::string>("__plugin__") )
        ->create( config.getView("strategyHandover") );
    strategyBestStation_= handoverStrategy::Factory::creator(
        config.getView("strategyBestStation").get<std::string>("__plugin__") )
        ->create( config.getView("strategyBestStation") );

    // Create Probes
    wns::probe::bus::ContextProviderCollection localcpc(
        &dynamic_cast<Component*>( getMSR()->getLayer() )
        ->getFUN()->getLayer()->getContextProviderCollection());

    localcpc.addProvider(wns::probe::bus::contextprovider::Container(
                             &probeAssociatedToContextProvider_));
    localcpc.addProvider(wns::probe::bus::contextprovider::Container(
                             &probeQoSCategoryContextProvider_));

    probeHandoverDuration_
        = wns::probe::bus::collector( localcpc,
                                      config, "handoverDurationProbeName");
    probeFailure_
        = wns::probe::bus::collector( localcpc,
                                      config, "handoverFailureProbeName");

    double minFrequency = config.get<double>("minFrequency");
    double bandwidth = config.get<double>("bandwidth");
    double bandwidthMakeshift = config.get<double>("bandwidthMakeshift");
    int subCarriers = config.get<int>("subCarriers");
    int numberOfChannels = config.get<int>("numberOfChannelsToScan");


    for( int i = 0; i != numberOfChannels; i++)
    {
        scanningStrategy::Interface::Station station;
        station.id = 0;
        station.tune.frequency = minFrequency + i*bandwidth;
        station.tune.bandwidth = bandwidthMakeshift;
        station.tune.numberOfSubCarrier = subCarriers;
        stationsToScan_.push_back( station );
    }

    tuneSiding_.frequency =  minFrequency + numberOfChannels*bandwidth;
    tuneSiding_.bandwidth = 1;
    tuneSiding_.numberOfSubCarrier = 1;

    friends_.connectionManagerName = config.get<std::string>("connectionManager");
    friends_.phyUserName = config.get<std::string>("phyUser");

    friends_.connectionManager = NULL;
    friends_.phyUser = NULL;
}



void
ControlPlaneManagerSS::start(StationID /*associateTo */, int qosCategory)
{
    assure( state_ == Dissociated,
            "wimac::ControlPlaneManagerSS::start: Object is in wrong state! \n" );

    LOG_INFO( getMSR()->getLayer()->getName(),
              ": calling ControlPlaneManagerSS::start().");

    qosCategory_ = qosCategory;

    // Set owns Traffic QoS category to probe ContextProvider.
    probeQoSCategoryContextProvider_.set(qosCategory_);

    // Do next step
    this->setScanningStrategy(Initial);
    this->doNextStep(InitialScanning);
}

void
ControlPlaneManagerSS::scanningStrategyControlREQ()
{
    if( state_ == Associated )
        this->doNextStep(Scanning);

    else if( state_ == Dissociated )
        this->doNextStep(InitialScanning);
}

void
ControlPlaneManagerSS::scanningStrategyResult(const MeasureValues& measuredValues)
{
    assure( state_ == Scanning || state_ == InitialScanning,
            "wimac::ControlPlaneManagerSS::scanningStrategyResult: Object is in wrong state! \n" );

    LOG_INFO( getMSR()->getLayer()->getName(),
              ": calling ControlPlaneManagerSS::resultScanning().");

    if (state_ == InitialScanning)
        this->stateInitinalScanning(measuredValues);

    else if (state_ == Scanning)
        this->stateScanning(measuredValues);

}

void
ControlPlaneManagerSS::resultAssociating(const bool result, const double failure)
{
    assure( state_ == Associating,
            "wimac::ControlPlaneManagerSS::resultRanging: Object is in wrong state! \n" );

    LOG_INFO( getMSR()->getLayer()->getName(),
              ": calling ControlPlaneManagerSS::resultAssociating: ", result);

    if(result == false)
    { // Ranging failed
        failure_ = failure;
        this->doNextStep(Dissociated);
        return;
    }

    // Do next step
    this->doNextStep(Associated);
}



void
ControlPlaneManagerSS::resultDissociating(
    const handoverStrategy::Interface::Stations ackBaseStations)
{
    assure( state_ == Dissociating,
            "wimac::ControlPlaneManagerSS::resultHandover: Object is in wrong state! \n" );

    LOG_INFO( getMSR()->getLayer()->getName(),
              ": calling ControlPlaneManagerSS::resultDissociating().");

    if(ackBaseStations.empty())
    { // Handover failed
        this->doNextStep(Dissociated);
        return;
    }

    // Do next step
    ackBaseStations_ = ackBaseStations;
    this->doNextStep(Associating);

}



void
ControlPlaneManagerSS::onMSRCreated()
{
    friends_.connectionManager = dynamic_cast<Component*>( getMSR()->getLayer() )
        ->getManagementService<service::ConnectionManager>
        (friends_.connectionManagerName);

    friends_.phyUser = dynamic_cast<Component*>( getMSR()->getLayer() )
        ->getFUN()->findFriend<PhyUser*>(friends_.phyUserName);
}



void
ControlPlaneManagerSS::stateInitinalScanning(const MeasureValues& measureValues)
{
    assure( state_ == InitialScanning,
            "wimac::service::ControlPlaneManagerSS::stateInitinalScanning: Object is in wrong state! \n" );

    if( measureValues.empty() ) // Get no measureValues, do nothing
    {
        this->doNextStep(Dissociated);
        return;
    }


    // get best station
    strategyBestStation_->storeValues(measureValues);
    targetBaseStations_ = strategyBestStation_->decide(0);
    ackBaseStations_ = targetBaseStations_;


    if( ackBaseStations_.empty() ) // Get no targetBaseStation, do nothing
    {
        this->doNextStep(Dissociated);
        return;
    }

    // Do next Step
    this->setScanningStrategy(Main);
    this->doNextStep(Associating);
}


void
ControlPlaneManagerSS::stateScanning(const MeasureValues& measureValues)
{
    assure( state_ == Scanning,
            "wimac::service::ControlPlaneManagerSS::stateScanning: Object is in wrong state! \n" );

    if( measureValues.empty() )
    {
        // Scanning failed
        this->doNextStep(Dissociated);
        return;
    }

    // Store measure values in handoverstrategy
    strategyHandover_->storeValues(measureValues);


    // Handover decission
    ConnectionIdentifier::StationID associateToStation;
    ConnectionIdentifier::StationID ownID;
    ownID = dynamic_cast<Component*>(getMSR()->getLayer())->getID();
    ConnectionIdentifierPtr ci (
        friends_.connectionManager->getBasicConnectionFor( ownID ) );
    assure( ci->valid_,
            "ControlPlaneManagerSS::stateScanning: Didn't get ConnectionIdentifier from ConnectionManager!");
    associateToStation = ci->baseStation_;

    targetBaseStations_ = strategyHandover_->decide(associateToStation);

    if ( targetBaseStations_.empty() )
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Result HandoverDecision: ", "No Handover");

        // Do next Step
        this->doNextStep(Scanning);

    }else
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Result HandoverDecision, target Station:",
                 targetBaseStations_.begin()->id);

        //Probe setting
        handover_ = true;

        // Do next Step
        this->doNextStep(Dissociating);
    }
}



void
ControlPlaneManagerSS::doNextStep(const State state)
{
    if(state == InitialScanning)
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Switch to State: InitialScanning");
        state_ = InitialScanning;
        scanningStrategy_->controlRSP();
        return;

    } else if(state == Scanning)
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Switch to State: Scanning");
        state_ = Scanning;
        scanningStrategy_->controlRSP();
        return;

    } else if(state == Associating)
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Switch to State: Associating");

        // Probe settings
        // Set access point to probe ContextProvider, we're associated/associating to.
        probeAssociatedToContextProvider_.set( (*ackBaseStations_.begin()).id );
        startTimeHO_ = wns::simulator::getEventScheduler()->getTime();

        state_ = Associating;
        associating_->start( *ackBaseStations_.begin(), qosCategory_ );
        return;

    } else if(state == Dissociating)
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Switch to State: Dissociating");

        // NotifyObservers
        if(handover_)
            this->notifyEventObservers("Dissociating");

        state_ = Dissociating;
        dissociating_->start( targetBaseStations_);
        return;

    } else if(state == Associated)
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Switch to State: Associated");

        // Probe writing
        if(handover_ && probeHandoverDuration_)
            probeHandoverDuration_->put( wns::simulator::getEventScheduler()->getTime()
                                         - startTimeHO_);

        // NotifyObservers
        if(handover_)
            this->notifyEventObservers("Associated");

        // Probe setting
        startTimeHO_ = 0;
        handover_ = false;

        state_ = Associated;
        return;

    } else if(state == Dissociated)
    {
        LOG_INFO(getMSR()->getLayer()->getName(),
                 ": Switch to State: Dissociated");

        if(probeFailure_ && (state == Scanning))
            probeFailure_->put(1);

        if(probeFailure_ && (state == Dissociating))
            probeFailure_->put(2);

        if(probeFailure_ && (state == Associating))
            probeFailure_->put(3+failure_);

        if(probeFailure_ && handover_)
            probeFailure_->put(4+failure_);


        // Probe settings
        this->notifyEventObservers("Dissociated");
        probeAssociatedToContextProvider_.set(0);
        startTimeHO_ = 0;
        handover_ = false;
        failure_ = 0;

        // Reset UT: Remove all Connection Idenfifier and set station to siding
        this->setScanningStrategy(Initial);
        friends_.phyUser->startMeasuring();
        friends_.phyUser->setRxFrequency(tuneSiding_);
        friends_.phyUser->setTxFrequency(tuneSiding_);
        friends_.connectionManager->deleteAllConnections();


        state_ =  Dissociated;
        return;
    } else
    {
        assure(0,"wimac::service::ControlPlaneManagerSS::doNextStep: State unknown!");
        return;
    }
}

void
ControlPlaneManagerSS::setScanningStrategy(const ScanningMode mode)
{
    if(scanningStrategy_)
    {
        delete scanningStrategy_;
        scanningStrategy_ = NULL;
    }

    if( mode == Initial )
    {
        scanningStrategy_ = scanningStrategy::Factory::creator(
            configScanningStrategyInitial_.get<std::string>("__plugin__") )
            ->create( this,
                      dynamic_cast<Component*>( getMSR()->getLayer() ),
                      configScanningStrategyInitial_ );

        scanningStrategy_->onFUNCreated();
        scanningStrategy_->setup(stationsToScan_);
    }
    else if( mode == Main )
    {
        scanningStrategy_ = scanningStrategy::Factory::creator(
            configScanningStrategyMain_.get<std::string>("__plugin__") )
            ->create( this,
                      dynamic_cast<Component*>( getMSR()->getLayer() ),
                      configScanningStrategyMain_ );

        scanningStrategy_->onFUNCreated();
        scanningStrategy_->setup(stationsToScan_);
    }
    else
        assure(0,"ControlPlaneManagerSS::setScanningStrategy: Unkown Scanning Mode");
}


