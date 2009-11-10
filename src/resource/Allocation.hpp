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

#ifndef WIMAC_RESOURCE_ALLOCATION_HPP
#define WIMAC_RESOURCE_ALLOCATION_HPP

#include <map>

#include <SmartPtr.hpp>

#include <WNS/resource/Job.hpp>

namespace wimac {
namespace resource {

/**
 * @brief The Allocation represents a resource allocation.
 *
 * The Allocation represents a resource allocation of a job on
 * resource units. For this, the allocation keeps a list of pairs of
 * UnitIDs and Bits.
 */
class Allocation
{
public:
    /**
     * @brief Create an allocation for job.
     */
    Allocation(const wns::SmartPtr<wimac::resource::Job> job);

    /**
     * @brief Allocate size bits of the resource unit with the job.
     */
    void allocate(int jobID, const Bit& size);

private:
    std::list<int> jobs_;
    Bit unallocated_;
};
}
}

#endif
