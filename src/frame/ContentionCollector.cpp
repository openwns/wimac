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

#include <WIMAC/frame/ContentionCollector.hpp>

#include <WNS/ldk/fcf/FrameBuilder.hpp>
#include <WNS/Exception.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <DLL/StationManager.hpp>
#include <WIMAC/Classifier.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/PhyUserCommand.hpp>
#include <WIMAC/PhyUser.hpp>
#include <WIMAC/Utilities.hpp>
#include <WIMAC/parameter/PHY.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::frame::ContentionCollector,
	wns::ldk::FunctionalUnit,
	"wimac.frame.ContentionCollector",
	wns::ldk::FUNConfigCreator );


using namespace wimac;
using namespace wimac::frame;


ContentionCollector::ContentionCollector( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config ) :
	wns::ldk::fcf::CompoundCollector( config ),
	wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>(fun),
	scheduler::PDUWatchProvider(fun),
	accepting_(false),
	backOff_(-1),
	maximumDuration_(0),
	accumulatedDuration_(0.0),
	compounds_(),
	layer_(NULL),
	phyMode(wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
		(wns::service::phy::phymode::createPhyMode( config.getView("phyMode") ) ) ),
	contentionAccess_()
{
	friends_.classifier_ = NULL;
	friends_.phyUser_ = NULL;


	wns::pyconfig::View configCA = wns::pyconfig::View( config, "contentionAccess");
	contentionAccess_.enabled = configCA.get<bool>("enabled");
	if(contentionAccess_.enabled)
	{
		contentionAccess_.slotLengthInSymbols = configCA.get<int>("slotLengthInSymbols");
		contentionAccess_.numberOfSlots = configCA.get<int>("numberOfSlots");
	} else
	{
		contentionAccess_.slotLengthInSymbols = 0;
		contentionAccess_.numberOfSlots = 0;
	}

}

void ContentionCollector::doStartCollection(int)
{
	assure( compounds_.empty(), "Queue in compoundCollector is not empty");

	if(this->getMaximumDuration() == 0)
		return;  //This CompoundCollector isn't in use

	if( contentionAccess_.enabled )
	{
		// first check the contention access configuration
		if( (contentionAccess_.numberOfSlots*contentionAccess_.slotLengthInSymbols* parameter::ThePHY::getInstance()->getSymbolDuration()) > this->getMaximumDuration() )
			throw wns::Exception("wimac::frame::ContentionCollector::ContentionCollector: Defined ranging slots are longer than the total ranging frame phase\n");
		if( contentionAccess_.numberOfSlots*contentionAccess_.slotLengthInSymbols*parameter::ThePHY::getInstance()->getSymbolDuration() < this->getMaximumDuration() - 1e-9 )
			throw wns::Exception("wimac::frame::ContentionCollector::ContentionCollector: Defined ranging slots are too short for this ranging frame phase. They are wasting the frame resource.\n");


		if( (backOff_ >= 0) && (backOff_ < contentionAccess_.numberOfSlots) )
		{
			accepting_ = true;
			accumulatedDuration_ =  backOff_ * contentionAccess_.slotLengthInSymbols
				* parameter::ThePHY::getInstance()->getSymbolDuration();
			maximumDuration_ = (backOff_+1) * contentionAccess_.slotLengthInSymbols
				* parameter::ThePHY::getInstance()->getSymbolDuration();

			assure( (maximumDuration_ - 1e-13 <= this->getMaximumDuration()),
					"ContentionCollector::startCollection: maximumDuration for this slot is longer than maximumDuration for total Phase " );
			backOff_ = -1;
			getReceptor()->wakeup();
		}else
		{
			accumulatedDuration_ = 0.0;
			maximumDuration_ = this->getMaximumDuration();
			accepting_ = false;
			backOff_ -=  contentionAccess_.numberOfSlots;
		}

	} else
	{
		accumulatedDuration_ = 0.0;
		maximumDuration_ = this->getMaximumDuration();
		accepting_ = true;
		getReceptor()->wakeup();
	}
}

void ContentionCollector::doOnData( const wns::ldk::CompoundPtr& compound )
{
	getDeliverer()->getAcceptor( compound )->onData( compound );
}

bool ContentionCollector::doIsAccepting( const wns::ldk::CompoundPtr& compound ) const
{
	if (!accepting_)
		return false;

	wns::ldk::ClassifierCommand* cCommand =
		friends_.classifier_->getCommand( compound->getCommandPool() );
	ConnectionIdentifier::Ptr ci
		= friends_.connectionManager->getConnectionWithID(cCommand->peer.id);
	if( ci->ciNotListening_ > 0)
		return false;


	if ( (getDuration(compound) + accumulatedDuration_) < maximumDuration_ )
	{
		return true;
	}
	return false;
}

simTimeType
ContentionCollector::getDuration(const wns::ldk::CompoundPtr& compound) const
{
	double dataRate =
		phyMode->getDataRate();

	Bit compoundSize =
		compound->getLengthInBits() + getFrameBuilder()->getOpcodeSize();
	return compoundSize / dataRate;
}

void ContentionCollector::doSendData( const wns::ldk::CompoundPtr& compound )
{
	wns::ldk::ClassifierCommand* clcom =
		friends_.classifier_->getCommand( compound->getCommandPool() );
	ConnectionIdentifier::Ptr connection =
		friends_.connectionManager->getConnectionWithID( clcom->peer.id );
	Component::StationIDType destinationID = 0;

	std::string stationType = wns::service::dll::StationTypes::toString( layer_->getStationType() );
	// das geht auch "stringfrei": switch(layer_->getStationType()) { ...
	if( stationType == "AP" )
	{
		destinationID = connection->subscriberStation_;
	}
	else if( stationType == "UT"||  stationType == "RUT" )
	{
		destinationID = connection->baseStation_;
	}
	else if(  stationType == "FRS")
	{
		/** \todo In relay stations the direction of a compound can not be
		 *	evaluated here. Since we only use the ContentionCollector in
		 *	DLScheduler in RelayStations the subscriber is set hardcoded as
		 *  destination for all compounds.
		 */
		destinationID = connection->subscriberStation_;
	}
	else
	{
		assure( 0, "unknown or unsupported station type" );
	}

	PhyUserCommand* phyUserCommand =
		friends_.phyUser_->activateCommand( compound->getCommandPool() );

	OmniUnicastPhyAccessFunc* func = new OmniUnicastPhyAccessFunc;

	func->destination_ =
		dynamic_cast<wimac::Component*>(getFUN()->getLayer())
		->getStationManager()->getStationByID( destinationID )->getNode();

	simTimeType duration = getDuration( compound );

	func->transmissionStart_ =
		accumulatedDuration_ + Utilities::getComputationalAccuracyFactor();
	func->transmissionStop_ =
		accumulatedDuration_ + duration;
	assureNotNull(phyMode.getPtr());
	func->phyMode_ = phyMode;

	accumulatedDuration_ += duration;

	phyUserCommand->local.pAFunc_.reset( func );
	phyUserCommand->peer.cellID_ = layer_->getCellID();
	phyUserCommand->peer.source_ = layer_->getNode();
	phyUserCommand->peer.phyModePtr = phyMode;
	phyUserCommand->magic.sourceComponent_ = layer_;
	phyUserCommand->magic.contentionAccess_ = contentionAccess_.enabled;

	LOG_INFO(getFUN()->getName(),": setting destination of compound to: ",
			 dynamic_cast<wimac::Component*>(getFUN()->getLayer())
			 ->getStationManager()
			 ->getStationByID( destinationID )
			 ->getName() );

	this->pduWatch( compound );

	compounds_.push_back( compound );
}

void ContentionCollector::doStart(int)
{
	double phaseStartTime =
		wns::simulator::getEventScheduler()->getTime();

	setTimeout( getCurrentDuration() );

	while ( !compounds_.empty() )
	{
		wns::ldk::CompoundPtr compound = compounds_.front();
		PhyUserCommand* phyUserCommand =
			friends_.phyUser_->getCommand( compound->getCommandPool() );
		OmniUnicastPhyAccessFunc* func =
			dynamic_cast<OmniUnicastPhyAccessFunc*>(phyUserCommand->local.pAFunc_.get());
		func->transmissionStart_ += phaseStartTime;
		func->transmissionStop_ += phaseStartTime;

		if ( getConnector()->
			 hasAcceptor( compounds_.front() ) )
		{
			getConnector()->
				getAcceptor( compounds_.front() )->sendData( compounds_.front() );
			compounds_.pop_front();
		}
		else
		{
			throw wns::Exception(
			   "Lower FU is not accepting the compound but is supposed to");
		}
	}
}

void ContentionCollector::onTimeout()
{
	getFrameBuilder()->finishedPhase( this );
}


void ContentionCollector::onFUNCreated()
{
	wns::ldk::fcf::CompoundCollector::onFUNCreated();

	assureType(getFUN()->getLayer(), wimac::Component*);
	layer_ = dynamic_cast<wimac::Component*>(getFUN()->getLayer());

	friends_.classifier_ =
		getFUN()->findFriend<wimac::ConnectionClassifier*>("classifier");
	friends_.phyUser_ =
		getFUN()->findFriend<wimac::PhyUser*>("phyUser");
	CompoundCollector::onFUNCreated();

	friends_.connectionManager = layer_->getManagementService
		<service::ConnectionManager>("connectionManager");
}

void ContentionCollector::setBackOff(int backOff)
{
	assure(contentionAccess_.enabled,
		   " ContentionCollector::setBackOff: Functionality is only useable if contentionAccess is enabled!\n");

	if ( backOff_ >= 0) // backOff is already set
	{
		LOG_INFO(getFUN()->getName(),
				 ": backOff is already been set! Remain backOff: ", backOff_);
		return;
	}

	LOG_INFO(getFUN()->getName(),": Set backOff to: ", backOff);
	backOff_ = backOff;
}


