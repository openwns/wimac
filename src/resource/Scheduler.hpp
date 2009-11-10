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

#ifndef WIMAC_RESOURCE_SCHEDULER_HPP
#define WIMAC_RESOURCE_SCHEDULER_HPP

#include <memory>

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/resource/Pool.hpp>
#include <WIMAC/scheduler/Interface.hpp>


namespace wimac {
namespace resource {


/**
 * @brief The Scheduler distributes jobs according to the scheduling
 * strategy over the available resource units.
 */
template
< typename JOBTYPE,
  typename STRATEGY,
  typename MAPPER>
class Scheduler :
        public virtual wimac::scheduler::Interface
{
public:
    typedef JOBTYPE JobType;
    typedef STRATEGY Strategy;
    typedef MAPPER Mapper;

    typedef typename MAPPER::Ranking Ranking;

    Scheduler(wns::ldk::fun::FUN*, const wns::pyconfig::View&);

    /**
     * @brief Schedule a single job.
     */
    void schedule(const wns::ldk::CompoundPtr&);

    /**
     * @brief Starts the scheduling process.
     *
     * @sa wimac::scheduler::Interface
     */
    void startScheduling()
    {
        throw wns::Exception("Scheduler::startScheduling() is not implemented");
    }

    /**
     * @brief Finish the schedule process.
     */
    void finishScheduling()
    {
        throw wns::Exception("Scheduler::finishScheduling() is not implemented");
    }

    /**
     * @brief Deliver the calculated schedule to the connector.
     */
    void deliverSchedule(wns::ldk::Connector*);

    /**
     * @brief Makes the scheduler familiar with its receptor.
     */
    void setReceptor(wns::ldk::Receptor* receptor)
    {
        receptor_ = receptor;
    }

    /**
     * @brief Set the duration for the next schedule round.
     */
    void setDuration(const wns::simulator::Time& duration)
    {
        duration_ = duration;
    }

    /**
     * @brief Get the duration of the next schedule.
     */
    wns::simulator::Time getDuration() const
    {
        return duration_;
    }

    /**
     * @brief Sets the functional unit network for this scheduler.
     */
    void setFUN(wns::ldk::fun::FUN*){}

private:
    std::auto_ptr<Mapper> mapper_;
    std::auto_ptr<Strategy> strategy_;

    wns::ldk::Receptor* receptor_;
    wns::simulator::Time duration_;

    bool doIsAccepting( const wns::ldk::CompoundPtr&) const
    {
        throw wns::Exception("Scheduler::doIsAccepting() is not implemented");
    }
};


template <typename JOBTYPE, typename STRATEGY, typename MAPPER>
Scheduler<JOBTYPE, STRATEGY, MAPPER>::Scheduler(wns::ldk::fun::FUN* fun,
                                                const wns::pyconfig::View& config)
{
    mapper_.reset(new Mapper(fun, config.getView("mapper")));
    strategy_.reset(new Strategy(config.getView("strategy")));
}

template <typename JOBTYPE, typename STRATEGY, typename MAPPER>
void
Scheduler<JOBTYPE, STRATEGY, MAPPER>::schedule(const wns::ldk::CompoundPtr& compound)
{
    JobType job(compound);
    Ranking ranking = mapper_->acquire(job.getID());
    strategy_->schedule(job, ranking);
}

template <typename JOBTYPE, typename STRATEGY, typename MAPPER>
void Scheduler<JOBTYPE, STRATEGY, MAPPER>::deliverSchedule(wns::ldk::Connector*)
{
    throw wns::Exception("void DefaultScheduler::deliverSchedule(wns::ldk::Connector*) unimplemented");
}

}
}

#endif
