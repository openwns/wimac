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

#ifndef WIMAC_SCHEDULER_REGISTRYPROXYWIMAC_HPP
#define WIMAC_SCHEDULER_REGISTRYPROXYWIMAC_HPP

#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/Logger.hpp>
#include <WNS/scheduler/RegistryProxyInterface.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/service/phy/phymode/PhyModeMapperInterface.hpp>
#include <WNS/StaticFactory.hpp>


namespace wimac {

    namespace frame {
        class MapHandlerInterface;
    }
    namespace scheduler {

	class Scheduler;
	/// The registry proxy for the WiMAC.
	class RegistryProxyWiMAC
		: public wns::scheduler::RegistryProxyInterface
	{
	public:
		RegistryProxyWiMAC(wns::ldk::fun::FUN*, const wns::pyconfig::View&);
		~RegistryProxyWiMAC() {}


		wns::scheduler::UserID getUserForCID(wns::scheduler::ConnectionID cid);
		virtual wns::service::dll::UnicastAddress getPeerAddressForCID(wns::scheduler::ConnectionID cid);
		wns::scheduler::ConnectionVector getConnectionsForUser(const wns::scheduler::UserID user);
		float getMinTPForCID(wns::scheduler::ConnectionID cid);
		float getMaxDelayForCID(wns::scheduler::ConnectionID cid);
		wns::Ratio getMinSIRForCID(wns::scheduler::ConnectionID cid);
		wns::scheduler::ConnectionID getCIDforPDU(const wns::ldk::CompoundPtr& compound);
		void setFriends( const wns::ldk::CommandTypeSpecifierInterface* _classifier );
		void setFUN(const wns::ldk::fun::FUN* _fun);
		std::string getNameForUser(const wns::scheduler::UserID user);
		wns::service::phy::phymode::PhyModeMapperInterface* getPhyModeMapper() const;
		wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> getBestPhyMode(const wns::Ratio&);
		wns::scheduler::UserID getMyUserID();
		simTimeType getOverhead(int numBursts);
		wns::scheduler::ChannelQualityOnOneSubChannel estimateTxSINRAt(
            const wns::scheduler::UserID user, int slot);
		wns::scheduler::ChannelQualityOnOneSubChannel estimateRxSINROf(
            const wns::scheduler::UserID user, int slot);
		wns::Power estimateInterferenceStdDeviation(const wns::scheduler::UserID user);
		wns::scheduler::Bits getQueueSizeLimitPerConnection();
		int getStationType(const wns::scheduler::UserID user);
        virtual wns::scheduler::UserSet filterReachable( wns::scheduler::UserSet users ); // soon obsolete
		virtual wns::scheduler::UserSet filterReachable( wns::scheduler::UserSet users, const int frameNr );
		virtual wns::scheduler::ConnectionSet filterReachable(wns::scheduler::ConnectionSet connections, const int frameNr, bool useHARQ );
		virtual wns::scheduler::PowerMap calcULResources(const wns::scheduler::UserSet&, unsigned long int) const;
		virtual wns::scheduler::UserSet getActiveULUsers() const;
		/**@brief returns one for UTs, and #connected UTs in case of RNs */
		virtual int getTotalNumberOfUsers(const wns::scheduler::UserID user);
		void switchFilterTo(int qos);

		/** @brief get the ChannelsQualities (CQI) on all the subbands of the user.
		    Eventually for a future frameNr (prediction). */
		virtual wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
		getChannelQualities4UserOnUplink(wns::scheduler::UserID user, int frameNr);
		virtual wns::scheduler::ChannelQualitiesOnAllSubBandsPtr
		getChannelQualities4UserOnDownlink(wns::scheduler::UserID user, int frameNr);

	  /** @brief registerCID */
	  virtual void
	  registerCID(wns::scheduler::ConnectionID cid, wns::scheduler::UserID userID/*nextHop!*/) {};

	  /** @brief deregisterCID (important e.g. for Handover)*/
	  virtual void
	  deregisterCID(wns::scheduler::ConnectionID cid, const wns::scheduler::UserID userID) {};

	  /** @brief deregisterUser (important e.g. for Handover)*/
	  virtual void
	  deregisterUser(const wns::scheduler::UserID userID) {};

		virtual wns::scheduler::PowerCapabilities
		getPowerCapabilities(const wns::scheduler::UserID user) const;

		virtual wns::scheduler::PowerCapabilities
		getPowerCapabilities() const;

		/** @brief gets the number of QoS classes (for QoS Scheduling) **/
		int
		getNumberOfQoSClasses();
		/** @brief gets the number of priorities (for QoS Scheduling) **/
		virtual int
		getNumberOfPriorities();

		/** @brief get all CIds for the Priority Class (for QoS Scheduling) **/
		virtual wns::scheduler::ConnectionList&
		getCIDListForPriority(int priority);

		virtual wns::scheduler::ConnectionSet
		getConnectionsForPriority(int priority);

	  // added to RegistryProxyInterface
	  //const wns::service::phy::phymode::PhyModeInterfacePtr
	  //getPhyMode(wns::scheduler::ConnectionID /*cid*/);

	  int
	  getPriorityForConnection(wns::scheduler::ConnectionID /*cid*/);

	  //std::string
	  //compoundInfo(const wns::ldk::CompoundPtr& /*compound*/);

	  bool
	  getDL() const;

	  virtual bool
	  getCQIAvailable() const;

	protected:
		wns::ldk::fun::FUN* fun;
		wimac::Component* layer2;
		service::ConnectionManager* connManager;

		///\todo remove this when node->getComponent can deliver Layer2
		std::map<wns::scheduler::UserID, ConnectionIdentifier::StationID> userId2StationId;

		struct {
			wns::ldk::CommandTypeSpecifierInterface* classifier;
		} friends;

		//std::auto_ptr<wns::service::phy::phymode::PhyModeMapperInterface> phyModeMapper;
		wns::service::phy::phymode::PhyModeMapperInterface* phyModeMapper;

		const int queueSize;

		int currentQoSFilter;

	private:
		wns::scheduler::UserSet filterListening( wns::scheduler::UserSet users );
		wns::scheduler::UserSet filterQoSbased( wns::scheduler::UserSet users );

		wns::scheduler::PowerCapabilities powerUT;
		wns::scheduler::PowerCapabilities powerAP;
		wns::scheduler::PowerCapabilities powerFRS;

        int numberOfPriorities;
        wns::scheduler::ConnectionList cidList;
        bool isDL_;
        wimac::frame::MapHandlerInterface* mapHandler;
	};

}} // namespace wimac::scheduler
#endif // WIMAC_SCHEDULER_REGISTRYPROXYWIMAC_HPP


