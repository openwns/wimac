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


#ifndef WIMAC_RESOURCE_STRATEGY_FCFS_HPP
#define WIMAC_RESOURCE_STRATEGY_FCFS_HPP

#include <WNS/pyconfig/View.hpp>
#include <WIMAC/resource/Mapper.hpp>
#include <WNS/Exception.hpp>

namespace wimac {
namespace resource {
namespace strategy {


template <typename JOBTYPE>
class FCFS
{
public:
    FCFS(const wns::pyconfig::View&){}

    void schedule(const JOBTYPE& job, const Mapper::Ranking& ranking)
    {
        throw wns::Exception("FCFS::schedule not implemented");
    }

    void start(){}

    void finish(){}

    template <typename T>
    void deliver(T*)
    {
        throw wns::Exception("FCFS::deliver not implemented");
    }
};

}
}
}
#endif
