/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
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
 * \file
 * \author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/controlplane/SetupConnection.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/ldk/Classifier.hpp>



using namespace wimac::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::controlplane::SetupConnectionBS,
	wns::ldk::FunctionalUnit,
	"wimac.controlplane.SetupConnectionBS",
	wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::controlplane::SetupConnectionSS,
	wns::ldk::FunctionalUnit,
	"wimac.controlplane.SetupConnectionSS",
	wns::ldk::FUNConfigCreator);



/********** SetupConnectionBS **************************************************/

SetupConnectionBS::SetupConnectionBS(wns::ldk::fun::FUN* fun,
									 const wns::pyconfig::View& config) :
	wns::ldk::CommandTypeSpecifier< SetupConnectionCommand >(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<>(),
	wns::Cloneable< SetupConnectionBS >(),
	wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
	waitingForSsDSA_ACK_(),
	compoundQueue_(),
	timerWaitingForACK_( config.get<ConnectionIdentifier::Frames>
						 ("timerWaitingForACK") ),
	dsa_rspPDUSize_( config.get<Bit>("dsa_rspPDUSize") ),
	friends_()
{
	friends_.connectionManagerName = config.get<std::string>("connectionManager");
	friends_.connectionClassifierName = config.get<std::string>("connectionClassifier");
	friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");

	friends_.connectionManager = NULL;
	friends_.connectionClassifier = NULL;
	friends_.newFrameProvider = NULL;

}



void
SetupConnectionBS::doOnData(const wns::ldk::CompoundPtr& compound)
// get Management Messages (Compound)
{
	LOG_INFO( getFUN()->getName(),
			  ": Receiving management message:",
			  getCommand( compound->getCommandPool() )
			  ->peer.managementMessageType );

	SetupConnectionCommand* command;
	command = getCommand( compound->getCommandPool() );

	if ( command->peer.managementMessageType == MACManagementMessage::DSA_REQ )
	{
		this->doOnDSA_REQ(compound);
	} else if (command->peer.managementMessageType == MACManagementMessage::DSA_ACK)
	{
		this->doOnDSA_ACK(compound);
	} else
	{
		assure(0 ,"SetupConnectionBS::doOnData: managementMessageType is unknown type");
	}

} //doOnData



void
SetupConnectionBS::doWakeup()
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


		/********** Set Timer *********************************/
		for( WaitingForSsDSA_ACK::iterator it = waitingForSsDSA_ACK_.begin();
			 it != waitingForSsDSA_ACK_.end(); ++it)
		{
			if( (*it).subscriberStation == ci->subscriberStation_ )
			{
				(*it).remainTimerWaitingForACK = timerWaitingForACK_;
			}
		}


		// Remove compound from queue and send
		compoundQueue_.remove(compound);
		getConnector()->getAcceptor(compound)->sendData(compound);
	}
} // wakeup



void
SetupConnectionBS::onFUNCreated()
{
	assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
		   wns::service::dll::StationTypes::UT(),
		   "wimac::SetupConnectionBS: Station is not a BS! \n" );


	friends_.connectionManager = getFUN()->getLayer()->
		getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

	friends_.connectionClassifier = getFUN()
		->findFriend<wns::ldk::FunctionalUnit*>(friends_.connectionClassifierName);

	friends_.newFrameProvider = getFUN()
		->findFriend<wns::ldk::fcf::NewFrameProvider*>
		(friends_.newFrameProviderName);

	friends_.newFrameProvider->attachObserver(this);

} // onFUNCreated()



void SetupConnectionBS::messageNewFrame()
{
	for(WaitingForSsDSA_ACK::iterator it = waitingForSsDSA_ACK_.begin();
		it != waitingForSsDSA_ACK_.end(); )
	{
		WaitingForSsDSA_ACK::iterator copyIt = it;
		++it; // Because of removing element from list.Otherwise it
              // would be invalid before increasing;
		if ( (*copyIt).remainTimerWaitingForACK == 0 )
		{
			LOG_INFO(getFUN()->getName(),
					 ": timerWaitingForDSA_ACK run out for CID:",
					 (*copyIt).connectionIdentifier.cid_);

			//friends_.connectionManager->deleteCI((*copyIt).connectionIdentifier.cid_);
			// At the moment an incomplete message exchange by an lost ACK,
			// take the simulator to an inconsistent state. The UT has its data
			// connection and sends data to the AP which hasn't any data
			// connection for that UT.
			// But now an lost RSP won't be caught, so that the AP has data
			// connections which aren't allowed. The ConnectionManager will delete these
			// needless connections if an new data connection for that station
			// is created. In this way, uniqueness of the data connections is
			// assured.

			waitingForSsDSA_ACK_.erase( copyIt );
		} else // This else is necessary because copyIt must exist
			if((*copyIt).remainTimerWaitingForACK > 0)
				--((*copyIt).remainTimerWaitingForACK);
	}

} //messageNewFrame()




////////////// Privat functions /////////////////

void
SetupConnectionBS::doOnDSA_REQ(const wns::ldk::CompoundPtr& compound)
{
	assure(this->getCommand(compound->getCommandPool())->peer.managementMessageType
		   ==  MACManagementMessage::DSA_REQ,
		   "wimac::controlPlane::SetupConnectionBS::doOnDSA_REQ: only accept DSA_REQ!\n");


	/********* get commands *******************************/
	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

	SetupConnectionCommand* command;
	command = getCommand( compound->getCommandPool() );


	/*********  Extract information from command ************/
	SetupConnectionCommand::TransactionID transactionID;
	ConnectionIdentifier::QoSCategory ulQoSPriorityREQ;
	ConnectionIdentifier::QoSCategory dlQoSPriorityREQ;
	ConnectionIdentifier::Ptr dsaCI;

	dsaCI = friends_.connectionManager->getConnectionWithID( cCommand->peer.id );
	assure( dsaCI, "Need ConnectionIdetifier for Received DSA_REQ Compound! \n");
	transactionID = command->peer.dsa_req.transactionID;
	ulQoSPriorityREQ = command->peer.dsa_req.ulQoSPriority;
	dlQoSPriorityREQ = command->peer.dsa_req.dlQoSPriority;

	// Everythink ok?
	assure( ulQoSPriorityREQ && dlQoSPriorityREQ && transactionID,
			"wimac::SetupConnectionBS::DATAind: DSA_REQ wron content! \n" );


	/********* Reset possible incomplete DSA message exchange *********/
	for(WaitingForSsDSA_ACK::iterator it = waitingForSsDSA_ACK_.begin();
		it != waitingForSsDSA_ACK_.end(); )
	{
		WaitingForSsDSA_ACK::iterator copyIt = it;
		++it; // Because of removing element from list.Otherwise it
              // would be invalid before increasing;

		if( (*copyIt).subscriberStation == dsaCI->subscriberStation_ )
		{
			LOG_INFO(getFUN()->getName(),
					 ": Reset incompleted DSA message exchange for CID:",
					 (*copyIt).connectionIdentifier.cid_);
			friends_.connectionManager->deleteCI((*copyIt).connectionIdentifier.cid_);
			waitingForSsDSA_ACK_.erase( copyIt );
		}
	}


	/********* Create connection identifiers for UL & DL *******************/
	ConnectionIdentifier newUlCI(
		dsaCI->baseStation_,
		dsaCI->subscriberStation_,
		dsaCI->subscriberStation_,
		ConnectionIdentifier::Data,
		ConnectionIdentifier::Uplink,
		ulQoSPriorityREQ);

	ConnectionIdentifier newDlCI(
		dsaCI->baseStation_,
		dsaCI->subscriberStation_,
		dsaCI->subscriberStation_,
		ConnectionIdentifier::Data,
		ConnectionIdentifier::Downlink,
		dlQoSPriorityREQ);

	newUlCI = friends_.connectionManager->appendConnection(newUlCI);
	newDlCI = friends_.connectionManager->appendConnection(newDlCI);


	/********* Append CIDs to waitingForSsDSA_ACK_ ************************/
	CIWaitingForSsDSA_ACK newUlciw(dsaCI->subscriberStation_,transactionID,
								   newUlCI,-1);
	CIWaitingForSsDSA_ACK newDlciw(dsaCI->subscriberStation_,transactionID,
								   newDlCI,-1);

	waitingForSsDSA_ACK_.push_back(newUlciw);
	waitingForSsDSA_ACK_.push_back(newDlciw);


	/*********** Create response DSA_RSP *********************/
	ConnectionIdentifier::Ptr pmCI;
	pmCI = friends_.connectionManager
        ->getPrimaryConnectionFor( dsaCI->subscriberStation_ );
	assure( pmCI,
			"SetupConnectionBS::doOnDSA_REQ: Didn't get ConnectionIdentifier from ConnectionManager!");

	wns::ldk::CompoundPtr newCompound;
	newCompound = wns::ldk::CompoundPtr(
		new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

	wns::ldk::ClassifierCommand* newCCommand;
	newCCommand = dynamic_cast<wns::ldk::ClassifierCommand*>
		(friends_.connectionClassifier
		 ->activateCommand( newCompound->getCommandPool() ) );
	newCCommand->peer.id = pmCI->cid_;

	assure( dynamic_cast<wns::ldk::ClassifierCommand*>
			(friends_.connectionClassifier->
			 getCommand(newCompound->getCommandPool()))->peer.id,
			"wimac::SetupConnectionBS::doOnData: No primary management connection for this subscriber station!  \n");

	SetupConnectionCommand* newCommand;
	newCommand = activateCommand( newCompound->getCommandPool() );
	newCommand->peer.managementMessageType = MACManagementMessage::DSA_RSP;

	newCommand->peer.dsa_rsp.transactionID = transactionID;
	newCommand->peer.dsa_rsp.ulCID = newUlCI.cid_;
	newCommand->peer.dsa_rsp.ulQoSPriority = newUlCI.qos_;
	newCommand->peer.dsa_rsp.dlCID = newDlCI.cid_;
	newCommand->peer.dsa_rsp.dlQoSPriority = newDlCI.qos_;

	newCommand->magic.size = dsa_rspPDUSize_;


	/**********  Send Compound ********************/
	compoundQueue_.push_back(newCompound);
	doWakeup();
}



void
SetupConnectionBS::doOnDSA_ACK(const wns::ldk::CompoundPtr& compound)
{
	assure(this->getCommand(compound->getCommandPool())->peer.managementMessageType
		   ==  MACManagementMessage::DSA_ACK,
		   "wimac::controlPlane::SetupConnectionBS::doOnDSA_ACK: only accept DSA_ACK!\n");

	/*********  Extract information from command ************/
	ConnectionIdentifier::Ptr dsaCI;
	SetupConnectionCommand::TransactionID transactionID;

	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );
	dsaCI = friends_.connectionManager->getConnectionWithID(cCommand->peer.id);
	assure( dsaCI, "Need ConnectionIdetifier for Received DSA_ACK Compound! \n");

	transactionID = getCommand( compound->getCommandPool() )
		->peer.dsa_ack.transactionID;

	// Everythink ok?
	assure( dsaCI && transactionID,
			"wimac::SetupConnectionBS::doOnDSA_ACK: DSA_ACK wron content! \n" );

	/********* Remove entries from waitingForSsDSA_ACK_ *********************/
	for (WaitingForSsDSA_ACK::iterator it = waitingForSsDSA_ACK_.begin();
		 it != waitingForSsDSA_ACK_.end();)
	{
		WaitingForSsDSA_ACK::iterator copyIt = it;
		++it;  // Because of removing element from list.Otherwise it
		       // would be invalid before increasing;

		if (    ( (*copyIt).subscriberStation == dsaCI->subscriberStation_ )
				&& ( (*copyIt).transactionID == transactionID )
			)
		{
			copyIt = waitingForSsDSA_ACK_.erase(copyIt);
		}
	}
}




/*********************** SetupConnectionSS ********************************************/

SetupConnectionSS::SetupConnectionSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config ) :
	wns::ldk::CommandTypeSpecifier< SetupConnectionCommand >(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<>(),
	wns::Cloneable< SetupConnectionSS >(),
	wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
	remainTimerWaitingForRSP_(0),
	activeTransactionID_(0),
	highestTransactionID_(0),
	compoundQueue_(),
	callBackInterface_(NULL),
	timerWaitingForRSP_( config.get<ConnectionIdentifier::Frames>
						 ("timerWaitingForRSP") ),
    dsa_reqPDUSize_( config.get<Bit>("dsa_reqPDUSize") ),
	dsa_ackPDUSize_( config.get<Bit>("dsa_ackPDUSize") ),
	friends_()
{
	friends_.connectionManagerName = config.get<std::string>("connectionManager");
	friends_.connectionClassifierName = config.get<std::string>("connectionClassifier");
	friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");

	friends_.connectionManager = NULL;
	friends_.connectionClassifier = NULL;
	friends_.newFrameProvider = NULL;

}



void
SetupConnectionSS::start(ConnectionIdentifier::QoSCategory qosPriority,
						 SetupConnectionCallBackInterface* callBackInterface)
{
	assure (qosPriority,
			"wimac::SetupConnectionSS::setupConnection: Need qosPriority. \n");
	assure (callBackInterface,
			"wimac::SetupConnectionSS::setupConnection: Need callBackInterface! \n");

	LOG_INFO( getFUN()->getName(),": start SetupConnectionSS.");

	// Check if SetupConnectionSS is in use
	if(callBackInterface_)
	{
		LOG_INFO(getFUN()->getName(),": SetupConnection is used.");
		callBackInterface->resultSetupConnection(false);
		return;
	}

	callBackInterface_ = callBackInterface;

	if (highestTransactionID_ == 65535)
	{
		highestTransactionID_ = 1;
	}
	else
	{
		++highestTransactionID_;
	}

	activeTransactionID_ = highestTransactionID_;

	/********* Create DSA_REQ Compound ***********/
	wns::ldk::CompoundPtr newCompound;
	newCompound = wns::ldk::CompoundPtr(
		new wns::ldk::Compound( getFUN()->getProxy()->createCommandPool() ) );

	wns::ldk::ClassifierCommand* cCommand;
	ConnectionIdentifier::Ptr pmCI;

	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->activateCommand(newCompound->getCommandPool()) );
	pmCI = friends_.connectionManager->getPrimaryConnectionFor(
		ConnectionIdentifier::StationID(
			dynamic_cast<dll::Layer2*>( getFUN()->getLayer() )->getID() )
		);
	assure( pmCI,
			"SetupConnectionSS::start: Didn't get ConnectionIdentifier from ConnectionManager!");
	cCommand->peer.id = pmCI->cid_;

	this->activateCommand( newCompound->getCommandPool() );
	this->getCommand( newCompound->getCommandPool() )->
		peer.managementMessageType = MACManagementMessage::DSA_REQ;

	this->getCommand( newCompound->getCommandPool() )->
		peer.dsa_req.transactionID = activeTransactionID_;
	this->getCommand( newCompound->getCommandPool() )->
		peer.dsa_req.ulQoSPriority = qosPriority;
	this->getCommand( newCompound->getCommandPool() )->
		peer.dsa_req.dlQoSPriority = qosPriority;

	this->getCommand( newCompound->getCommandPool() )->
		magic.size = dsa_reqPDUSize_;


	/********* Send Compound  ***********/
	compoundQueue_.push_back(newCompound);
	doWakeup();

} // start



void
SetupConnectionSS::doOnData(const wns::ldk::CompoundPtr& compound)
// get Management Messages Compound
{
	LOG_INFO( getFUN()->getName(),
			  ": Receiving management message:",
			  getCommand( compound->getCommandPool() )
			  ->peer.managementMessageType );

	SetupConnectionCommand* command;
	command = getCommand( compound->getCommandPool() );

	if ( command->peer.managementMessageType == MACManagementMessage::DSA_RSP )
	{
		this->doOnDSA_RSP(compound);
	} else
	{
		assure(0 ,"SetupConnectionSS::doOnData: managementMessageType is unknown type");
	}

} //doOnData



void
SetupConnectionSS::doWakeup()
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


		//Set timerWaitingForRSP
		if (getCommand( compound->getCommandPool() )->peer.managementMessageType
			== MACManagementMessage::DSA_REQ)
		{
			remainTimerWaitingForRSP_ = timerWaitingForRSP_;
			friends_.newFrameProvider->attachObserver(this);
		}

		// Remove compound from queue and send
		compoundQueue_.remove(compound);
		getConnector()->getAcceptor(compound)->sendData(compound);
	}
} // doWakeup



void
SetupConnectionSS::onFUNCreated()
{
	assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
		   wns::service::dll::StationTypes::AP(),
		   "wimac::SetupConnectionSS: Station is not a subscriber station! \n" );

	friends_.connectionManager =getFUN()->getLayer()->
		getManagementService<service::ConnectionManager>(friends_.connectionManagerName);


	friends_.connectionClassifier = getFUN()
		->findFriend<wns::ldk::FunctionalUnit*>(friends_.connectionClassifierName);

	friends_.newFrameProvider = getFUN()
		->findFriend<wns::ldk::fcf::NewFrameProvider*>(friends_.newFrameProviderName);

} // onFUNCreated()



void
SetupConnectionSS::messageNewFrame()
{
	if ( remainTimerWaitingForRSP_ <= 0 )
	{
		LOG_INFO(getFUN()->getName(), " timerWaitingForRSP run out!");
		friends_.newFrameProvider->detachObserver(this);
		remainTimerWaitingForRSP_ = 0;
		//activeTransactionID_ = 0;
		this->result( false );
	}
	--remainTimerWaitingForRSP_;

}




///////////////// privat Functions //////////////////

void
SetupConnectionSS::doOnDSA_RSP(const wns::ldk::CompoundPtr& compound)
{
	assure(getCommand(compound->getCommandPool())->peer.managementMessageType
		   ==  MACManagementMessage::DSA_RSP,
		   "wimac::controlPlane::doOnDSA_RSP: only accept DSA_RSP! \n");


	/******* Has the waitingForRSP timer run out? **********/
	if ( remainTimerWaitingForRSP_ <= 0 )
		return;

	wimac::controlplane::SetupConnectionCommand* command;
	command = getCommand( compound->getCommandPool() );

	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );


	/*********  Extract information from command ************/
	ConnectionIdentifier::Ptr dsaCI;
	SetupConnectionCommand::TransactionID transactionID;
	ConnectionIdentifier::CID ulCID;
	ConnectionIdentifier::QoSCategory ulQoSPriority;
	ConnectionIdentifier::CID dlCID;
	ConnectionIdentifier::QoSCategory dlQoSPriority;

	dsaCI = friends_.connectionManager->getConnectionWithID( cCommand->peer.id );
	assure( dsaCI,
			"SetupConnectionSS::doOnDSA_RSP: Didn't get ConnectionIdentifier from ConnectionManager for that compound CID!");

	transactionID = this->getCommand( compound->getCommandPool() )
		->peer.dsa_rsp.transactionID;
	ulCID = this->getCommand( compound->getCommandPool() )
		->peer.dsa_rsp.ulCID;
	ulQoSPriority = this->getCommand( compound->getCommandPool() )
		->peer.dsa_rsp.ulQoSPriority;
	dlCID = this->getCommand( compound->getCommandPool() )
		->peer.dsa_rsp.dlCID;
	dlQoSPriority = this->getCommand( compound->getCommandPool() )
		->peer.dsa_rsp.dlQoSPriority;

	// Everythink ok?
	assure(ulCID && ulQoSPriority && dlCID && dlQoSPriority,
		   "wimac::SetupConnectionBS::DATAind: DSA_RSP wron content! \n" );

	//Reset timerWaitingForRSP
	remainTimerWaitingForRSP_ = 0;
	friends_.newFrameProvider->detachObserver(this);


	/*********** Is it the right SetupConnection process *****/
	if ( transactionID != activeTransactionID_ )
		return;


	/*********** Create response DSA_ACK *********************/
	wns::ldk::CompoundPtr newCompound;
	newCompound = wns::ldk::CompoundPtr(
		new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

	service::ConnectionManager::ConnectionIdentifierPtr bCI ( friends_.connectionManager
        ->getBasicConnectionFor( ConnectionIdentifier::StationID(
                                     dynamic_cast<Component*>(getFUN()->getLayer())
                                     ->getID() ) ) );
	assure(bCI->valid_,
		   "SetupConnectionSS::doOnDSA_RSP: Didn't get ConnectionIdentifier from ConnectionManager!");
	friends_.connectionClassifier->activateCommand( newCompound->getCommandPool() );
	dynamic_cast<wns::ldk::ClassifierCommand*>
		( friends_.connectionClassifier->getCommand( newCompound->getCommandPool() ) )
		->peer.id = bCI->cid_;

	this->activateCommand( newCompound->getCommandPool() );
	this->getCommand( newCompound->getCommandPool() )->
		peer.managementMessageType = MACManagementMessage::DSA_ACK;
	this->getCommand( newCompound->getCommandPool() )->
		peer.dsa_ack.transactionID = transactionID;

	this->getCommand( newCompound->getCommandPool() )->
		magic.size = dsa_ackPDUSize_;


	/************ Send Compound ****************************/
	compoundQueue_.push_front(newCompound);
	doWakeup();


	/********* Create connection identifiers for UL & DL and append them *******/
	ConnectionIdentifier newUlCI(
		dsaCI->baseStation_,
		ulCID,
		dsaCI->subscriberStation_,
		dsaCI->subscriberStation_,
		ConnectionIdentifier::Data,
		ConnectionIdentifier::Uplink,
		ulQoSPriority);

    ConnectionIdentifier newDlCI(
		dsaCI->baseStation_,
		dlCID,
		dsaCI->subscriberStation_,
		dsaCI->subscriberStation_,
		ConnectionIdentifier::Data,
		ConnectionIdentifier::Downlink,
		dlQoSPriority);

	friends_.connectionManager->appendConnection(newUlCI);
	friends_.connectionManager->appendConnection(newDlCI);


	/********* return result ******************************************/
	this->result( true );
}



void
SetupConnectionSS::result(bool result)
{
	LOG_INFO( getFUN()->getName(),": Stop SetupConnection with result:", result);

	/********* Call the callBackInterface *****************/
	SetupConnectionCallBackInterface* callBackInterface = callBackInterface_;
	callBackInterface_ = NULL;
	callBackInterface->resultSetupConnection(result);
}




