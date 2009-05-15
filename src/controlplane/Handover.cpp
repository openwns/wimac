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

#include <WIMAC/controlplane/Handover.hpp>
#include <WIMAC/Classifier.hpp>
#include <WNS/service/dll/StationTypes.hpp>

using namespace wimac::controlplane;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::controlplane::HandoverBS,
	wns::ldk::FunctionalUnit,
	"wimac.controlplane.HandoverBS",
	wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::controlplane::HandoverSS,
	wns::ldk::FunctionalUnit,
	"wimac.controlplane.HandoverSS",
	wns::ldk::FUNConfigCreator);



/*************************** HandoverBS ***************************************/

HandoverBS::HandoverBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
	wns::ldk::CommandTypeSpecifier< HandoverCommand >(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<>(),
	wns::Cloneable< HandoverBS >(),
	wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
	ssInHandoverProcess_(),
	doMOB_HO_IND_(),
	compoundQueue_(),
	timerWaitingForIND_( config.get<int>("timerWaitingForIND") ),
	mob_bsho_rspPDUSize_( config.get<Bit>("mob_bsho_rspPDUSize") ),
	friends_()
{
	friends_.connectionManagerName = config.get<std::string>("connectionManager");
	friends_.connectionClassifierName
		= config.get<std::string>("connectionClassifier");
	friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");

	friends_.connectionManager = NULL;
	friends_.connectionClassifier = NULL;
	friends_.newFrameProvider = NULL;

}



void
HandoverBS::doOnData(const wns::ldk::CompoundPtr& compound)
{
	LOG_INFO( getFUN()->getName(),
			  ": Receiving management message:",
			  getCommand( compound->getCommandPool() )
			  ->peer.managementMessageType );

	HandoverCommand* command;
	command = getCommand( compound->getCommandPool() );


	if ( command->peer.managementMessageType
		 == MACManagementMessage::MOB_MSHO_REQ )
	{
		this->doOnMOB_MSHO_REQ(compound);
	}
	else if (command->peer.managementMessageType
			 == MACManagementMessage::MOB_HO_IND)
	{
		this->doOnMOB_HO_IND(compound);
	}else
	{
		assure(0 ,"HandoverBS::doOnData: managementMessageType is unknown type!\n");
	}

} //DATAind



void
HandoverBS::doWakeup()
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
} // wakeup



void
HandoverBS::onFUNCreated()
{
	assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
		   wns::service::dll::StationTypes::UT(),
		   "wimac::HandoverBS: Station is not a BS! \n" );


	friends_.connectionClassifier = getFUN()
		->findFriend<wimac::ConnectionClassifier*>(friends_.connectionClassifierName);

	friends_.connectionManager = dynamic_cast<wimac::Component*>
		(getFUN()->getLayer())->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

	friends_.newFrameProvider = getFUN()
		->findFriend<wns::ldk::fcf::NewFrameProvider*>
		(friends_.newFrameProviderName);

	friends_.newFrameProvider->attachObserver(this);

} // onFUNCreated()



void HandoverBS::messageNewFrame()
{
	/******** Decrease ssInHandoverProcess **************/
	for(SSInHandoverProcess::iterator it = ssInHandoverProcess_.begin();
		it != ssInHandoverProcess_.end(); ++it)
	{
		if ((*it).second <= 0)
		{
			LOG_INFO( getFUN()->getName(),
					  ": Timer waiting for MOB_HO_IND has run out for station:",
					  (*it).first );
		} else
		{
			--((*it).second);
		}
	}


	/******** doMOB_HO_IND - delete ConnectionIdentifier from SS **************/
	for( std::list<ConnectionIdentifier::StationID>::iterator
			 it = doMOB_HO_IND_.begin(); it != doMOB_HO_IND_.end(); )
	{
		// Delete all ConnectionIdentifiers for Subscriber Station
	    ConnectionIdentifier::List cis;
		cis = friends_.connectionManager->getAllCIForSS( *it );
		assure( !cis.empty(),
				"HandoverBS::doOnMOB_HO_IND: didn't get any ConnectionIdentifiers!\n");

		for(service::ConnectionManager
				::ConnectionIdentifiers::const_iterator it2	= cis.begin();
			it2 !=cis.end(); ++it2)
		{
			friends_.connectionManager->deleteCI( (*it2)->cid_ );
		}

		doMOB_HO_IND_.erase(it++);
	}


} //messageNewFrame()



////////////// Privat functions /////////////////

void
HandoverBS::doOnMOB_MSHO_REQ(const wns::ldk::CompoundPtr& compound)
{
	assure(this->getCommand(compound->getCommandPool())->peer.managementMessageType
		   ==  MACManagementMessage::MOB_MSHO_REQ,
		   "wimac::Handover::doOnMOB_MSHO_REQ: only accept MOB_MSHO_REQ!  \n");

	HandoverCommand* command;
	command = getCommand( compound->getCommandPool() );
	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

	/*********  Extract information from command ************/
	HandoverCommand::TransactionID transactionID;
	service::handoverStrategy::Interface::Stations targetBaseStations;
	ConnectionIdentifier::StationID subscriberStation;
	ConnectionIdentifier::Ptr ci;
	ci = friends_.connectionManager->getConnectionWithID(cCommand->peer.id);
	assure(ci,
		   "HandoverBS::doOnMOB_MSHO_REQ: Didn't get ConnectionIdentifier from ConnectionManager for the CID of that compound!");

	transactionID = command->peer.mob_msho_req.transactionID;
	targetBaseStations = command->peer.mob_msho_req.targetBaseStations;

	subscriberStation =  ci->subscriberStation_;

	// Everythink ok?
	assure( !targetBaseStations.empty(),
			"wimac::Handover::DATAind: Handover need targetBaseStation from SuscriberStation! \n" );

/*

No intelligence for the Response Message (ackBaseStations) implemented in the moment!

*/

	/*********** Create response MOB_BSHO_RSP *********************/
  	wns::ldk::CompoundPtr newCompound;
	newCompound = wns::ldk::CompoundPtr(
		new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));
	service::ConnectionManager::ConnectionIdentifierPtr bCI (
		friends_.connectionManager->getBasicConnectionFor( subscriberStation ) );
	assure(bCI->valid_,
		   "HandoverBS::doOnMOB_MSHO_REQ: Didn't get ConnectioMessageExchanger::doOnData(nIdentifier from ConnectionManager!");

	wns::ldk::ClassifierCommand* newCCommand;
	newCCommand = friends_.connectionClassifier
		->activateCommand( newCompound->getCommandPool() );
	newCCommand->peer.id = bCI->cid_;

	HandoverCommand* newCommand;
	newCommand = activateCommand( newCompound->getCommandPool() );
	newCommand->peer.managementMessageType = MACManagementMessage::MOB_BSHO_RSP;
	newCommand->peer.mob_bsho_rsp.transactionID = transactionID;
	newCommand->peer.mob_bsho_rsp.ackBaseStations = targetBaseStations;
	newCommand->magic.size = mob_bsho_rspPDUSize_;

	// Stating Station in Handover Process, timerWaitingForIND
	ssInHandoverProcess_[subscriberStation] = timerWaitingForIND_;

    // Send Compound
	compoundQueue_.push_back(newCompound);
	doWakeup();

} // doOnMOB_MSHO_REQ



void
HandoverBS::doOnMOB_HO_IND(const wns::ldk::CompoundPtr& compound)
{
	assure(this->getCommand(compound->getCommandPool())->peer.managementMessageType
		   ==  MACManagementMessage::MOB_HO_IND,
		   "wimac::Handover::doOnMOB_HO_IND: only accept MOB_HO_IND!  \n");

	HandoverCommand* command;
	command = getCommand( compound->getCommandPool() );
	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );

	ConnectionIdentifier::Ptr ci;
	ci = friends_.connectionManager->getConnectionWithID(cCommand->peer.id);
	assure( ci,
			"HandoverBS::doOnMOB_HO_IND: Didn't get ConnectionIdnetifier from ConnectionManager for CID of that compound!");


	ConnectionIdentifier::StationID subscriberStation;
	subscriberStation = ci->subscriberStation_;

	//get timer for this subscriberStation
	SSInHandoverProcess::iterator it = ssInHandoverProcess_.begin();
	for(;it != ssInHandoverProcess_.end(); ++it)
	{
		if((*it).first == subscriberStation)
		{
			break;
		}
	}


	// have we found the subscriberStation
	if( it != ssInHandoverProcess_.end() )
	{
		// Is the timer waiting for IND runningt
		if( (*it).second > 0 )
		{
			doMOB_HO_IND_.push_back( subscriberStation );

		} else
		{
			LOG_INFO( getFUN()->getName(),
					  ": Timer waiting for MOB_HO_IND has run out for station: ",
				  subscriberStation);
		}

		// remove timer for this station
		ssInHandoverProcess_.erase(it);

	}else
	{
		LOG_INFO( getFUN()->getName(),
				  ": no such station in handover process.    stationID: ",
				  subscriberStation);
		assure(0, " No such station in handover process!\n");

	}


} // doOnMOB_HO_IND




/*********************** HandoverSS ********************************************/

HandoverSS::HandoverSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
	wns::ldk::CommandTypeSpecifier< HandoverCommand >(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<>(),
	wns::ldk::HasDeliverer<>(),
	wns::Cloneable< HandoverSS >(),
	wns::ldk::fcf::NewFrameObserver(dynamic_cast<wns::ldk::FunctionalUnit*>(this)->getName()),
	wimac::scheduler::PDUWatchObserver(),
	activeTransactionID_(0),
	highestTransactionID_(0),
	remainTimerWaitingForRSP_(-1),
	doMOB_HO_IND_(),
	compoundQueue_(),
	callBackInterface_(NULL),
	ackBaseStations_( service::handoverStrategy::Interface::Stations() ),
	timerWaitingForRSP_( config.get<int>("timerWaitingForRSP")),
	mob_msho_reqPDUSize_( config.get<Bit>("mob_msho_reqPDUSize") ),
	mob_ho_indPDUSize_( config.get<Bit>("mob_ho_indPDUSize") ),
	friends_()
{
	friends_.connectionClassifierName = config.get<std::string>("connectionClassifier");
	friends_.newFrameProviderName = config.get<std::string>("newFrameProvider");
    friends_.connectionManagerName = config.get<std::string>("connectionManager");
	friends_.pduWatchProviderName = config.get<std::string>("pduWatchProvider");

	friends_.connectionClassifier = NULL;
	friends_.newFrameProvider = NULL;
	friends_.connectionManager = NULL;
	friends_.pduWatchProvider = NULL;

}



void
HandoverSS::start(service::handoverStrategy::Interface::Stations targetBaseStations,
				  HandoverCallBackInterface* callBackInterface)
{
	assure (!targetBaseStations.empty(),
			"wimac::HandoverSS::handover: Need target base station. \n");
	assure (callBackInterface,
			"wimac::HandoverSS::handover: Get no callBackInterface! \n");


	LOG_INFO( getFUN()->getName(),": start HandoverSS.");

    if(callBackInterface_)
	{
		LOG_INFO( getFUN()->getName(),": Handover is used.");
		callBackInterface->resultHandover( service::handoverStrategy::Interface::Stations());
		return;
	}

	callBackInterface_ = callBackInterface;
	ackBaseStations_ = service::handoverStrategy::Interface::Stations();


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


	/********* Create MOB_MSHO_REQ Compound ***********/
	wns::ldk::CompoundPtr newCompound;
	newCompound = wns::ldk::CompoundPtr(
		new wns::ldk::Compound( getFUN()->getProxy()->createCommandPool() ) );

	// ClassifierCommand( CID )
	ConnectionIdentifier::StationID ownStationID;
	ownStationID = dynamic_cast<Component*>( getFUN()->getLayer() )->getID();
	service::ConnectionManager::ConnectionIdentifierPtr basicCI
		= friends_.connectionManager->getBasicConnectionFor(ownStationID);
	assure(basicCI->valid_,
		   "HandoverSS::start: Didn't get ConnectionIdentifier from ConnectionManager!");

	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier
		->activateCommand( newCompound->getCommandPool() ) );
	cCommand->peer.id = basicCI->cid_; // basic CID

	// HandoverCommand
	HandoverCommand* command;
	command = activateCommand(newCompound->getCommandPool());
	command->peer.managementMessageType	= MACManagementMessage::MOB_MSHO_REQ;
	command->peer.mob_msho_req.transactionID = activeTransactionID_;
	command->peer.mob_msho_req.targetBaseStations = targetBaseStations;
	command->magic.size = mob_msho_reqPDUSize_;


	/********* Send Compound  ***********/
	compoundQueue_.push_back(newCompound);
	doWakeup();

}



void
HandoverSS::doOnData(const wns::ldk::CompoundPtr& compound)
{
	LOG_INFO( getFUN()->getName(),
			  ": Receiving management message:",
			  getCommand( compound->getCommandPool() )
			  ->peer.managementMessageType );

	HandoverCommand* command;
	command = this->getCommand( compound->getCommandPool() );

	if ( command->peer.managementMessageType == MACManagementMessage::MOB_BSHO_RSP )
	{
		this->doOnMOB_BSHO_RSP(compound);
	}else
	{
		assure(0 ,"HandoverSS::doOnData: managementMessageType is unknown type");
	}
}

void
HandoverSS::doWakeup()
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
			== MACManagementMessage::MOB_MSHO_REQ)
		{
			remainTimerWaitingForRSP_ = timerWaitingForRSP_;
		}


		// Remove compound from queue and send
		compoundQueue_.remove(compound);
		getConnector()->getAcceptor(compound)->sendData(compound);
	}
}

void
HandoverSS::onFUNCreated()
{
	assure(dynamic_cast<wimac::Component*>(getFUN()->getLayer())->getStationType() !=
		   wns::service::dll::StationTypes::AP(),
		   "wimac::HandoverSS: Station is not a subscriber station! \n" );


	friends_.connectionClassifier = getFUN()
		->findFriend<wimac::ConnectionClassifier*>
		(friends_.connectionClassifierName);

	friends_.newFrameProvider = getFUN()
		->findFriend<wns::ldk::fcf::NewFrameProvider*>
		(friends_.newFrameProviderName);

	friends_.connectionManager = dynamic_cast<wimac::Component*>
		(getFUN()->getLayer())->getManagementService<service::ConnectionManager>(friends_.connectionManagerName);

	friends_.pduWatchProvider = getFUN()
		->findFriend<scheduler::PDUWatchProvider*>
		(friends_.pduWatchProviderName);

	friends_.newFrameProvider->attachObserver(this);
}



void
HandoverSS::messageNewFrame()
{
	/******* run out timer waitingForRSP *************/
	if (remainTimerWaitingForRSP_ == 0)
	{
		// Reset timerWaitingForRSP
		remainTimerWaitingForRSP_ = -1;
		LOG_INFO( getFUN()->getName(), ": timerWaitingForRSP is running out!");

		this->result(service::handoverStrategy::Interface::Stations());
	}

	/************* decrease timer waitingForRSP *******************/
	if (remainTimerWaitingForRSP_ > 0)
		--remainTimerWaitingForRSP_;

	/********* Delete all ConnectionIdentifiers for this access point ****/
	for(std::list<wns::ldk::CommandPool>::iterator it = doMOB_HO_IND_.begin();
		it != doMOB_HO_IND_.end();)
	{
		HandoverCommand* command;
		command = this->getCommand( &(*it) );
		wns::ldk::ClassifierCommand* cCommand;
		cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
			friends_.connectionClassifier->getCommand( &(*it) ) );

		ConnectionIdentifier::Ptr ci;
		ci = friends_.connectionManager->getConnectionWithID( cCommand->peer.id );
		assure(ci,
			   "HandoverSS::messageNewFrame: Didn't get ConnectionIdentifier from ConnectionManager for that compound!");
		ConnectionIdentifier::StationID ap;
		ap = ci->baseStation_;
		service::ConnectionManager::ConnectionIdentifiers cis;
		cis = friends_.connectionManager->getAllCIForBS( ap );

		assure( !cis.empty(),
				"HandoverBS::doOnMOB_BSHO_RSP: didn't get any ConnPHY.Station ectionIdentifiers!\n");

		for(service::ConnectionManager::ConnectionIdentifiers::const_iterator
				it2	= cis.begin(); it2 !=cis.end(); ++it2)
		{
			friends_.connectionManager->deleteCI( (*it2)->cid_ );
		}

		doMOB_HO_IND_.erase(it++);

		this->result( ackBaseStations_ );
	}

}



void
HandoverSS::notifyPDUWatch( wns::ldk::CommandPool commandPool )
{
	HandoverCommand* command;
	command = this->getCommand( &commandPool );
	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( &commandPool ) );

	// This Compound isn't the right one
	if(command->peer.managementMessageType != MACManagementMessage::MOB_HO_IND)
	{
		LOG_INFO( getFUN()->getName(), ": notifyPDUWatch getting wrong PDU.");
		return;
	}

	friends_.pduWatchProvider->detachObserver(this);

	doMOB_HO_IND_.push_back(commandPool);

}




////////////// Privat functions /////////////////

void
HandoverSS::doOnMOB_BSHO_RSP(const wns::ldk::CompoundPtr& compound)
{
	assure(getCommand(compound->getCommandPool())->peer.managementMessageType
		   ==  MACManagementMessage::MOB_BSHO_RSP,
		   "wimac::HandoverSS::doOnMOB_MSHO_RSP: only accept MOB_BSHO_RSP!\n");

	if( remainTimerWaitingForRSP_ <= 0 ) // if timer run out, we don't accept
	{									 // anything
		LOG_INFO( getFUN()->getName(),
				  ": doOnMOB_BSHO_RSP remainTimerWaitingForRSP has run out. ");
		return;
	}

	HandoverCommand* command;
	command = this->getCommand( compound->getCommandPool() );
	wns::ldk::ClassifierCommand* cCommand;
	cCommand = dynamic_cast<wns::ldk::ClassifierCommand*>(
		friends_.connectionClassifier->getCommand( compound->getCommandPool() ) );


	/*********  Extract information from command ************/
	HandoverCommand::TransactionID transactionID;
	transactionID = command->peer.mob_bsho_rsp.transactionID;
	ackBaseStations_ = command->peer.mob_bsho_rsp.ackBaseStations;

	// Everythink ok?
	assure( !ackBaseStations_.empty(),
			"wimac::HandoverSS::doOnMOB_BSHO_RSP: need ackBaseStations from message source! \n" );

    // Is it the right transactionID?
	if ( transactionID != activeTransactionID_ )
		return;

	// reset Timer, management message is Received and ok
	remainTimerWaitingForRSP_ = -1;

	/*********** Create response MOB_HO_IND *********************/
	wns::ldk::CompoundPtr newCompound;
	newCompound = wns::ldk::CompoundPtr(
		new wns::ldk::Compound(getFUN()->getProxy()->createCommandPool()));

	wns::ldk::ClassifierCommand * newCCommand;
	newCCommand = 	dynamic_cast<wns::ldk::ClassifierCommand*>(friends_.connectionClassifier
		->activateCommand( newCompound->getCommandPool() ) );
	service::ConnectionManager::ConnectionIdentifierPtr bCI (
		friends_.connectionManager->getBasicConnectionFor(
		ConnectionIdentifier::StationID(
			dynamic_cast<Component*>(getFUN()->getLayer() )->getID() )) );
	assure(bCI->valid_,
		   "HandoverSS::doOnMOB_MSHO_RSP: Didn't get ConnectionIdentifier from ConnectionManager!");

	newCCommand->peer.id = bCI->cid_;

	this->activateCommand( newCompound->getCommandPool() );
	this->getCommand( newCompound->getCommandPool() )->
		peer.managementMessageType = MACManagementMessage::MOB_HO_IND;

/*

No intelligence for selecting on of the ackBaseStations!

 */

	this->getCommand( newCompound->getCommandPool() )
		->peer.mob_ho_ind.transactionID = transactionID;
	this->getCommand( newCompound->getCommandPool() )
		->peer.mob_ho_ind.newBaseStation = ackBaseStations_.begin()->id;
	this->getCommand( newCompound->getCommandPool() )
		->magic.size = mob_ho_indPDUSize_;

	// Send or queue Compound
	friends_.pduWatchProvider->attachObserver(this);
	compoundQueue_.push_back(newCompound);
	doWakeup();

} //doOnMOB_BSHO_RSP



void
HandoverSS::result(service::handoverStrategy::Interface::Stations ackBaseStations)
{
	if ( !ackBaseStations.empty() )
	{
		LOG_INFO( getFUN()->getName(), ": Handover succeed!");

			for (service::handoverStrategy::Interface::Stations::const_iterator it
					= ackBaseStations.begin();
				it != ackBaseStations.end(); ++it)
			{
				LOG_INFO( getFUN()->getName(),"           ackAP:",(*it).id,
						  "  f:", (*it).tune.frequency);
			}

	} else
	{
	   	LOG_INFO( getFUN()->getName(),
				  ": Handover doesn't get new target base Stations! ");
	}

	HandoverCallBackInterface* callBackInterface = callBackInterface_;
	callBackInterface_ = NULL;
	callBackInterface->resultHandover( ackBaseStations );

}




