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
#pragma once

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <WNS/scheduler/queue/QueueInterface.hpp>

namespace wns { namespace ldk {
    class HasReceptorInterface;
    }}


namespace wimac { namespace scheduler {

    class BypassQueue:
        public wns::scheduler::queue::QueueInterface,
        boost::noncopyable
    {
    public:
        BypassQueue(wns::ldk::HasReceptorInterface* parent, const wns::pyconfig::View& config);


        /**
         * @brief Returns true if the queue belonging to cid has
         * backlogged PDUs.
         */
        bool
        queueHasPDUs(wns::scheduler::ConnectionID);

        /**
         * @brief
         *
         * Return True if there is no compound in the queue
         */
        bool
        isEmpty() const;

        /**
         * @brief Returns the set of connections for which the queue
         * has backlogged PDUs.
         */
        wns::scheduler::ConnectionSet
        filterQueuedCids(wns::scheduler::ConnectionSet connections);


        /**
         * @brief Delivers the PDU that is the head-of-line element,
         * e.g. the oldest entry of the queue, in the queue specified
         * by cid.
         *
         * The exact sorting criterion for the queues (FIFO/LIFO etc.)
         * depends on the specific implementation of the queue.
         */
        wns::ldk::CompoundPtr
        getHeadOfLinePDU(wns::scheduler::ConnectionID cid);

        /**
         * @brief Delivers the size in bits of the PDU that will be
         * returned by the next call to getHeadOfLinePDU(ConnectionID
         * cid)
         */
        int
        getHeadOfLinePDUbits(wns::scheduler::ConnectionID cid);

        /**
         * @brief Retrurn True if a queue for this cid exists.
         */
        bool
        hasQueue(wns::scheduler::ConnectionID cid);

        /**
         * @brief Resets all internal queues and deletes all
         * backlogged PDUs.
         */
        ProbeOutput
        resetAllQueues();

        /**
         * @brief Resets all queues belonging to the given user and
         * deletes all backlogged PDUs in these queues.
         */
        ProbeOutput
        resetQueues(wns::scheduler::UserID);

        /**
         * @brief Resets only the queue belonging to the given
         * ConnectionID and all backlogged PDUs from this queue.
         */
        ProbeOutput
        resetQueue(wns::scheduler::ConnectionID);

        /**
         * @brief true if getHeadOfLinePDUSegment() is supported
         */
        bool
        supportsDynamicSegmentation() const;

        /**
         * @brief get compound out and do segmentation into #bits (gross)
         */
        wns::ldk::CompoundPtr
        getHeadOfLinePDUSegment(wns::scheduler::ConnectionID cid, int bits);

        /**
         * @brief Returns a UserSet of all users who have backlogged
         * PDUs in at least one of their queues.
         */
        wns::scheduler::UserSet
        getQueuedUsers() const;

        /**
         * @brief Returns a ConnectionSet of all connections who have
         * backlogged PDUs in his queue.
         */
        wns::scheduler::ConnectionSet
        getActiveConnections() const;

        /**
         * @brief Returns a ConnectionSet of all connections with a
         * certain priority who have backlogged PDUs in his queue.
         */
        wns::scheduler::ConnectionSet
        getActiveConnectionsForPriority(unsigned int priority) const;

        /**
         * @brief Returns the number of Compounds the Queue has stored
         * for a certain user.  Only used in
         * scheduler/strategy/ProportionalFairBase
         */
        uint32_t
        numCompoundsForUser(wns::scheduler::UserID user) const;

        /**
         * @brief Returns the number of Compounds the Queue has stored for a
         * certain user
         */
        uint32_t
        numCompoundsForCid(wns::scheduler::ConnectionID cid) const;

        /**
         * @brief Returns the number of Bits schedules for one user.
         * Only used in scheduler/strategy/ProportionalFairBase
         */
        uint32_t
        numBitsForUser(wns::scheduler::UserID user) const;

        /**
         * @brief Returns the number of Bits schedules for one user
         */
        uint32_t
        numBitsForCid(wns::scheduler::ConnectionID cid) const;

        /**
         * @brief Returns the a container of QueueStatus for each cid
         */
        wns::scheduler::QueueStatusContainer
        getQueueStatus() const;

        /**
         * @brief Has to be called before calling put(CompoundPtr) to
         * see whether the queue module can still accept the PDU.
         *
         * According to the specific implementation the acceptance may
         * be determined by the fill level of the correspondig queue.
         * called in doIsAccepting(..) of the ResourceScheduler-FU.
         */
        bool
        isAccepting(const wns::ldk::CompoundPtr&) const;

        /**
         * @brief Is used to store a PDU in the queue.
         *
         * Before calling put(compound), a call to
         * isAccepting(compound) has to be performed. The queue module
         * has to retrieve the corresponding UserID and ConnctionID on
         * its own.
         */
        void
        put(const wns::ldk::CompoundPtr& compound);

        /**
         * @brief Gives the queue module access to the RegistryProxy
         */
        void
        setColleagues(wns::scheduler::RegistryProxyInterface* registry);

        /**
         * @brief Gives the queue module access to the FUN
         */
        void
        setFUN(wns::ldk::fun::FUN* fun);

        /**
         * @brief print number of bits and pdus in each queue
         */
        std::string
        printAllQueues();

        wns::ldk::Receptor*
        getReceptor() const;


        


        struct IsAcceptingChecker
        {
            virtual bool operator()(const wns::ldk::CompoundPtr&) = 0;
        };

    private:

        wns::ldk::HasReceptorInterface* hasReceptor_;

        mutable IsAcceptingChecker* isAcceptingChecker_;

        wns::ldk::CompoundPtr current_;

        struct {
            wns::scheduler::RegistryProxyInterface* registry;
        } colleagues_;
    };

}
}
