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


#ifndef WIMAC_HPP
#define WIMAC_HPP

#include <WNS/module/Module.hpp>
#include <WNS/ldk/ldk.hpp>

/**
 * @page IEEE 802.16 module (WiMAC)
 *
 * @section into_sec Introduction
 *
 * The WiMAC module implements the IEEE 802.16 protocol.
 *
 * @section main_part_sec Main part of the WiMAC module
 *
 * The main functionality of the WiMAC is implemented in the FUN of the WiMAC
 * Layer2. Within the FUN the IEEE 802.16 frame is controlled by the Frame
 * Configuration Framework. Additional functionality is implemented through
 * Functional Units and management classes.
 */

/**
 * @namespace wimac
 * The wimac namespace contains all WiMAC specific implementations.
 */

/**
 * @namespace wimac::frame
 * The wimac::frame namespace contains all implemetations that are specific for
 * the WiMAC frame.
 */

/**
 * @namespace wimac::scheduler
 * Scheduler related classes.
 */

namespace dll{
    class StationManager;
}
namespace wimac {

    /**
     * @brief Anchor of the library.
     *
     * Start up and shut down are coordinated from here. The
     * openWNS-core will use an instance of this class to start up and
     * shut down the library. Therefor it calls GetModuleInterface()
     * which returns a pointer to an instance of this class. The
     * method startUp() is called subsequently.
     */

    class WiMAC :
            public wns::module::Module<WiMAC>
    {

    public:
        WiMAC(const wns::pyconfig::View& _pyConfigView);

        virtual void configure();
        virtual void startUp();
        virtual void shutDown();
    };
}

#endif
