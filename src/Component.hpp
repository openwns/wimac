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


#ifndef WIMAC_COMPONENT_HPP
#define WIMAC_COMPONENT_HPP
#include <WNS/node/component/Component.hpp>
#include <WNS/ldk/Layer.hpp>
#include <WNS/service/phy/ofdma/DataTransmission.hpp>
#include <WNS/probe/bus/ContextProvider.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/service/dll/DataTransmission.hpp>
#include <WNS/ldk/fun/Main.hpp>

#include <WIMAC/ConnectionIdentifier.hpp>


namespace wimac {

    namespace service {
        class ControlPlaneManagerInterface;
        class ConnectionManager;
    }

    class FunctionalUnit;
    class CommandProxy;

    class StationType
    {
    public:
        enum {
            AP,
            FRS,
            UT,
            RT
        };

        static std::string toString(int type)
        {
            switch (type)
            {
            case AP:
                return "AP";
            case FRS:
                return "FRS";
            case UT:
                return "UT";
            case RT:
                return "RT";
            default:
                wns::Exception e("Unknown station type with id: ");
                e << type;
                throw e;
            }
        }
    };

    /**
     * @brief Component represents the wimac component, virtually
     * being the jacket for all functional units.
     */
    class Component:
        public wns::node::component::Component,
        public wns::ldk::Layer,
        public wns::probe::bus::ContextProvider
    {
    public:
        typedef ConnectionIdentifier::StationID StationID;
        typedef ConnectionIdentifier::QoSCategory QoSCategory;

        Component(wns::node::Interface*, const wns::pyconfig::View&);

        std::string
        getName() const;

        /**
         * @brief Get the ID of the cell which is the station ID of
         * the base station.
         */
        unsigned int getCellID() const;

        /**
         * @brief Get the ID of this station.
         */
        unsigned int getID() const
        {
            return id_;
        }


        /**
         * @brief Distance measure from the associated BS to be used as probe instance id.
         *
         * For a AP, ring is always 1
         * For a FRS, ring is always the ring of the next station in uplink + 2
         * For a UT, ring is always the ring of the RAP + 1
         *
         * BS(1) <--> RS(3) <--> RS(5) <--> UT(6)
         *   ^          ^          ^
         *   |          |          |
         *   v          v          v
         *  UT(2)      UT(4)      UT(6)
         *
         */
        unsigned int getRing() const
        {
            return ring_;
        }

        /** @brief Access to the DLL Address */
        wns::service::dll::UnicastAddress getDLLAddress() const
        {
            return address_;
        }

        /**
         * @brief Returns the functional unit network of this component
         */
        wns::ldk::fun::Main*
        getFUN();

        /**
         * @brief Used by class PseudoBWreqGenerator for BWreq shortcut.
         *
         * @todo Replace getNumberOfQueuedPDUs with a better solution.
         */
        int getNumberOfQueuedPDUs(ConnectionIdentifiers cis);

        // ComponentInterface
        virtual void onNodeCreated();
        virtual void onWorldCreated();
        virtual void onShutdown();

        /**
         * @brief Indicate the station type, i.e. access point or user terminal.
         *
         * The station type IDs correspond to the enum
         * StationType. Here we do not use the IEEE 802.16
         * terminology. Instead we use the terms access point (AP) and
         * user terminal (UT).
         */
        int getStationType() const
        {
            return stationType_;
        }

        /**
         * @brief Implementation of the virtual method in ContextProvider.
         */
        void doVisit(wns::probe::bus::IContext&) const;


        /**
         *  @brief Returns the MAC address of this component.
         */
        wns::service::dll::UnicastAddress
        getMACAddress() const
        {
            return address_;
        }

    private:
        Component(const Component&); // disallow copy constructor
        Component& operator=(const Component&); // disallow assignment

        virtual void
        doStartup();


        wns::ldk::fun::Main* fun_;

        //Values form PyConfig
        int stationType_;
        unsigned int id_;
        wns::service::dll::UnicastAddress address_;
        unsigned int ring_;

        wns::probe::bus::ContextProviderCollection contextProviders_;
    };
}

#endif
