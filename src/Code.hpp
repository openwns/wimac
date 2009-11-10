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
#ifndef WIMAC_CODE_HPP
#define WIMAC_CODE_HPP

#include <boost/rational.hpp>

namespace wimac {

class Code
{
public:
    Code(const wns::pyconfig::View& config):
        rate_(config.get<int>("numerator"), config.get<int>("denominator"))
    {
    }

    /**
     * @brief Returns the coding rate of the code.
     */
    boost::rational<int> rate() const
    {
        return rate_;
    }
private:
    friend class PhyMode;
    Code() : rate_(0){}
    boost::rational<int> rate_;
};
}

#endif
