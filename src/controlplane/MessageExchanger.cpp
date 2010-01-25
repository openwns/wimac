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

#include <WIMAC/controlplane/MessageExchanger.hpp>

#include <WNS/logger/Logger.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/Component.hpp>

using namespace wimac::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::controlplane::MessageExchanger,
    wns::ldk::FunctionalUnit,
    "wimac.controlplane.MessageExchanger",
    wns::ldk::FUNConfigCreator);

MessageExchanger::MessageExchanger(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::CommandTypeSpecifier< MessageExchangerCommand >(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<>(),
    wns::ldk::HasDeliverer<>(),
    wns::Cloneable< MessageExchanger >(),
    wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
    messageExchanges_(MessageExchanges()),
    callBackInterface_(NULL),
    compoundQueue_(),
    logger_("WIMAC", "MessageExchanger"),
    connectionType_( config.get<int>("connectionType") ),
    timerWaitingForReply_( config.get<int>("timerWaitingForReply") ),
    messages_(),
    friends_()
{
    for(int i = 0; i < config.len("messages"); ++i)
    {
        wns::pyconfig::View message = config.getView("messages", i);
        messages_[message.get<MessageType>("messageType")] = message.get<Bit>("size");
    }


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
MessageExchanger::start(MessageExchangerCallBackInterface* callBackInterface)
{
    assure (callBackInterface,
            "wimac::MessageExchanger::start: Get no callBackInterface! \n");

    MESSAGE_SINGLE(NORMAL,logger_, getFUN()->getName()
                   << ": MessagesExchanger name: " << this->getName()
                   << ";   calling start().");

    if(callBackInterface_)
    {
        MESSAGE_SINGLE(NORMAL, logger_, getFUN()->getName()
                       << ": MessagesExchanger name: "
                       << this->getName() << ";   is in use.");
        callBackInterface->resultMessageExchanger(this->getName(),false);
        return;
    }

    this->clear();

    callBackInterface_ = callBackInterface;

    StationID peerID = getPeerID(0);

    messageExchanges_[peerID].exchangeID.time = time(0);
    messageExchanges_[peerID].exchangeID.funName = this->getName();
    messageExchanges_[peerID].remainTimerWaitingForReply = -1;
    messageExchanges_[peerID].sendMessage = messages_.end();


    /********** Create and send Compound *****************/
    MessageType messageType = messages_.begin()->first;
    compoundQueue_.push_back( this->createMessage(peerID, messageType) );
    doWakeup();
}

void
MessageExchanger::doOnData(const wns::ldk::CompoundPtr& compound)
{
    MESSAGE_SINGLE(NORMAL, logger_,
                   getFUN()->getName() << ": receiving management message: "
                   << getCommand( compound->getCommandPool() )
                   ->peer.managementMessageType );

    MessageExchangerCommand* command;
    command = getCommand( compound->getCommandPool() );
    wns::ldk::ClassifierCommand* cCommand;
    cCommand = friends_.connectionClassifier
        ->getCommand( compound->getCommandPool() ) ;


    /*********** Get peerID *********************/
    ConnectionIdentifierPtr ci =
        friends_.connectionManager->getConnectionWithID(cCommand->peer.id);
    assure(ci,
           "MessageExchanger::doOnData: Didn't get ConnectionIdentifier "
           "from ConnectionManager for CID of that compound!");

    StationID stationID =
        dynamic_cast<Component*>(getFUN()->getLayer())->getID();

    StationID peerID = 0;
    if( ci->subscriberStation_ == stationID )
    {
        peerID = ci->baseStation_;
    }
    else if( ci->baseStation_ == stationID )
    {
        peerID = ci->subscriberStation_;
    }
    else
    {
        throw wns::Exception("MessageExchanger::start: no peer found in Ranging CI");
    }

    /******* Ceck if this station is the right receiver.******/
    if ( command->peer.peerID != dynamic_cast<Component*>
         (getFUN()->getLayer())->getID() )
        return;


    /********** get this Message form all  messages ***********/
    Messages::const_iterator it = messages_.begin();
    for(; it != messages_.end(); ++it)
    {
        if( it->first == command->peer.managementMessageType )
            break;
    }
    assure( it != messages_.end(),
            "MessageExchanger::doOnData: Received unknown message type!\n");


    /********** Handel new peer and check integrity ***********/
    if(messageExchanges_.find(peerID)== messageExchanges_.end())
    { //Check if it is the first message
        if(it != messages_.begin())
        {
            MESSAGE_SINGLE(NORMAL, logger_, getFUN()->getName()
                           << ": MessageExchangerName: " << this->getName()
                           << ";   New peer hasn't started messages exchange with the first message!");
            return;
        }

        //Insert new PeerSet
        messageExchanges_[peerID].exchangeID = command->peer.exchangeID;
        messageExchanges_[peerID].remainTimerWaitingForReply = -1;
        messageExchanges_[peerID].sendMessage = messages_.end();
    } else
    { // Check PeerSet
        if(messageExchanges_[peerID].exchangeID != command->peer.exchangeID)
        {
            MESSAGE_SINGLE(NORMAL, logger_, getFUN()->getName()
                           << ": MessageExchangerName" << this->getName()
                           << ";   Wrong exchangeID. This message is not for us or wrong messages exchange!");
            return;
        }
        Messages::const_iterator messageBefore = it;
        --messageBefore;
        if(messageExchanges_[peerID].sendMessage != messageBefore)
        {
            MESSAGE_SINGLE(NORMAL,logger_, getFUN()->getName()
                           << ": MessageExchangerName: " << this->getName()
                           << ";    This message is in wrong sequence of that messages exchange!");
            return;
        }
        if(messageExchanges_[peerID].remainTimerWaitingForReply <= 0)
        {
            MESSAGE_SINGLE(NORMAL, logger_, getFUN()->getName()
                           << ": MessageExchangerName: " << this->getName()
                           << ";   Timer WaitingForReply has run out for that message!");
            return;
        }
    }

    /********* Reset timerWaitingForRSP *************/
    messageExchanges_[peerID].remainTimerWaitingForReply = -1;


    /********* Get next message in exchange process *******/
    ++it; // Get next message


    /********** Has received last Message -> End message exchange **********/
    if( it == messages_.end() )
    {
        this->result(peerID, true);
        return;
    }

    // Create and send Compound
    compoundQueue_.push_back( this->createMessage( peerID, it->first ) );
    doWakeup();
}

void
MessageExchanger::doWakeup()
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


        MESSAGE_SINGLE(NORMAL, logger_,
                       getFUN()->getName() << ": sending management message: "
                       << getCommand( compound->getCommandPool() )
                       ->peer.managementMessageType );


        /*********** Get peerID *********************/
        StationID peerID;
        peerID = this->getPeerID(cCommand->peer.id);


        /********** Set timerWaitingForReply ********/
        messageExchanges_[peerID].remainTimerWaitingForReply = timerWaitingForReply_;

        /********** remove compound from queue and send *******************/
        compoundQueue_.remove(compound);
        getConnector()->getAcceptor(compound)->sendData(compound);

        /********** Has sent last Message -> End message exchange *******/
        if(  messageExchanges_[peerID].sendMessage == --messages_.end() )
        {
            this->result(peerID, true);
            return;
        }
    }
}

void
MessageExchanger::onFUNCreated()
{
    friends_.connectionClassifier = getFUN()
        ->findFriend<wimac::ConnectionClassifier*>(friends_.connectionClassifierName);

    friends_.connectionManager = dynamic_cast<Component*>(getFUN()->getLayer())
        ->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

    friends_.newFrameProvider = getFUN()
        ->findFriend<wns::ldk::fcf::NewFrameProvider*>
        (friends_.newFrameProviderName);

    friends_.newFrameProvider->attachObserver(this);
}

void
MessageExchanger::messageNewFrame()
{
    for(MessageExchanges::iterator it = messageExchanges_.begin();
        it != messageExchanges_.end(); )
    {
        // Copy iterator, because it could be that the element or the whole list
        // will be deleted.
        MessageExchanges::iterator cpIt = it;
        ++it;

        if (cpIt->second.remainTimerWaitingForReply == 0)
        {
            /********* Reset timerWaitingForRSP *************/
            cpIt->second.remainTimerWaitingForReply = -1;

            MESSAGE_SINGLE(NORMAL,logger_, getFUN()->getName() << ": MessageExchangerName: "
                           << this->getName()
                           << ";    timerWaitingForReplay is running out!");
            this->result(cpIt->first, false);
            continue; // It could be that element of cpIt exist no more.
        }

        /************* decrease TimerWaitingForReply *******************/
        if (cpIt->second.remainTimerWaitingForReply > 0)
            --(cpIt->second.remainTimerWaitingForReply);
    }
}

void
MessageExchanger::result(StationID peerID, bool result)
{
    MESSAGE_SINGLE(NORMAL, logger_, getFUN()->getName() << ": MessageExchangerName: "
                   << this->getName() << ";    finished [PeerID,result]:  " << peerID << ", " << result);

    if(callBackInterface_)
    {
        MessageExchangerCallBackInterface* callBackInterface = callBackInterface_;
        callBackInterface_ = NULL;
        this->clear();
        callBackInterface->resultMessageExchanger( this->getName(), result);
    } else
        messageExchanges_.erase(peerID);
}

wns::ldk::CompoundPtr
MessageExchanger::createMessage(const StationID peerID, const MessageType messageType)
{
    /************* set sendMessage in PeerSet *****************/
    messageExchanges_.find(peerID)->second.sendMessage = messages_.find(messageType);

    /*********** Create new message Compound *************************************/
    wns::ldk::CompoundPtr newCompound;
    newCompound = wns::ldk::CompoundPtr(
        new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

    // Set ClassifierCommand
    wns::ldk::ClassifierCommand* newCCommand;
    newCCommand = friends_.connectionClassifier
        ->activateCommand( newCompound->getCommandPool() );

    ConnectionIdentifiers cis =
        friends_.connectionManager->getOutgoingConnections(peerID);
    assure(!cis.empty(),
           "MessageExchanger::createMessage: Haven't get ConnectionIdentifiers for PeerID!\n");

    ConnectionIdentifiers::const_iterator
        it = cis.begin();
    for(; it != cis.end(); ++it)
    {
        if((*it)->connectionType_ == connectionType_)
            break;
    }
    assure( it != cis.end(),
            "MessageExchanger::createMessage: Haven't found outgoing  ConnectionIdentifier with right ConnectionType!\n");

    newCCommand->peer.id = (*it)->cid_;


    // Set MessageExchangerCommand
    MessageExchangerCommand* newCommand;
    newCommand = activateCommand( newCompound->getCommandPool() );
    newCommand->peer.managementMessageType = messageType;
    newCommand->peer.peerID = peerID;
    newCommand->peer.exchangeID = messageExchanges_.find(peerID)->second.exchangeID;
    newCommand->magic.size = messages_.find(messageType)->second;

    return newCompound;
}

MessageExchanger::StationID
MessageExchanger::getPeerID(const CID cid) const
{
    ConnectionIdentifierPtr ci;
    StationID stationID = -1;
    StationID peerID = -1;

    ci = friends_.connectionManager->getConnectionWithID(cid);
    assure( ci,
            "MessageExchanger::getPeerID: Haven't get an ConnectionIdentifier for CID!\n" );
    stationID = dynamic_cast<Component*>(getFUN()->getLayer())->getID();

    if( ci->subscriberStation_ == stationID )
        peerID = ci->baseStation_;
    else if( ci->baseStation_ == stationID )
        peerID = ci->subscriberStation_;
    else
        assure(0,"MessageExchanger::start: no peer found in Ranging CI");

    return peerID;
}

void
MessageExchanger::clear()
{
    messageExchanges_.clear();
    callBackInterface_ = NULL;
    compoundQueue_.clear();
}


