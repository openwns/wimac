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

#include <WIMAC/controlplane/Ranging.hpp>

#include <WNS/rng/RNGen.hpp>
#include <WNS/ldk/Classifier.hpp>

#include <WIMAC/services/ConnectionManager.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/PhyUser.hpp>

using namespace wimac;
using namespace wimac::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::controlplane::RangingBS,
    wns::ldk::FunctionalUnit,
    "wimac.controlplane.RangingBS",
    wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::controlplane::RangingSS,
    wns::ldk::FunctionalUnit,
    "wimac.controlplane.RangingSS",
    wns::ldk::FUNConfigCreator);


Ranging::Ranging(wns::ldk::fun::FUN* fun, const wns::pyconfig::View&) :
    wns::ldk::CommandTypeSpecifier< RangingCommand >(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>()
{
}

RangingBS::RangingBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Ranging(fun, config),
    wns::Cloneable< RangingBS >(),
    rng_rspPDUSize_(config.get<Bit>("rng_rspPDUSize")),
    friends_()
{
    friends_.connectionManagerName = config.get<std::string>("connectionManager");
    friends_.connectionClassifierName = config.get<std::string>("connectionClassifier");

    friends_.connectionManager = NULL;
    friends_.connectionClassifier = NULL;

}

void
RangingBS::doOnData(const wns::ldk::CompoundPtr& compound)
    // get Management Messages (Compound) do the switch
{
    LOG_INFO( getFUN()->getName(),
              ": Receiving management message:",
              getCommand( compound->getCommandPool() )
              ->peer.managementMessageType );

    RangingCommand* command;
    command = getCommand( compound->getCommandPool() );

    if ( command->peer.managementMessageType
         == MACManagementMessage::RNG_REQ )
    {
        this->doOnRNG_REQ(compound);
    } else
    {
        assure(0 ,"RangingBS::doOnData: managementMessageType is unknown type!\n");
    }

}



void
RangingBS::doWakeup()
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

void
RangingBS::onFUNCreated()
{

    assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
           wns::service::dll::StationTypes::UT(),
           "wimac::RangingBS: Station is not a BS! \n" );

    friends_.connectionManager =getFUN()->getLayer()
        ->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

    friends_.connectionClassifier = getFUN()
        ->findFriend<FunctionalUnit*>(friends_.connectionClassifierName);

}

void
RangingBS::doOnRNG_REQ(const wns::ldk::CompoundPtr& compound)
{
    assure(this->getCommand(compound->getCommandPool())->peer.managementMessageType
           ==  MACManagementMessage::RNG_REQ,
           "wimac::Ranging::doOnRANG_REQ: only accept RNG_REQ!  \n");

    /*********  Extract information from command ************/
    RangingCommand::TransactionID transactionID;
    ConnectionIdentifier::StationID baseStation;
    ConnectionIdentifier::StationID subscriberStation;

    transactionID = this->getCommand( compound->getCommandPool() )
        ->peer.rng_req.transactionID;
    baseStation = this->getCommand( compound->getCommandPool() )
        ->peer.rng_req.baseStation;
    subscriberStation = this->getCommand( compound->getCommandPool() )
        ->peer.rng_req.subscriberStation;

    // Everythink ok?
    assure(baseStation
           ==  dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getID(),
           "wimac::Ranging::DATAind: Range to wrong base Station! \n");


    /*********** Create ranging ConnectionIdentifier *************/
    ConnectionIdentifier rangingCI(
        baseStation,
        0,
        0,
        0,
        ConnectionIdentifier::InitialRanging,
        ConnectionIdentifier::Bidirectional,
        ConnectionIdentifier::Signaling);
    friends_.connectionManager->appendConnection(rangingCI);


    /*********** Create ConnectionIdentifiers *************/
    // basic CID
    ConnectionIdentifier bCI(
        baseStation,
        subscriberStation,
        subscriberStation,
        ConnectionIdentifier::Basic,
        ConnectionIdentifier::Bidirectional,
        ConnectionIdentifier::Signaling);
    bCI = friends_.connectionManager->appendConnection(bCI);

    // primary management CID
    ConnectionIdentifier pmCI(
        baseStation,
        subscriberStation,
        subscriberStation,
        ConnectionIdentifier::PrimaryManagement,
        ConnectionIdentifier::Bidirectional,
        ConnectionIdentifier::Signaling);
    pmCI = friends_.connectionManager->appendConnection(pmCI);


    /*********** Create RNG_RSP Compound ********************/
    wns::ldk::CompoundPtr newCompound;
    newCompound = wns::ldk::CompoundPtr(
        new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

    friends_.connectionClassifier->activateCommand(newCompound->getCommandPool());
    dynamic_cast<wns::ldk::ClassifierCommand*>
        (friends_.connectionClassifier->getCommand( newCompound->getCommandPool() ))
        ->peer.id = 0;

    this->activateCommand(newCompound->getCommandPool());
    this->getCommand( newCompound->getCommandPool() )->peer.managementMessageType
        = MACManagementMessage::RNG_RSP;
    this->getCommand( newCompound->getCommandPool() )->peer.rng_rsp.transactionID
        = transactionID;
    this->getCommand( newCompound->getCommandPool() )->peer.rng_rsp.baseStation
        = baseStation;
    this->getCommand( newCompound->getCommandPool() )->peer.rng_rsp.subscriberStation
        = subscriberStation;
    this->getCommand( newCompound->getCommandPool() )->peer.rng_rsp.basicCID
        = bCI.cid_;
    this->getCommand( newCompound->getCommandPool() )->peer.rng_rsp.primaryManagementCID
        = pmCI.cid_;
    this->getCommand( newCompound->getCommandPool() )->magic.size = rng_rspPDUSize_;


    /************ Send Compound ********************************/
    compoundQueue_.push_back(newCompound);
    doWakeup();
}

RangingSS::RangingSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    Ranging(fun, config),
    wns::Cloneable< RangingSS >(),
    wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
    activeTransactionID_(0),
    highestTransactionID_(0),
    remainTimerWaitingForRSP_(-1),
    remainNumberOfRetries_(-1),
    compoundQueue_(),
    callBackInterface_(NULL),
    rngCompound_(wns::ldk::CompoundPtr()),
    rngDis_(),
    rng_reqPDUSize_(config.get<Bit>("rng_reqPDUSize")),
    timerWaitingForRSP_(config.get<int>("timerWaitingForRSP")),
    numberOfRetries_(config.get<int>("numberOfRetries")),
    boWindowSizeMin_(config.get<int>("boWindowSizeMin")),
    boWindowSizeMax_(config.get<int>("boWindowSizeMax")),
    friends_()
{
    friends_.connectionManagerName
        = config.get<std::string>("connectionManager");
    friends_.connectionClassifierName
        = config.get<std::string>("connectionClassifier");
    friends_.newFrameProviderName
        = config.get<std::string>("newFrameProvider");

    friends_.connectionManager = NULL;
    friends_.connectionClassifier = NULL;
    friends_.newFrameProvider = NULL;
}

void
RangingSS::start(wns::service::phy::ofdma::Tune tune,
                 ConnectionIdentifier::StationID baseStation,
                 RangingCallBackInterface* callBackInterface)
{
    assure (baseStation,
            "wimac::RangingSS::range: Need base station to range. \n");
    assure (callBackInterface,
            "wimac::RangingSS::range: Get no callBackInterface! \n");

    LOG_INFO( getFUN()->getName(),": start RangingSS.");

    // Check if RangingSS is in use
    if(callBackInterface_)
    {
        LOG_INFO( getFUN()->getName(),": RangingSS is used.");
        callBackInterface_->resultRanging(false);
        return;
    }

    callBackInterface_ = callBackInterface;

    /********* Create Connection Identifier for CID 0 (Ranging) ***********/
    // Necessary to tune scheduler to the right baseStation
    ConnectionIdentifier connection(
        baseStation,
        0,
        dynamic_cast<wimac::Component*>( getFUN()->getLayer() )->getID(),
        dynamic_cast<wimac::Component*>( getFUN()->getLayer() )->getID(),
        ConnectionIdentifier::InitialRanging,
        ConnectionIdentifier::Bidirectional,
        ConnectionIdentifier::Signaling );
    friends_.connectionManager->appendConnection(connection);


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


    /********* Create RNG_REQ Compound ***********/
    wns::ldk::CompoundPtr newCompound;
    RangingCommand* command;
    newCompound = wns::ldk::CompoundPtr(
        new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

    // connectionClassifier( CID )
    friends_.connectionClassifier->activateCommand( newCompound->getCommandPool() );
    dynamic_cast<wns::ldk::ClassifierCommand*>
        (friends_.connectionClassifier->getCommand( newCompound->getCommandPool() ))
        ->peer.id = 0; // 0 Initinal ranging CID

    // Ranging
    command = this->activateCommand(newCompound->getCommandPool());
    command->peer.managementMessageType = MACManagementMessage::RNG_REQ;
    command->peer.rng_req.transactionID = activeTransactionID_;
    command->peer.rng_req.baseStation = baseStation;
    command->peer.rng_req.subscriberStation = dynamic_cast<wimac::Component*>
        (getFUN()->getLayer())->getID();
    command->magic.size = rng_reqPDUSize_;

    this->sendContentionAccess(newCompound);
}

void
RangingSS::doOnData(const wns::ldk::CompoundPtr& compound)
    // get Management Messages Compound
{
    LOG_INFO( getFUN()->getName(),
              ": Receiving management message:",
              getCommand( compound->getCommandPool() )
              ->peer.managementMessageType );

    RangingCommand* command;
    command = getCommand( compound->getCommandPool() );


    if ( command->peer.managementMessageType
         == MACManagementMessage::RNG_RSP )
    {
        this->doOnRNG_RSP(compound);
    }else
    {
        assure(0 ,"RangingSS::doOnData: managementMessageType is unknown type!\n");
    }

}

void
RangingSS::doWakeup()
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


        LOG_INFO( getFUN()->getName(), ": Sending management message:",
                  getCommand( compound->getCommandPool() )
                  ->peer.managementMessageType );


        /********* Set timerWaitingForRSP *************/
        remainTimerWaitingForRSP_ = timerWaitingForRSP_;
        friends_.newFrameProvider->attachObserver(this);


        // Remove compound from queue and send
        compoundQueue_.remove(compound);
        getConnector()->getAcceptor(compound)->sendData(compound);
    }
}

void
RangingSS::onFUNCreated()
{
    assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
           wns::service::dll::StationTypes::AP(),
           "wimac::RangingBS: Station is not a BS! \n" );

    friends_.connectionManager =
        getFUN()->getLayer()->getManagementService<service::ConnectionManager>
        (friends_.connectionManagerName);

    friends_.connectionClassifier = getFUN()
        ->findFriend<FunctionalUnit*>(friends_.connectionClassifierName);

    friends_.newFrameProvider = getFUN()
        ->findFriend<wns::ldk::fcf::NewFrameProvider*>
        (friends_.newFrameProviderName);

}

void
RangingSS::messageNewFrame()
{
    if ( remainTimerWaitingForRSP_ == 0 )
    {
        LOG_INFO( getFUN()->getName(), ": timer WaitingforRSP run out! ");

        /********* Reset timerWaitingForRSP *************/
        friends_.newFrameProvider->detachObserver(this);
        remainTimerWaitingForRSP_ = -1;

        if( remainNumberOfRetries_ > 0)
        {
            this->sendContentionAccess(rngCompound_);
        }else
        {
            remainNumberOfRetries_ = -1;
            resultRanging(false);
        }

    }

    if(remainTimerWaitingForRSP_ > 0)
        --remainTimerWaitingForRSP_;
}


void
RangingSS::doOnRNG_RSP(const wns::ldk::CompoundPtr& compound)
{
    assure(getCommand(compound->getCommandPool())->peer.managementMessageType
           ==  MACManagementMessage::RNG_RSP,
           "wimac::RangingSS::doOnRNG_RSP: only accept RNG_RSP! \n");


    RangingCommand* command =  getCommand( compound->getCommandPool() );


    /*********** get ranging ConnectionIdentifier *************/
    ConnectionIdentifierPtr rangingCI =
        friends_.connectionManager->getConnectionWithID( 0 );
    assure(rangingCI,
           "RangingSS::doOnRNG_RSP: Didn't get ConnectionIdentifier from ConnectionManager for CID:0!");

    // Is this message for us?
    if ( command->peer.rng_rsp.subscriberStation
         !=  dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getID() )
    {
        return;
    }

    // Timer WaitingForRSP nust not expired
    if( remainTimerWaitingForRSP_ <= 0)
    {
        LOG_INFO(getFUN()->getName(), ": Timer watingForRSP has run out!");
        return;
    }

    // Is it the right transactionID?
    if ( command->peer.rng_rsp.transactionID != activeTransactionID_ )
        return;

    // Everythink ok?
    assure(command->peer.rng_rsp.baseStation ==  rangingCI->baseStation_,
           "wimac::Ranging::DATAind: Range to wrong base Station! \n");
    assure(command->peer.rng_rsp.subscriberStation == rangingCI->subscriberStation_,
           "wimac::Ranging::DATAind: Wrong subscriberStation! \n");
    assure(command->peer.rng_rsp.subscriberStation
           ==  dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getID(),
           "wimac::Ranging::DATAind: Wrong subscriberStation! \n");

    /********** Reset timerWaitingForRSP and numberOfRetries ***********/
    friends_.newFrameProvider->detachObserver(this);
    remainTimerWaitingForRSP_ = -1;
    remainNumberOfRetries_ = -1;

    /*********** Create ConnectionIdentifiers *************/
    // basic CID
    ConnectionIdentifier bConnection(
        command->peer.rng_rsp.baseStation,
        command->peer.rng_rsp.basicCID,
        command->peer.rng_rsp.subscriberStation,
        command->peer.rng_rsp.subscriberStation,
        ConnectionIdentifier::Basic,
        ConnectionIdentifier::Bidirectional,
        ConnectionIdentifier::Signaling);
    friends_.connectionManager->appendConnection(bConnection);

    // primary management CID
    ConnectionIdentifier pmConnection(
        command->peer.rng_rsp.baseStation,
        command->peer.rng_rsp.primaryManagementCID,
        command->peer.rng_rsp.subscriberStation,
        command->peer.rng_rsp.subscriberStation,
        ConnectionIdentifier::PrimaryManagement,
        ConnectionIdentifier::Bidirectional,
        ConnectionIdentifier::Signaling);
    friends_.connectionManager->appendConnection(pmConnection);

    resultRanging(true);
}

void
RangingSS::resultRanging(bool result)
{
    LOG_INFO( getFUN()->getName(),": Stop Ranging with result:", result);

    /********* Call the callBackInterface *****************/
    RangingCallBackInterface* callBackInterface = callBackInterface_;
    callBackInterface_ = NULL;
    callBackInterface->resultRanging(result);

}

void
RangingSS::sendContentionAccess(const wns::ldk::CompoundPtr compound)
{
    if (remainNumberOfRetries_ < 0)
    {  // first Try
        remainNumberOfRetries_ = numberOfRetries_ + 1;
    }
    --remainNumberOfRetries_;

    /*********** Copy Compound to rngCompound_ *******/
    rngCompound_ =  compound->copy();


    /********** Get window size *********************/
    int windowSize = boWindowSizeMin_ * (numberOfRetries_ - remainNumberOfRetries_ + 1);
    if (windowSize > boWindowSizeMax_)
        windowSize = boWindowSizeMax_;

    int backOff = int(rngDis_()*windowSize);
    LOG_INFO(getFUN()->getName(),": Set backOff to: ", backOff,
             "   WindowsSize: ", windowSize);

    /********* Send or queue Compound  ***********/
    compoundQueue_.push_back(compound);
    doWakeup();
}
