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
#ifndef WIMAC_RESOURCE_UNIT_HPP
#define WIMAC_RESOURCE_UNIT_HPP

#include <WNS/simulator/Bit.hpp>
#include <WNS/SmartPtr.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

#include <WIMAC/resource/Job.hpp>
#include <WIMAC/PhyMode.hpp>

namespace wimac {
namespace resource {

class Mapper;

/**
 * @brief The ResourceUnit represents an atomic resource element.
 *
 * The scheduler schedules packets on the resource unit basis.
 */
class Unit
{
public:
    Unit(Mapper* mapper):
        mapper_(mapper),
        phyMode_(PhyMode::null())
    {
    }

    Unit(const Unit&);

    /**
     * @brief Returns the bit capacity of this resource unit for the
     * given job.
     *
     * The capacity is calculated by the maximum capacity of this unit
     * minus the amount of bits of the allocated jobs.
     */
    Bit capacity(int jobID) const;

    /**
     * @brief Each resource unit can be identified by the unique id.
     */
    int ID() const
    {
        return id_;
    }

    /**
     * @brief Returns a ranking for the priority of the job.
     *
     * Each resource unit can be allocated by a job. The rank gives a
     * priority for the allocation of the job on this resource unit.
     */
    int rank(int jobID) const;

    /**
     * @brief Mandatory comparison operator.
     */
    bool operator<(const Unit& rhs)
    {
        return id_ < rhs.id_;
    }

private:

    int id_;
    
    Mapper* mapper_;

    std::map<Job*, Bit> jobs_;

    wimac::PhyMode phyMode_;
    //wns::service::phymode::PhyModeInterface phyMode_;
};
}
}
#endif
