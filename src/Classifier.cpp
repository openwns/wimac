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

#include <WIMAC/Classifier.hpp>

#include <WNS/service/dll/Address.hpp>

#include <DLL/UpperConvergence.hpp>
#include <DLL/StationManager.hpp>
#include <DLL/Layer2.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/ConnectionRule.hpp>

#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/ConnectionRule.hpp>
#include <WIMAC/Utilities.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::ConnectionClassifier,
	wns::ldk::FunctionalUnit,
	"wimac.ConnectionClassifier",
	wns::ldk::FUNConfigCreator);

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::ClassifierMock,
	wns::ldk::FunctionalUnit,
	"wimac.ClassifierMock",
	wns::ldk::FUNConfigCreator);


using namespace wimac;


/********** ConnectionClassifier ***********************************************/

ConnectionClassifier::ConnectionClassifier( wns::ldk::fun::FUN* fun,
											const wns::pyconfig::View& /* config*/ )
	: wns::ldk::CommandTypeSpecifier< wns::ldk::ClassifierCommand >(fun)
{}



void ConnectionClassifier::doOnData(const wns::ldk::CompoundPtr& compound)
{
	if( classifyIncoming(compound) == -1 ) //Invalid Compound, throw away
		return;

	getDeliverer()->getAcceptor(compound)->onData(compound);
}



void ConnectionClassifier::doSendData(const wns::ldk::CompoundPtr& compound)
{

    //Activate command and allocate classify id
	assure( !( getFUN()->getProxy()
			   ->commandIsActivated( compound->getCommandPool(), this ) ),
			" wimac::ConnectionClassifier::processOutgoing: ConnectionClassifier shouldn't get activated command! \n");

	wns::ldk::ClassifierCommand* command = NULL;
	command = activateCommand( compound->getCommandPool() );
	command->peer.id = classifyOutgoing( compound);

	getConnector()->getAcceptor(compound)->sendData(compound);
}



void ConnectionClassifier::onFUNCreated()
{
	friends_.upperConvergence = getFUN()->findFriend<dll::UpperConvergence*>("upperConvergence");
	friends_.connectionManager =
		getFUN()->getLayer()->getManagementService<service::ConnectionManager>
		("connectionManager");

	assureType( getFUN()->getLayer(), wimac::Component* );
	friends_.layer = dynamic_cast<wimac::Component*>( getFUN()->getLayer() );
}



wns::ldk::CommandPool*
ConnectionClassifier::createReply(const wns::ldk::CommandPool* original) const
{
	wns::ldk::CommandPool* newCommand = getFUN()->getProxy()->createReply(original, this);
	wns::ldk::ClassifierCommand* newClassifierCommand = activateCommand( newCommand );
	wns::ldk::ClassifierCommand* classifierCommand = getCommand( original );
	newClassifierCommand->peer.id = classifierCommand->peer.id;
	return newCommand;
}



wns::ldk::ClassificationID
ConnectionClassifier::classifyIncoming( const wns::ldk::CompoundPtr& compound )
{
	wns::ldk::ClassifierCommand* command;
	command = getCommand( compound->getCommandPool() ) ;

	dll::UpperCommand* ucCommand =
		friends_.upperConvergence->getCommand(compound->getCommandPool());


	LOG_INFO( getFUN()->getLayer()->getName(), ": classify incomming compound.   CID: ",
			  command->peer.id, " to destMACAdr: ", ucCommand->peer.targetMACAddress );

	ConnectionIdentifier::Ptr ci
		( friends_.connectionManager->getConnectionWithID(command->peer.id) );

	if( !ci )
	{
		std::stringstream error;
		error << "No ConnectionIdentifier registered with ID " << command->peer.id;
		throw wns::Exception( error.str() );
	}

	ConnectionIdentifier::StationID sourceID = -1;
	if( ci->subscriberStation_ == dynamic_cast<dll::Layer2*>(getFUN()->getLayer())->getID() )
		sourceID = ci->baseStation_;
	if( ci->baseStation_ == dynamic_cast<dll::Layer2*>(getFUN()->getLayer())->getID() )
		sourceID = ci->remoteStation_;

	return command->peer.id;
}



wns::ldk::ClassificationID
ConnectionClassifier::classifyOutgoing( const wns::ldk::CompoundPtr& compound )
{
	dll::UpperCommand* ucCommand =
		friends_.upperConvergence->getCommand(compound->getCommandPool());

	service::ConnectionManager::ConnectionIdentifiers cis;
	cis = friends_.connectionManager->getOutgoingDataConnections(
		ucCommand->peer.targetMACAddress.getInteger() );


	if ( cis.empty() )
	{
		if(friends_.layer->getStationType() == wns::service::dll::StationTypes::UT() ||
		   friends_.layer->getStationType() == wns::service::dll::StationTypes::RUT() )
		{
			//FIXME If no CI exist for this target MACAdress, it should start
			//      controlplane::SetupConnection or controlplaneSimple::SetupConnection
			//      to creat an dataconnection
			cis = friends_.connectionManager
				->getAllDataConnections( ConnectionIdentifier::Uplink );
		}

		assure( !cis.empty(),
				"ConnectionClassifier::processOutgoing: No connections found for destination");
	}

	assure( cis.size() == 1, "ConnectionClassifier::classifyOutgoing: Only on ConnectionIdentifier to target MacAdress is at the moment allowed. \n");

	LOG_INFO( getFUN()->getName(), ": classify outgoing Compound.   destMACAdr: ",
			  ucCommand->peer.targetMACAddress, " to CID: ",
			  ( *cis.begin() )->cid_ );

	return (*cis.begin())->cid_;

}



/*****************    private functions    ****************/

bool
ConnectionClassifier::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
	dll::UpperCommand* ucCommand;
	service::ConnectionManager::ConnectionIdentifiers cis;

	ucCommand = friends_.upperConvergence->getCommand(compound->getCommandPool());


	// try to get ConnectionIdentifier for this compound
	cis = friends_.connectionManager->getOutgoingDataConnections(
		ucCommand->peer.targetMACAddress.getInteger() );

	if( cis.empty() )
	{
		if(friends_.layer->getStationType() == wns::service::dll::StationTypes::UT() ||
		   friends_.layer->getStationType() == wns::service::dll::StationTypes::RUT() )
		{
			cis = friends_.connectionManager
				->getAllDataConnections( ConnectionIdentifier::Uplink );
		}
	}

	// return true if CI for this compound exist and lower FU isAccepting
	if (  !cis.empty()
		&& getConnector()->hasAcceptor(compound) )
	{
		return true;
	}else
	{
		return false;
	}

}

