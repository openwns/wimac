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
#ifndef WIMAC_RESOURCE_MAPPER_HPP
#define WIMAC_RESOURCE_MAPPER_HPP

#include <vector>
#include <boost/noncopyable.hpp>

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/resource/Pool.hpp>
#include <WIMAC/resource/Job.hpp>


namespace wns {
namespace ldk {
namespace fun {
class FUN;
}
}
}

namespace wimac {
namespace resource {


/**
 * @brief The Mapper assigns a physical resource partition to
 * each resource unit.
 *
 * Usually a resource unit occupies a single subband and a single
 * timeslot. The Mapper keeps detailed information about the mapping
 * of resource units to physical resources.
 *
 *
 */
class Mapper :
        boost::noncopyable
{
public:

    typedef std::vector<Unit> Ranking;

    Mapper(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

    /**
     * @brief Return a pool of resource units ordered by decreasing
     * scheduling priority/rank for the given job.
     */
    Ranking
    acquire(int jobID);

    /**
     * @brief Get the resource unit with the given id.
     */
    Unit
    getUnit(int id);


    /**
     * @brief Get the bit capacity of the unit for the given job.
     *
     * Usually only the mapper has detailed information about the
     * current channel quality for the given job. As a result the
     * mapper is able to calculate the capacity of the given unit for
     * the job.
     */
    Bit
    capacity(const Unit&, const Job&);

    /**
     * @brief Retuns the number of subchannels the mapper is working
     * with.
     */
    int getSubchannels() const
    {
        return subChannels_;
    }

    /**
     * @brief Return the subchannel of the resource unit.
     */
    int
    getSubchannel(const Unit&) const;

    wns::ldk::fun::FUN*
    getFUN() const
    {
        return fun_;
    }

private:

    Mapper();

    std::vector<Unit> units_;

    int subChannels_;

    wns::ldk::fun::FUN* fun_;

    /**
     * @brief Defines the number or symbols pers resource unit.
     */
    int symPerUnit_;
};
}
}

#endif
