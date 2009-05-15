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

#ifndef WIMAC_HANDOVER_HPP
#define WIMAC_HANDOVER_HPP

#include <WIMAC/Logger.hpp>
#include <WIMAC/ConnectionKey.hpp>
#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>

#include <WIMAC/MACHeader.hpp>
#include <WIMAC/CIRMeasureInterface.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/services/handoverStrategy/Interface.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/FlowSeparator.hpp>
#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>

namespace wimac{
	class ConnectionClassifier;

namespace controlplane{



/**
 *@todo (gra): It seems to be better, if only one management message Command exist
 *             for all ControlFUs. Members of the peer struct should be only the 
 *             ManagementMessageType and a Container with the specific message 
 *             informations.
 */
class HandoverCommand :
	public wns::ldk::Command
{
public:
	typedef uint16_t TransactionID;

	HandoverCommand()
		{
			peer.managementMessageType = MACManagementMessage::UCD;
			// UCD is wrong, but 0

			peer.mob_msho_req.transactionID = 0;
			peer.mob_msho_req.targetBaseStations = service::handoverStrategy::Interface::Stations();

			peer.mob_bsho_rsp.transactionID = 0;
			peer.mob_bsho_rsp.ackBaseStations = service::handoverStrategy::Interface::Stations();

			peer.mob_ho_ind.transactionID = 0;
			peer.mob_ho_ind.newBaseStation = 0;

			magic.size = 0;
		};

	~HandoverCommand()
		{
		};

	virtual
	Bit getSize() const
        {
			return magic.size;
        }

	struct {} local;

	struct {
		MACManagementMessage::ManagementMessageType managementMessageType;

		struct MOB_MSHO_REQ{
			///Unique identifier for this transaction
			TransactionID transactionID;
			service::handoverStrategy::Interface::Stations targetBaseStations;
		} mob_msho_req;

		struct MOB_BSHO_RSP{
			///Unique identifier for this transaction
			TransactionID transactionID;
			service::handoverStrategy::Interface::Stations ackBaseStations;
		} mob_bsho_rsp;

		struct MOB_HO_IND{
			///Unique identifier for this transaction
			TransactionID transactionID;
			service::handoverStrategy::Interface::StationID newBaseStation;
		} mob_ho_ind;

	} peer;

	struct {
		Bit size;
	} magic;

};



/*********** HandoverCallBackInterface *****************************************/
class HandoverCallBackInterface
{
public:
	virtual ~HandoverCallBackInterface()
		{
		}

	virtual void
	resultHandover(service::handoverStrategy::Interface::Stations newBaseStations) = 0;

};



/********************* HandoverBS ***********************************************/
    /**
	 * @brief HandoverBS implementation for message exchange in the Base Station.
	 *
     * - HandoverBS reacts on an MOB_MSHO_REQ() with an MOB_BSHO_RSP
     * - After reveicing an MOB_HO_IND it will determine all connections and
     *   compounds for that subscriber station
     *
	 */

class HandoverBS:
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier< HandoverCommand >,
	public wns::ldk::HasReceptor<>,
	public wns::ldk::HasConnector<>,
	public wns::ldk::HasDeliverer<>,
	public wns::Cloneable< HandoverBS >,
	public wns::ldk::fcf::NewFrameObserver
{
public:
	HandoverBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& );
	~HandoverBS(){};

	virtual bool
	doIsAccepting(const wns::ldk::CompoundPtr&) const
		{
			assure (0,
					"wimac::HandoverBS: doIsAcception is not in use! \n");
			return false;
		} // doIsAccepting



	virtual void
	doSendData(const wns::ldk::CompoundPtr&)
		{
			assure (0,
					"wimac::HandoverBS: doSendData is not in use! \n");
		} // doSendData

	virtual void
	doOnData(const wns::ldk::CompoundPtr& compound);

	virtual void
	doWakeup();

	virtual void
	onFUNCreated();

	virtual void
	messageNewFrame();

private:

	void doOnMOB_MSHO_REQ(const wns::ldk::CompoundPtr& compound);
	void doOnMOB_HO_IND(const wns::ldk::CompoundPtr& compound);

	typedef std::map<ConnectionIdentifier::StationID, ConnectionIdentifier::Frames>
	SSInHandoverProcess;
	SSInHandoverProcess ssInHandoverProcess_;

	std::list<ConnectionIdentifier::StationID> doMOB_HO_IND_;


	std::list<wns::ldk::CompoundPtr> compoundQueue_;


	//Static values from PyConfig
	ConnectionIdentifier::Frames timerWaitingForIND_;
	Bit mob_bsho_rspPDUSize_;

	struct{
		std::string connectionManagerName;
		std::string connectionClassifierName;
		std::string newFrameProviderName;

		service::ConnectionManager* connectionManager;
		wimac::ConnectionClassifier* connectionClassifier;
		wns::ldk::fcf::NewFrameProvider* newFrameProvider;
	} friends_;

};



/************** HandoverSS *****************************************************/
/**
* @brief HandoverSS implementation for message exchange in the
*        Subscriber Station.
*
* - Start HandoverSS with function ho()
* - Sends an MOB_MSHO_REQ()
* - Receives an MOB_BSHO_RSP()
* - React with an MOB_HO_IND()
+ - Delete all ConnectionIdentifiers
*
*/
class HandoverSS :
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier< HandoverCommand >,
	public wns::ldk::HasReceptor<>,
	public wns::ldk::HasConnector<>,
	public wns::ldk::HasDeliverer<>,
	public wns::Cloneable< HandoverSS >,
	public wns::ldk::fcf::NewFrameObserver,
	public wimac::scheduler::PDUWatchObserver
{
public:

	HandoverSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	~HandoverSS() {};

	void
	start(service::handoverStrategy::Interface::Stations,
		  HandoverCallBackInterface* callBackInterface);

	virtual bool
	doIsAccepting(const wns::ldk::CompoundPtr&) const
		{
			assure (0,
					"wimac::HandoverSS: doIsAcception is not in use! \n");
			return false;
		} // doIsAccepting

	virtual void
	doSendData(const wns::ldk::CompoundPtr&)
		{
			assure (0,
					"wimac::HandoverSS: doSendData is not in use! \n");
		} // doSendData

	virtual void
	doOnData(const wns::ldk::CompoundPtr& compound);

	virtual void
	doWakeup();

	virtual void
	onFUNCreated();

	/// Implementation of NewFrameObserver
	virtual void
	messageNewFrame();


	/// Implementation of PDUWatchObserver
	virtual void
	notifyPDUWatch( wns::ldk::CommandPool commandPool );

private:

	void doOnMOB_BSHO_RSP(const wns::ldk::CompoundPtr& compound);
	void result(service::handoverStrategy::Interface::Stations ackBaseStations);

	HandoverCommand::TransactionID activeTransactionID_;
	HandoverCommand::TransactionID highestTransactionID_;

	ConnectionIdentifier::Frames remainTimerWaitingForRSP_;
	std::list<wns::ldk::CommandPool> doMOB_HO_IND_;

   	std::list<wns::ldk::CompoundPtr> compoundQueue_;
	HandoverCallBackInterface* callBackInterface_;
	service::handoverStrategy::Interface::Stations ackBaseStations_;


	//Static values from PyConfig
	ConnectionIdentifier::Frames timerWaitingForRSP_;
	Bit mob_msho_reqPDUSize_;
	Bit mob_ho_indPDUSize_;

	struct{
		std::string connectionClassifierName;
		std::string newFrameProviderName;
		std::string connectionManagerName;
		std::string pduWatchProviderName;

		wns::ldk::FunctionalUnit* connectionClassifier;
		wns::ldk::fcf::NewFrameProvider* newFrameProvider;
		service::ConnectionManager* connectionManager;
		scheduler::PDUWatchProvider* pduWatchProvider;
	} friends_;
};

}} // controlplane // wimac

#endif // NOT defined WIMAC_HANDOVER_HPP


