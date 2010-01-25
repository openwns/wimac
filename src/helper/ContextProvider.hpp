/******************************************************************************
 * WiFiMAC (IEEE 802.11)                                                      *
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

#ifndef WIMAC_HELPER_CONTEXTPROVIDER_HPP
#define WIMAC_HELPER_CONTEXTPROVIDER_HPP

#include <WNS/probe/bus/CommandContextProvider.hpp>
#include <WIMAC/UpperConvergence.hpp>

namespace wimac { namespace helper { namespace contextprovider {

    /**
    * @brief Context provider for a given compound: Filters by the source
    *   address given in the upperConvergenceComand
    */
    class SourceAddress:
        virtual public wns::probe::bus::CommandContextProvider<wimac::UpperCommand>
    {
    public:
        SourceAddress(wns::ldk::fun::FUN* fun, std::string ucCommandName);

        virtual
        ~SourceAddress();

    private:
        virtual void
        doVisitCommand(wns::probe::bus::IContext& c, const wimac::UpperCommand* command) const;
    };

    /**
    * @brief Context provider for a given compound: Filters by the target
    *   address given in the upperConvergenceComand
    */
    class TargetAddress:
        virtual public wns::probe::bus::CommandContextProvider<wimac::UpperCommand>
    {
    public:
        TargetAddress(wns::ldk::fun::FUN* fun, std::string ucCommandName);

        virtual
        ~TargetAddress();

    private:
        virtual void
        doVisitCommand(wns::probe::bus::IContext& c, const wimac::UpperCommand* command) const;
    };
}}}

#endif // WIMAC_HELPER_CONTEXTPROVIDER_HPP
