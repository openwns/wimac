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
#ifndef WIMAC_RANG_HPP
#define WIMAC_RANG_HPP

#include <WNS/logger/Logger.hpp>

#include <WNS/service/dll/DataTransmission.hpp>
#include <WNS/service/dll/Handler.hpp>
#include <WNS/node/component/Component.hpp>
#include <WNS/container/Registry.hpp>

#include <WNS/osi/PDU.hpp>

namespace wimac {
    /**
     * @brief RANG Radio Access Network Gateway.
     *
     * This is the gateway to the core network for all APs.  The RANG is
     * connected to the Network Layer (IP) of the server. The APs don't
     * have an IP component, they terminate with the Layer2. So, the
     * communication between the server and an AP goes via the RANG. The
     * RANG has the funtion of a router, i.e. it forwards outgoing packets
     * to the correct AP's DLL. From the Viewpoint of the server's NL, it
     * behaves like a DLL itself
     */
    class RANG :
        public virtual wns::service::dll::UnicastDataTransmission,
        public virtual wns::service::dll::Notification,
        public virtual wns::service::dll::Handler,
        public wns::node::component::Component
    {
        typedef wns::container::Registry<wns::service::dll::UnicastAddress,
                                         wns::service::dll::UnicastDataTransmission*> AccessPointLookup;
    public:
        RANG(wns::node::Interface*, const wns::pyconfig::View&);


	//****
	/*
	void wns::service::dll::DataTransmission<Address>::sendData(
		const Address&,
		const wns::osi::PDUPtr&,
		wns::service::dll::protocolNumber,
		wns::service::dll::FlowID)
		
		[with Address = wns::service::dll::UnicastAddress]
	*/

        /** @name wns::service::dll::DataTransmission service */
        //@{
        virtual void
        sendData(
            const wns::service::dll::UnicastAddress& _peer,
            const wns::SmartPtr<wns::osi::PDU>& _data,
            wns::service::dll::protocolNumber protocol,
	    wns::service::dll::FlowID _dllFlowID = wns::service::dll::NoFlowID);//****
        //@}

        virtual wns::service::dll::UnicastAddress
        getMACAddress() const { return wns::service::dll::UnicastAddress();}

        /** @name wns::service::dll::Notification service */
        //@{
        virtual void
        registerHandler(wns::service::dll::protocolNumber protocol,
                        wns::service::dll::Handler* _dh);
        //@}

        /** @name wns::service::dll::Handler Interface */
        //@{
        /**
         * @brief standard onData method
         */
        virtual void onData(const wns::SmartPtr<wns::osi::PDU>& _data,
		                  wns::service::dll::FlowID _dllFlowID = wns::service::dll::NoFlowID);//****

        /**
         * @brief Modified Handler Interface for APs.
         *
         * Modified Handler Interface for APs. It gets two parameters in
         * addition to the PDUPtr to keep the RANG's routing table up to
         * date. */
        bool knowsAddress(wns::service::dll::UnicastAddress _sourceMACAddress);

        wns::service::dll::UnicastDataTransmission*
        getAccessPointFor(wns::service::dll::UnicastAddress _sourceMACAddress);

        void updateAPLookUp(wns::service::dll::UnicastAddress _sourceMACAddress,
                            wns::service::dll::UnicastDataTransmission* _ap);

        void removeAddress(wns::service::dll::UnicastAddress _sourceMACAddress,
                           wns::service::dll::UnicastDataTransmission* _ap);

        void onData(const wns::SmartPtr<wns::osi::PDU>& _data,
                    wns::service::dll::UnicastAddress _sourceMACAddress,
                    wns::service::dll::UnicastDataTransmission* _ap,
                    wns::service::dll::FlowID _dllFlowID = wns::service::dll::NoFlowID);//****
        //@}

        /** @name wns::node::component::Component Interface */
        //@{
        virtual void onNodeCreated();
        virtual void onWorldCreated();
        virtual void onShutdown();
        virtual void registerFlowHandler(wns::service::dll::FlowHandler*){};
        virtual void registerIRuleControl(wns::service::dll::IRuleControl*){};
	//@}

    protected:
        virtual void
        doStartup();

    private:
        wns::pyconfig::View config;
        AccessPointLookup accessPointLookup;

        /**
         * @brief Needed for demultiplexing of upper layer protocols.
         */
        typedef wns::container::Registry<wns::service::dll::protocolNumber, wns::service::dll::Handler*> DataHandlerRegistry;

        /**
         * @brief Registry for datahandlers. Each datahandler is select
         * by the protocol number.
         */
        DataHandlerRegistry dataHandlerRegistry;

        wns::logger::Logger logger;
    };
}

#endif


