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

#ifndef WIMAC_RESOURCE_JOB_HPP
#define WIMAC_RESOURCE_JOB_HPP

#include <map>

#include <WNS/simulator/Bit.hpp>
#include <WNS/RefCountable.hpp>
#include <WNS/ldk/Compound.hpp>

namespace wns {
namespace node {
class Interface;
}
}

namespace wimac {
namespace resource {

typedef unsigned int JobID;

/**
 * @brief The Job represents a packet that the scheduler has to fill
 * into resource units.
 */
class Job :
    public wns::RefCountable
{
public:

    Job(const wns::ldk::CompoundPtr& compound);

    /**
     * @brief The length of the job in bits.
     */
    Bit length() const;

    /**
     * @brief The amount of unallocated bits.
     */
    Bit unallocated() const;

    /**
     * @brief Get the id of this job.
     */
    JobID getID() const
    {
        return id_;
    }

    /**
     * @brief Returns the node of the destination station.
     */
    wns::node::Interface*
    getDestination() const;

private:
    typedef std::map< int, Bit > UnitID2Bit;

    /**
     * @brief Count the amount of bits that are distributed over
     * resource units.
     */
    UnitID2Bit units_;

    JobID id_;

    wns::ldk::CompoundPtr compound_;

    static JobID idCounter_;
};
}
}

#endif
