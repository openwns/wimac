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

#ifndef WIMAC_SETUPCONNECTION_HPP
#define WIMAC_SETUPCONNECTION_HPP

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>
#include <WNS/pyconfig/View.hpp>

#include <WIMAC/MACHeader.hpp>
#include <WIMAC/CIRMeasureInterface.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/Logger.hpp>

namespace wimac{
namespace controlplane{



/**
 *@todo (gra): It seems to be better, if only one management message Command exist
 *             for all ControlFUs. Members of the peer struct should be only the 
 *             ManagementMessageType and a Container with the specific message 
 *             informations.
 */
class SetupConnectionCommand :
	public wns::ldk::Command
{
public:
	typedef	uint16_t TransactionID;
	SetupConnectionCommand()
		{
			peer.managementMessageType = MACManagementMessage::UCD;
			// UCD is wrong, but 0

			peer.dsa_req.transactionID = 0;
			peer.dsa_req.ulQoSPriority = ConnectionIdentifier::BE;
			peer.dsa_req.dlQoSPriority = ConnectionIdentifier::BE;

			peer.dsa_rsp.transactionID = 0;
			peer.dsa_rsp.ulCID = 0;
			peer.dsa_rsp.ulQoSPriority = ConnectionIdentifier::BE;
			peer.dsa_rsp.dlCID = 0;
			peer.dsa_rsp.dlQoSPriority = ConnectionIdentifier::BE;

			peer.dsa_ack.transactionID = 0;

			magic.size = 0;


		};

	~SetupConnectionCommand()
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

		struct DSA_REQ{
			TransactionID transactionID; ///Unique identifier for this
			///transaction for one Station.
			ConnectionIdentifier::QoSCategory ulQoSPriority;
			ConnectionIdentifier::QoSCategory dlQoSPriority;
		} dsa_req;

		struct DSA_RSP{
			TransactionID transactionID;

			ConnectionIdentifier::CID ulCID;
			ConnectionIdentifier::QoSCategory ulQoSPriority;

			ConnectionIdentifier::CID dlCID;
			ConnectionIdentifier::QoSCategory dlQoSPriority;
		} dsa_rsp;

		struct DSA_ACK{
			TransactionID transactionID;
		} dsa_ack;

	} peer;

	struct {
		Bit size;
	} magic;

};



/********** SetupConnectionCallBackInterface ***********************************/
class SetupConnectionCallBackInterface
{
public:
	virtual ~SetupConnectionCallBackInterface()
		{
		}

	virtual void
	resultSetupConnection(bool result) = 0;

};



/********************* SetupConnectionBS ***********************************************/
    /**
	 * @brief SetupConnectionBS implementation for message exchange in the Base Station.
	 *
     * - SetupConnectionBS reacts on an DSA_REQ[transactionID, ulQoSPriority,
	 *   dlQoSPriority ) with an DSA_RSP(transactionID, ulCID, ulQoSPriority,
	 *   dlCID, dlQoSPriority)
     * - After receiving an DSA_ACK it creats new ConnectionIdentifiers for UL & DL
     *
	 */

class SetupConnectionBS:
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier< SetupConnectionCommand >,
	public wns::ldk::HasReceptor<>,
	public wns::ldk::HasConnector<>,
	public wns::ldk::HasDeliverer<>,
	public wns::Cloneable< SetupConnectionBS >,
	public wns::ldk::fcf::NewFrameObserver
{
	struct CIWaitingForSsDSA_ACK{

		// Constructor
		CIWaitingForSsDSA_ACK(
			ConnectionIdentifier::StationID subscriberStationi,
			SetupConnectionCommand::TransactionID transactionIDi,
			ConnectionIdentifier connectionIdentifieri,
			ConnectionIdentifier::Frames remainTimerWaitingForACKi) :

			subscriberStation ( subscriberStationi ),
			transactionID ( transactionIDi ),
			connectionIdentifier ( connectionIdentifieri ),
			remainTimerWaitingForACK ( remainTimerWaitingForACKi )
		{ }

		// Values
		ConnectionIdentifier::StationID subscriberStation;
		SetupConnectionCommand::TransactionID transactionID;
		ConnectionIdentifier connectionIdentifier;
		ConnectionIdentifier::Frames remainTimerWaitingForACK;
	};
	typedef std::list< CIWaitingForSsDSA_ACK > WaitingForSsDSA_ACK;

public:
	SetupConnectionBS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& );
	~SetupConnectionBS(){};

	virtual bool
	doIsAccepting(const wns::ldk::CompoundPtr&) const
		{
			assure (0,
					"wimac::SetupConnectionBS: doIsAcception is not in use! \n");
			return false;
		} // doIsAccepting

	virtual void
	doSendData(const wns::ldk::CompoundPtr&)
		{
			assure (0,
					"wimac::SetupConnectionBS: soSendData is not in use! \n");
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
	void
	doOnDSA_REQ(const wns::ldk::CompoundPtr& compound);

	void
	doOnDSA_ACK(const wns::ldk::CompoundPtr& compound);


	WaitingForSsDSA_ACK waitingForSsDSA_ACK_;

	std::list<wns::ldk::CompoundPtr> compoundQueue_;

	//Static values from PyConfig
	const ConnectionIdentifier::Frames timerWaitingForACK_;
	const Bit dsa_rspPDUSize_;

	struct{
		std::string connectionManagerName;
		std::string connectionClassifierName;
		std::string newFrameProviderName;

		service::ConnectionManager* connectionManager;
		wns::ldk::FunctionalUnit* connectionClassifier;
		wns::ldk::fcf::NewFrameProvider* newFrameProvider;
	} friends_;

};



/************** SetupConnectionSS *****************************************************/
    /**
	 * @brief SetupConnectionSS implementation for message exchange in the
	 *        Subscriber Station.
	 *
     * - Start SetupConnectionSS with function setupConnection()
     * - Sends an DSA_REQ[transactionID, ulQoSPriority, dlQoSPriority]
	 * - Receives an DSA_RSP[transactionID,ulCID, ulQoSPriority, dlCID, dlQoSPriority]
     * - React with an DSA_ACK[transactionID]
	 + - Create  new ConnectionIdentifiers for UL and DL
	 *
	 */


class SetupConnectionSS :
	virtual public wns::ldk::FunctionalUnit,
	public wns::ldk::CommandTypeSpecifier< SetupConnectionCommand >,
	public wns::ldk::HasReceptor<>,
	public wns::ldk::HasConnector<>,
	public wns::ldk::HasDeliverer<>,
	public wns::Cloneable< SetupConnectionSS >,
	public wns::ldk::fcf::NewFrameObserver
{
public:

	SetupConnectionSS(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
	~SetupConnectionSS() {};

	virtual bool
	doIsAccepting(const wns::ldk::CompoundPtr&) const
		{
			assure (0,
					"wimac::SetupConnectionSS: doIsAcception is not in use! \n");
			return false;
		} // doIsAccepting



	virtual void
	doSendData(const wns::ldk::CompoundPtr&)
		{
			assure (0,
					"wimac::SetupConnectionSS: doSendData is not in use! \n");
		} // doSendData

	virtual void
	doOnData(const wns::ldk::CompoundPtr& compound);

	virtual void
	doWakeup();

	virtual void
	onFUNCreated();

	virtual void
	messageNewFrame();

	void
	start(ConnectionIdentifier::QoSCategory qosPriority,
		  SetupConnectionCallBackInterface* callBackInterface);


private:
	void
	doOnDSA_RSP(const wns::ldk::CompoundPtr& compound);

	void
	result(bool result);

	ConnectionIdentifier::Frames remainTimerWaitingForRSP_;
	SetupConnectionCommand::TransactionID activeTransactionID_;
	SetupConnectionCommand::TransactionID highestTransactionID_;


	std::list<wns::ldk::CompoundPtr> compoundQueue_;
	SetupConnectionCallBackInterface* callBackInterface_;


	//Static values from PyConfig
	const ConnectionIdentifier::Frames timerWaitingForRSP_;
	const Bit dsa_reqPDUSize_;
	const Bit dsa_ackPDUSize_;

	struct{
		std::string connectionManagerName;
		std::string connectionClassifierName;
		std::string newFrameProviderName;

		service::ConnectionManager* connectionManager;
		wns::ldk::FunctionalUnit* connectionClassifier;
		wns::ldk::fcf::NewFrameProvider* newFrameProvider;
	} friends_;

};

}} // controlplane // wimac

#endif // NOT defined WIMAC_SETUPCONNECTION_HPP


