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


#include <WIMAC/controlplane/Scanning.hpp>
#include <WNS/service/dll/StationTypes.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/Component.hpp>

using namespace wimac::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::controlplane::ScanningBS,
    wns::ldk::FunctionalUnit,
    "wimac.controlplane.ScanningBS",
    wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::controlplane::ScanningSS,
    wns::ldk::FunctionalUnit,
    "wimac.controlplane.ScanningSS",
    wns::ldk::FUNConfigCreator);


Scanning::Scanning( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& ) :
    wns::ldk::CommandTypeSpecifier< ScanningCommand >(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>()
{
}

void
Scanning::doWakeup()
{
    for(std::list<wns::ldk::CompoundPtr>::iterator it = compoundQueue_.begin();
        it != compoundQueue_.end(); )
    {
        wns::ldk::CompoundPtr compound = *it;
        it++;

        // Get ClassifierCommand and corresponding ConnectionIdentifier
        wns::ldk::ClassifierCommand* cCommand;
        cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
            friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

        ConnectionIdentifier::Ptr ci;
        ci = friends_.connectionManager->getConnectionWithID(cCommand->peer.id);

        // If no ConnectionIdentifier exist for this compound, compound is out
        // of date
        if(!ci)
        {
            compoundQueue_.remove(compound);
            continue;
        }

        // if no acceptor is accepting go to next compound in queue
        if ( !getConnector()->hasAcceptor(compound) )
            continue;


        LOG_INFO( getFUN()->getName(), ": sending management message:",
                  getCommand( compound->getCommandPool() )
                  ->peer.managementMessageType );

        // Remove compound from queue and send
        compoundQueue_.remove(compound);
        getConnector()->getAcceptor(compound)->sendData(compound);
    }
}

ScanningBS::ScanningBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Scanning(fun, config),
    wns::Cloneable< ScanningBS >(),
    wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
    scheduler::PDUWatchObserver(),
    doMOB_SCN_REQ_(),
    mob_scn_rspPDUSize_(config.get<Bit>("mob_scn_rspPDUSize"))
{
    friends_.connectionManagerName = config.get<std::string>("connectionManager");
    friends_.connectionClassifierName = config.get<std::string>
        ("connectionClassifier");
    friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");
    friends_.pduWatchProviderName = config.get<std::string>("pduWatchProvider");

    friends_.connectionManager = NULL;
    friends_.connectionClassifier = NULL;
    friends_.newFrameProvider = NULL;
    friends_.pduWatchProvider = NULL;

}

void
ScanningBS::doOnData(const wns::ldk::CompoundPtr& compound)
    // get Management Messages (Compound)
{
    LOG_INFO( getFUN()->getName(),
              ": Receiving management message:",
              getCommand( compound->getCommandPool() )
              ->peer.managementMessageType );

    ScanningCommand* command;
    command = this->getCommand( compound->getCommandPool() );

    if ( command->peer.managementMessageType == MACManagementMessage::MOB_SCN_REQ )
    {
        this->doOnMOB_SCN_REQ(compound);
    }else
    {
        assure(0 ,"ScanningBS::doOnData: managementMessageType is unknown type");
    }

}

void
ScanningBS::onFUNCreated()
{
    assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
           wns::service::dll::StationTypes::UT(),
           "wimac::ScanningBS: Station is not a BS! \n" );

    friends_.connectionManager = getFUN()->getLayer()
        ->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

    friends_.connectionClassifier = getFUN()
        ->findFriend<wns::ldk::FunctionalUnit*>(friends_.connectionClassifierName);

    friends_.newFrameProvider = getFUN()
        ->findFriend<wns::ldk::fcf::NewFrameProvider*>(friends_.newFrameProviderName);

    friends_.pduWatchProvider = getFUN()
        ->findFriend<scheduler::PDUWatchProvider*>(friends_.pduWatchProviderName);


    // Connect to MessageProvider
    friends_.newFrameProvider->attachObserver(this);

    // Connect to PDUWatchProvider
    friends_.pduWatchProvider->attachObserver(this);

}

void
ScanningBS::messageNewFrame()
{
    /********* set Users to not listening ******************************/
    for(std::list<wns::ldk::CommandPool>::iterator it = doMOB_SCN_REQ_.begin();
        it != doMOB_SCN_REQ_.end();)
    {
        ScanningCommand* command;
        command = this->getCommand( &(*it) );
        wns::ldk::ClassifierCommand* cCommand;
        cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>
            ( friends_.connectionClassifier->getCommand( &(*it) ) );

        // get subscriberStation for this compound
        ConnectionIdentifier::StationID utID;
        ConnectionIdentifier::Ptr ci;
        ci = friends_.connectionManager->getConnectionWithID(cCommand->peer.id);

        if(!ci)
        {// MOB_SCN_REQ is outdated
            doMOB_SCN_REQ_.erase(it++);
            continue;
        }

        utID = ci->subscriberStation_;

        // set CI to NotListening
        ConnectionIdentifiers cis =
            friends_.connectionManager->getAllCIForSS(utID);
        for ( ConnectionIdentifiers::iterator it2
                  = cis.begin(); it2 != cis.end(); ++it2 )
        {
            (*it2)->ciNotListening_ = command->peer.mob_scn_rsp.scanDuration;
        }

        friends_.connectionManager->changeConnections(cis);

        doMOB_SCN_REQ_.erase(it++);
    }


    /********* decrease all CI which are not listening *****************/
    friends_.connectionManager->decreaseCINotListening();
}

void
ScanningBS::notifyPDUWatch( wns::ldk::CommandPool commandPool )
{
    ScanningCommand* command;
    command = this->getCommand( &commandPool );
    wns::ldk::ClassifierCommand* cCommand;
    cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>
        ( friends_.connectionClassifier->getCommand( &commandPool ) );


    // This compound isn't the right one
    if(command->peer.managementMessageType != MACManagementMessage::MOB_SCN_RSP )
    {
        LOG_INFO( getFUN()->getName(), ": notifyPDUWatch get wrong PDU.");
        return;
    }

    doMOB_SCN_REQ_.push_back(commandPool);
}

void
ScanningBS::doOnMOB_SCN_REQ(const wns::ldk::CompoundPtr& compound)
{
    assure(this->getCommand(compound->getCommandPool())->peer.managementMessageType
           ==  MACManagementMessage::MOB_SCN_REQ,
           "wimac::ScanningBS::doOnMOB_SCN_REQ: only accept MOB_SCN_REQ!\n");

    ScanningCommand::TransactionID transactionID;
    int pmCID;
    int scanDuration;
    service::scanningStrategy::Interface::Stations stationsToScan;

    pmCID = dynamic_cast<wns::ldk::ClassifierCommand*>
        (friends_.connectionClassifier->getCommand(compound->getCommandPool()))
        ->peer.id;
    assure( friends_.connectionManager->getConnectionWithID(pmCID),
            "ScanningBS::doOnMOB_SCN_REQ: Didn't get ConnectionIdentifier for the CID of that Compound!");
    transactionID = this->getCommand(compound->getCommandPool())
        ->peer.mob_scn_req.transactionID;
    scanDuration = this->getCommand(compound->getCommandPool())
        ->peer.mob_scn_req.scanDuration;
    stationsToScan = this->getCommand(compound->getCommandPool())
        ->peer.mob_scn_req.stationsToScan;


    assure(stationsToScan.size() > 0,
           " ScanningBS::doOnMOB_SCN_REQ: mob_scn_req.stationsToScan no stations are given!\n");
    /*

    No intelligence for the Response Message implemented in the moment!

    */

    // Response
    //Create MOB_SCN_RSP Compound to  send or queue
    wns::ldk::CompoundPtr newCompound;
    newCompound = wns::ldk::CompoundPtr(
        new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

    friends_.connectionClassifier->activateCommand(newCompound->getCommandPool());
    dynamic_cast<wns::ldk::ClassifierCommand*>
        (friends_.connectionClassifier->getCommand( newCompound->getCommandPool() ))
        ->peer.id = pmCID;

    this->activateCommand(newCompound->getCommandPool());
    this->getCommand( newCompound->getCommandPool() )->peer.managementMessageType
        = MACManagementMessage::MOB_SCN_RSP;
    this->getCommand( newCompound->getCommandPool() )->peer.mob_scn_rsp.transactionID
        = transactionID;
    this->getCommand( newCompound->getCommandPool() )->peer.mob_scn_rsp.scanDuration
        = scanDuration;
    this->getCommand( newCompound->getCommandPool())
        ->peer.mob_scn_rsp.stationsToScan = stationsToScan;
    this->getCommand( newCompound->getCommandPool() )->magic.size
        = mob_scn_rspPDUSize_;


    //Send or queue compound
    compoundQueue_.push_back(newCompound);
    doWakeup();

}

ScanningSS::ScanningSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Scanning(fun, config),
    wns::Cloneable<ScanningSS>(),
    wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
    scan_(),
    activeTransactionID_(0),
    highestTransactionID_(0),
    remainTimerWaitingForRSP_(-1),
    callBackInterface_(NULL),
    timerWaitingForRSP_(config.get<ConnectionIdentifier::Frames>
                        ("timerWaitingForRSP")),
    timerBetweenFChange_(config.get<ConnectionIdentifier::Frames>
                         ("timerBetweenFrequencyChange")),
    mob_scn_reqPDUSize_(config.get<Bit>("mob_scn_reqPDUSize"))
{
    scan_.scan = false;
    scan_.stationsToScan = service::scanningStrategy::Interface::Stations();
    scan_.itStationsToScan = scan_.stationsToScan.begin();
    scan_.scanDuration = 0;
    scan_.remainingScanDuration = -1;
    scan_.startTime = 0;
    scan_.stopTime = 0;

    friends_.connectionManagerName = config.get<std::string>("connectionManager");
    friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");
    friends_.connectionClassifierName = config.get<std::string>("connectionClassifier");

    friends_.connectionManager = NULL;
    friends_.newFrameProvider = NULL;
    friends_.connectionClassifier = NULL;
}
void
ScanningSS::start(wimac::service::scanningStrategy::Interface::Stations stationsToScan,
                  ScanningCallBackInterface* callBackInterface)
{
    assure (callBackInterface,
            "wimac::ScanningSS::start: Get no callBackInterface! \n");
    assure (stationsToScan.size() > 0,
            "wimac::ScanningSS::start: No stations are given!\n");

    LOG_INFO( getFUN()->getName(),": calling start().");

    // Don't start Scanning when in use
    if(callBackInterface_)
    {
        LOG_INFO( getFUN()->getName(),": ScanningSS is used.");
        resultScanning( wimac::CIRMeasureInterface::MeasureValues() );
        return;
    }


    callBackInterface_ = callBackInterface;
    scan_.stationsToScan = stationsToScan;
    scan_.scanDuration = (scan_.stationsToScan.size()*timerBetweenFChange_);

    if ( (friends_.connectionManager->getAllBasicConnections()).size() > 0)
    {
        /****************** Set TransactionID ***************************/
        if (highestTransactionID_ == 65535)
        {
            highestTransactionID_ = 1;
        }
        else
        {
            ++highestTransactionID_;
        }

        activeTransactionID_ = highestTransactionID_;

        //Create MOB_SCN_REQ Compound
        ConnectionIdentifier::Ptr pmCI;
        wns::ldk::CompoundPtr newCompound;
        ScanningCommand* command;
        wns::ldk::ClassifierCommand* cCommand;

        pmCI = friends_.connectionManager->getPrimaryConnectionFor(
            dynamic_cast<Component*>( getFUN()->getLayer() )->getID() );
        assure(pmCI, "ScanningSS::start: Didn't get a ConnectionIdentifier from ConnectionManager!");

        newCompound = wns::ldk::CompoundPtr(
            new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));
        command = this->activateCommand(newCompound->getCommandPool());
        cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
            friends_.connectionClassifier->activateCommand( newCompound->getCommandPool() ) );

        cCommand->peer.id = pmCI->cid_;
        command->peer.managementMessageType = MACManagementMessage::MOB_SCN_REQ;
        command->peer.mob_scn_req.transactionID = activeTransactionID_;
        command->peer.mob_scn_req.scanDuration	= scan_.scanDuration;
        command->peer.mob_scn_req.stationsToScan = scan_.stationsToScan;
        command->magic.size = mob_scn_reqPDUSize_;

        //Set timerWaitingForRSP
        remainTimerWaitingForRSP_ = timerWaitingForRSP_;

        // Send compound
        compoundQueue_.push_back(newCompound);
        doWakeup();
    }
    else
    {
        //Initial Scanning
        startScanning();
    }
}

void
ScanningSS::doOnData(const wns::ldk::CompoundPtr& compound)
    // get Management Messages Compound
{
    LOG_INFO( getFUN()->getName(),
              ": Receiving management message:",
              getCommand( compound->getCommandPool() )
              ->peer.managementMessageType );

    ScanningCommand* command;
    command = this->getCommand( compound->getCommandPool() );

    if ( command->peer.managementMessageType == MACManagementMessage::MOB_SCN_RSP )
    {
        this->doOnMOB_SCN_RSP(compound);
    }else
    {
        assure(0 ,"ScanningSS::doOnData: managementMessageType is unknown type");
    }
}

void
ScanningSS::messageNewFrame()
{
    /********* decrease remainTimerWaitingForSCN_RSP if necessary **********/
    assure( !(remainTimerWaitingForRSP_ > 0 && scan_.scan),
            "ScanningSS::messageNewFrame: remainTimerWaitingForRSP mustn't set in scanning mode!");
    if (   ( scan_.scan == false )
           && ( remainTimerWaitingForRSP_ == 0 )
        )
    {
        remainTimerWaitingForRSP_ = -1;
        resultScanning( wimac::CIRMeasureInterface::MeasureValues() );
    }else if ( remainTimerWaitingForRSP_ > 0 )
    {
        --remainTimerWaitingForRSP_;
    }


    /********** Do on Scanning ***********************/
    if ( scan_.scan == true )
    {
        if( scan_.remainingScanDuration == scan_.scanDuration )
        {   // Start Scanning
            LOG_INFO(getFUN()->getName(), ": start Scanning");
            scan_.startTime = wns::simulator::getEventScheduler()->getTime();

            // set CI to NotListening
            ConnectionIdentifiers cis =
                friends_.connectionManager->getAllConnections();
            for( ConnectionIdentifiers::iterator
                     it = cis.begin(); it != cis.end(); ++it )
            {
                (*it)->ciNotListening_ = scan_.remainingScanDuration;
            }
            friends_.connectionManager->changeConnections(cis);

            // start measuring
            --scan_.remainingScanDuration;
            ++scan_.itStationsToScan;
        }else if( scan_.remainingScanDuration == 0 )
        {   // Stop Scanning
            LOG_INFO(getFUN()->getName(), ": stop Scanning");
            scan_.stopTime = wns::simulator::getEventScheduler()->getTime();
            scan_.scan = false;
            scan_.remainingScanDuration = -1;
            wimac::CIRMeasureInterface::MeasureValues measureValuesOutput;

            // Return measureValuesOutput to callBackInterface
            resultScanning(measureValuesOutput);
        }else if( scan_.remainingScanDuration > 0 )
        {
            if(  !(scan_.remainingScanDuration % timerBetweenFChange_)
                 &&(scan_.itStationsToScan != scan_.stationsToScan.end()) )
            {
                ++scan_.itStationsToScan;
            }
            --scan_.remainingScanDuration;
        } else
        {
            assure(0, "This can't happen!");
        }
    }


    /********* decrease all CI which are not listening *****************/
    friends_.connectionManager->decreaseCINotListening();
}




void
ScanningSS::onFUNCreated()
{
    assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
           wns::service::dll::StationTypes::AP(),
           "wimac::ScanningBS: Station is not a BS! \n" );

    friends_.newFrameProvider = getFUN()
        ->findFriend<wns::ldk::fcf::NewFrameProvider*>(friends_.newFrameProviderName);

    friends_.connectionManager = getFUN()->getLayer()
        ->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

    friends_.connectionClassifier = getFUN()
        ->findFriend<ConnectionClassifier*>(friends_.connectionClassifierName);


    // Connect to MessageProvider
    friends_.newFrameProvider->attachObserver(this);

}

void
ScanningSS::startScanning()
{
    assure (scan_.stationsToScan.size(),
            "ScanningSS::startScanning(): No stations to scan are given.\n");
    assure (int(scan_.stationsToScan.size()*timerBetweenFChange_)
            <= scan_.scanDuration, "ScanningSS::startScanning: scanDuration to short for this contract.\n");
    assure (scan_.scan == false,
            " ScanningSS::startScanning(): Can't start scanning, because Scanning is scanning at the moment.\n");

    // setVariables
    scan_.scan = true;
    scan_.remainingScanDuration = scan_.scanDuration;
    scan_.itStationsToScan = scan_.stationsToScan.begin();

}

void
ScanningSS::doOnMOB_SCN_RSP(const wns::ldk::CompoundPtr& compound)
{
    assure(getCommand(compound->getCommandPool())->peer.managementMessageType
           ==  MACManagementMessage::MOB_SCN_RSP,
           "wimac::ScanningSS::doOnMOB_SCN_RSP: only accept MOB_SCN_RSP!\n");

    if(remainTimerWaitingForRSP_ <= 0) // if timer run out, we don't accept
    { // anything
        LOG_INFO( getFUN()->getName(),
                  ": doOnMOB_SCN_RSP remainTimerWaitingForRSP has run out.");
        return;
    }

    /****** Extract Compound **********/
    ScanningCommand* scanningCommand;
    scanningCommand = this->getCommand( compound->getCommandPool() );
    wns::ldk::ClassifierCommand* cCommand;
    cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>
        ( friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

    /*********** Is it the right transactionID? *****/
    if ( scanningCommand->peer.mob_scn_rsp.transactionID != activeTransactionID_ )
        return;

    remainTimerWaitingForRSP_ = -1;

    // Set NotListening for all connections.
    ConnectionIdentifier::StationID subscriberStation;
    ConnectionIdentifier::List cINotListening;
    ConnectionIdentifier::Ptr ci;

    ci = friends_.connectionManager->getConnectionWithID(cCommand->peer.id);
    assure( ci,
            "ScanningSS::doOnMOB_SCN_RSP: Didn't get ConnectionIdentifier from ConnnectionManager for Compound CID!");
    subscriberStation = ci->subscriberStation_;


    assure(scanningCommand->peer.mob_scn_rsp.scanDuration
           >= int(scanningCommand->peer.mob_scn_rsp.stationsToScan.size()*timerBetweenFChange_),
           "ScanningSS::doOnData: mob_scn_rsp.scanDuration is to short for scanning contract! \n");

    assure(scanningCommand->peer.mob_scn_rsp.stationsToScan.size() > 0,
           "ScanningSS::doOnMOB_SCN_RSP: No stations are given!\n");

    // Start scanning
    scan_.scanDuration = scanningCommand->peer.mob_scn_rsp.scanDuration;
    scan_.stationsToScan = scanningCommand->peer.mob_scn_rsp.stationsToScan;
    startScanning();
}

void
ScanningSS::resultScanning(wimac::CIRMeasureInterface::MeasureValues measureValues)
{
    LOG_INFO( getFUN()->getName(),
              ": Scanning finished. StartTime: ",scan_.startTime,
              " StopTime: ", scan_.stopTime);

    if ( !measureValues.empty() )
    {
        LOG_INFO( getFUN()->getName(),
                  ":    Scanning measureValues [station, frequncy, cir]:");

        for (wimac::CIRMeasureInterface::MeasureValues::iterator it
                 = measureValues.begin();
             it != measureValues.end(); ++it)
        {
            LOG_INFO( getFUN()->getName(),
                      ":                           (", (*it).station->getName(),", "
                      ,(*it).tune.frequency," MHz, ", (*it).cir,")");
        }

    } else
    {
        LOG_INFO( getFUN()->getName(),
                  " Scanning failed, no measureValues! ");
    }
    scan_.scan = false;
    scan_.startTime = 0;
    scan_.stopTime = 0;

    ScanningCallBackInterface* callBackInterface = callBackInterface_;
    callBackInterface_ = NULL;
    callBackInterface->resultScanning(measureValues);
}




