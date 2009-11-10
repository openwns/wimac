/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
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
#ifndef WIMAC_PHYMODE_HPP
#define WIMAC_PHYMODE_HPP

#include <WNS/pyconfig/View.hpp>
#include <WNS/simulator/Bit.hpp>

#include <WIMAC/Code.hpp>
#include <WIMAC/Modulation.hpp>

namespace wimac {

class PhyMode
{
public:
    PhyMode(const wns::pyconfig::View& config):
        modulation_(config.getView("modulation")),
        code_(config.getView("code"))
    {
    }

    Bit symbolCapacity() const;

    static const PhyMode& null()
    {
        static const PhyMode null = PhyMode();
        return null;
    }

private:
    PhyMode();
    Modulation modulation_;
    Code code_;
};

}
#endif
