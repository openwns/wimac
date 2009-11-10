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
#include <WIMAC/resource/Mapper.hpp>

#include <algorithm>
#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/Layer.hpp>


#include <WIMAC/services/InterferenceCache.hpp>
#include <WIMAC/resource/Job.hpp>

using namespace wimac::resource;

/**
 * @brief A binary predecate that sorts for maximum capacity.
 */
struct MaxCapacity :
    public std::binary_function<Unit, Unit, bool>
{
    MaxCapacity(int id) :
        id_(id)
    {
    }

    bool operator()(const Unit& lhs, const Unit& rhs)
    {
        return lhs.capacity(id_) > rhs.capacity(id_);
    }

    int id_;

};

struct MaxRanking :
    public std::binary_function<Unit, Unit, bool>
{
    MaxRanking(int id) :
        id_(id)
    {
    }

    bool operator()(const Unit& lhs, const Unit& rhs)
    {
        return lhs.rank(id_) > rhs.rank(id_);
    }

    int id_;
};

Mapper::Mapper(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    fun_(fun),
    symPerUnit_(config.get<int>("symPerUnit"))
{
}

Mapper::Ranking
Mapper::acquire(int id)
{
    Ranking ranking(units_.size(), Unit(this));

    std::partial_sort_copy(units_.begin(), units_.end(),
                           ranking.begin(), ranking.end(), MaxCapacity(id));

    return ranking;
}

Unit
Mapper::getUnit(int id)
{
    if (id < 0 || id > int(units_.size()))
        throw wns::Exception("Unit ID out of range");
    return units_[id];
}

Bit
Mapper::capacity(const Unit& unit, const Job& job)
{
  /*
    wimac::service::InterferenceCache* ic =
        job.getDestination()
        ->getService<wimac::service::InterferenceCache*>("InterferenceCache");
    wns::Ratio expectSINR =
        ic->getAveragedCarrier(getFUN()->getLayer()->getNode(), getSubchannel(unit))/
        ic->getAveragedInterference(getFUN()->getLayer()->getNode(), getSubchannel(unit));
    
  */
    return 0;
}

int
Mapper::getSubchannel(const Unit& unit) const
{
    return unit.ID() % getSubchannels();
}
