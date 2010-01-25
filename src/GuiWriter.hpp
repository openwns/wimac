/******************************************************************************
 * WiMac                                                                    *
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#ifndef WIMAC_GUIWRITER_HPP
#define WIMAC_GUIWRITER_HPP


#include <WNS/ldk/fu/Plain.hpp>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>

namespace wimac {
    class PhyUser;
    /**
     * @brief Writer to Gui log
     *
     * The GuiWriter will write the log file that can be read by Gui.
     * 
     */
    class GuiWriter
    {

    public:
        GuiWriter(wns::probe::bus::ContextCollectorPtr guiProbe, PhyUser* phyuser);
        virtual ~GuiWriter();

 //       void setManagerAndFun(wifimac::lowerMAC::Manager* manager, wns::ldk::fun::FUN* fun);
 //       void writeToProbe(int macaddr, int frametype, int aPhyMode, wns::simulator::Time frameTxDuration, int channel, int TxPower);
        void writeToProbe(const wns::ldk::CompoundPtr& compound, int macaddr);

        

    private:
        bool first_created;
        wns::probe::bus::ContextCollectorPtr guiProbe_;
 //       wifimac::lowerMAC::Manager* amanager;
        wns::ldk::fun::FUN* fun_;
        PhyUser* phyuser_;

        static int gui_station_counter_;

        int gui_station_id_;
    };

} // namespace wimac

#endif // NOT defined WIFIMAC_CONVERGENCE_GUIWRITER_HPP


