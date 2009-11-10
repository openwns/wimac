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

#include <WIMAC/resource/Job.hpp>

#include <WNS/node/Interface.hpp>
#include <WNS/Exception.hpp>
#include <WNS/simulator/Bit.hpp>


using namespace wimac::resource;

JobID Job::idCounter_ = 0;

Job::Job(const wns::ldk::CompoundPtr& compound) :
    id_(++idCounter_),
    compound_(compound)
{
}

Bit
Job::length() const
{
    return 0;
}

wns::node::Interface*
Job::getDestination() const
{
    throw wns::Exception("Job::getDestination() unimplemented");
}

Bit
Job::unallocated() const
{
    Bit allocated = 0;
    for (UnitID2Bit::const_iterator it = units_.begin();
         it != units_.end();
         ++it)
    {
        allocated += it->second;
    }
    return length() - allocated;
}
