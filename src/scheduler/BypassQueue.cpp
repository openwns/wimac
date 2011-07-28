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

#include <WIMAC/scheduler/BypassQueue.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/simulator/Bit.hpp>
#include <WIMAC/Logger.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(wimac::scheduler::BypassQueue,
                                     wns::scheduler::queue::QueueInterface,
                                     "wimac.BypassQueue",
                                     wns::HasReceptorConfigCreator);


class QueueHasPDUs:
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    QueueHasPDUs(wns::scheduler::RegistryProxyInterface* reg,
                 wns::scheduler::ConnectionID cid):
        reg_(reg),
        cid_(cid),
        result_(false)
    {}

    bool operator()(const wns::ldk::CompoundPtr& compound)
    {
        if (reg_->getCIDforPDU(compound) == cid_)
            result_ = true;
        return false;
    }

    bool result() const {return result_;}

private:
    wns::scheduler::RegistryProxyInterface* reg_;
    wns::scheduler::ConnectionID cid_;
    bool result_;
};

class FilterQueued :
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    FilterQueued(wns::scheduler::RegistryProxyInterface* reg,
                 wns::scheduler::ConnectionSet candidates) :
        reg_(reg),
        candidates_(candidates)
    {
    }

    bool
    operator()(const wns::ldk::CompoundPtr& compound)
    {
        wns::scheduler::ConnectionID id = reg_->getCIDforPDU(compound);
        if (candidates_.find(id) != candidates_.end())
            result_.insert(id);
    }

    wns::scheduler::ConnectionSet
    result() const
    {
        return result_;
    }

private:
    wns::scheduler::RegistryProxyInterface* reg_;
    wns::scheduler::ConnectionSet candidates_;
    wns::scheduler::ConnectionSet result_;
};

class AcceptCID :
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    AcceptCID(wns::scheduler::RegistryProxyInterface* reg,
              wns::ldk::CompoundPtr* current,
              wns::scheduler::ConnectionID cid):
        reg_(reg),
        current_(current),
        cid_(cid)
    {
    }

    bool
    operator()(const wns::ldk::CompoundPtr& compound)
    {
        using namespace wimac;
        return (*current_ == wns::ldk::CompoundPtr())
            && (reg_->getCIDforPDU(compound) == cid_);
    }

private:
    wns::scheduler::RegistryProxyInterface* reg_;
    wns::ldk::CompoundPtr* current_;
    wns::scheduler::ConnectionID cid_;
    bool alreadyAccepted_;
};


class HeadOfLinePDUBits :
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    HeadOfLinePDUBits(wns::scheduler::RegistryProxyInterface* reg,
                      wns::scheduler::ConnectionID cid) :
        reg_(reg),
        cid_(cid),
        result_(0)
    {}

    bool
    operator()(const wns::ldk::CompoundPtr& compound)
    {
        if (reg_->getCIDforPDU(compound) == cid_)
            result_ = compound->getLengthInBits();
        return false;
    }

    Bit
    result() const
    {
        return result_;
    }
private:
    wns::scheduler::RegistryProxyInterface* reg_;
    wns::scheduler::ConnectionID cid_;
    Bit result_;
};

class ListUsers :
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    ListUsers(wns::scheduler::RegistryProxyInterface* reg) :
        reg_(reg),
        result_()
    {}

    bool
    operator()(const wns::ldk::CompoundPtr& compound)
    {
        using namespace wimac;
        result_.insert(reg_->getUserForCID( reg_->getCIDforPDU(compound)));
        return false;
    }

    wns::scheduler::UserSet
    result()
    {
        return result_;
    }

private:
    wns::scheduler::RegistryProxyInterface* reg_;
    wns::scheduler::UserSet result_;
};

class ListConnections :
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    ListConnections(wns::scheduler::RegistryProxyInterface* reg) :
        reg_(reg),
        result_()
    {}

    bool
    operator()(const wns::ldk::CompoundPtr& compound)
    {
        result_.insert(reg_->getCIDforPDU(compound));
        return false;
    }

    wns::scheduler::ConnectionSet
    result()
    {
        return result_;
    }

private:
    wns::scheduler::RegistryProxyInterface* reg_;
    wns::scheduler::ConnectionSet result_;
};

class ListPriorityConns :
    public wimac::scheduler::BypassQueue::IsAcceptingChecker
{
public:
    ListPriorityConns(wns::scheduler::RegistryProxyInterface* reg, int priority) :
        reg_(reg),
        priority_(priority),
        result_()
    {}

    bool
    operator()(const wns::ldk::CompoundPtr& compound)
    {
        if (reg_->getPriorityForConnection(reg_->getCIDforPDU(compound)) == priority_)
            result_.insert(reg_->getCIDforPDU(compound));
        return false;
    }

    wns::scheduler::ConnectionSet
    result()
    {
        return result_;
    }

private:
    wns::scheduler::RegistryProxyInterface* reg_;
    int priority_;
    wns::scheduler::ConnectionSet result_;
};

using namespace wimac::scheduler;


BypassQueue::BypassQueue(wns::ldk::HasReceptorInterface* parent, const wns::pyconfig::View&):
    hasReceptor_(parent),
    isAcceptingChecker_(0),
    current_(wns::ldk::CompoundPtr())
{
}

bool
BypassQueue::isAccepting(const wns::ldk::CompoundPtr& compound) const
{
    if (isAcceptingChecker_)
        return (*isAcceptingChecker_)(compound);
    return false;
}

bool
BypassQueue::queueHasPDUs(wns::scheduler::ConnectionID cid) const
{
    std::auto_ptr<QueueHasPDUs> queueHasPDUs( new QueueHasPDUs(colleagues_.registry, cid));
    isAcceptingChecker_ = queueHasPDUs.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    return queueHasPDUs->result();
}

bool
BypassQueue::isEmpty() const
{
    // Example from libwns:
    //    for (QueueContainer::const_iterator iter = queues.begin(); iter != queues.end(); ++iter)
    //    {
    //        if ((*iter).second.pduQueue.size() != 0)
    //            return false;
    //    }
    //    return true;
    throw wns::Exception("not implemented");
}


wns::scheduler::ConnectionSet
BypassQueue::filterQueuedCids(wns::scheduler::ConnectionSet connections)
{
    std::auto_ptr<FilterQueued> filtered( new FilterQueued(colleagues_.registry, connections));
    isAcceptingChecker_ = filtered.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    return filtered->result();
}

wns::ldk::CompoundPtr
BypassQueue::getHeadOfLinePDU(wns::scheduler::ConnectionID cid)
{
    std::auto_ptr<AcceptCID> acceptor(new AcceptCID(colleagues_.registry, &current_, cid));
    isAcceptingChecker_ = acceptor.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    wns::ldk::CompoundPtr result = current_;
    assure(result != wns::ldk::CompoundPtr(), "wimac::BypassQueue: about to return null compound");
    current_ = wns::ldk::CompoundPtr();
    return result;
}

int
BypassQueue::getHeadOfLinePDUbits(wns::scheduler::ConnectionID cid)
{
    std::auto_ptr<HeadOfLinePDUBits> counter(new HeadOfLinePDUBits(colleagues_.registry, cid));
    isAcceptingChecker_ = counter.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    return counter->result();
}

bool
BypassQueue::hasQueue(wns::scheduler::ConnectionID)
{
    return false;
}

void
BypassQueue::put(const wns::ldk::CompoundPtr& compound)
{
    assure((*isAcceptingChecker_)(compound), "BypassQueue is unable to accept a compound");
    assure(current_ == wns::ldk::CompoundPtr(), "Already accepted a compound");
    current_ = compound;
}

wns::scheduler::queue::QueueInterface::ProbeOutput
BypassQueue::resetAllQueues()
{
    return wns::scheduler::queue::QueueInterface::ProbeOutput();
}

wns::scheduler::queue::QueueInterface::ProbeOutput
BypassQueue::resetQueues(wns::scheduler::UserID)
{
    return wns::scheduler::queue::QueueInterface::ProbeOutput();
}

void
BypassQueue::frameStarts()
{
}

wns::scheduler::queue::QueueInterface::ProbeOutput
BypassQueue::resetQueue(wns::scheduler::ConnectionID)
{
    return wns::scheduler::queue::QueueInterface::ProbeOutput();
}

bool
BypassQueue::supportsDynamicSegmentation() const
{
    return false;
}

wns::ldk::CompoundPtr
BypassQueue::getHeadOfLinePDUSegment(wns::scheduler::ConnectionID cid, int bits)
{
    throw wns::Exception("Segmentation in bypass queue is not supported");
}


wns::scheduler::UserSet
BypassQueue::getQueuedUsers() const
{
    std::auto_ptr<ListUsers> listUsers(new ListUsers(colleagues_.registry));
    isAcceptingChecker_ = listUsers.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    return listUsers->result();
}

wns::scheduler::ConnectionSet
BypassQueue::getActiveConnections() const
{
    std::auto_ptr<ListConnections> listConnections(new ListConnections(colleagues_.registry));
    isAcceptingChecker_ = listConnections.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    return listConnections->result();
}

wns::scheduler::ConnectionSet
BypassQueue::getActiveConnectionsForPriority(unsigned int priority) const
{
    std::auto_ptr<ListPriorityConns> listConnections(new ListPriorityConns(colleagues_.registry, priority));
    isAcceptingChecker_ = listConnections.get();
    getReceptor()->wakeup();
    isAcceptingChecker_ = 0;
    return listConnections->result();
}

/* obsolete
unsigned long int
BypassQueue::numCompoundsForUser(wns::scheduler::UserID user) const
{
    throw wns::Exception("not implemented");
}
*/
unsigned long int
BypassQueue::numCompoundsForCid(wns::scheduler::ConnectionID cid) const
{
    throw wns::Exception("not implemented");
}
/* obsolete
unsigned long int
BypassQueue::numBitsForUser(wns::scheduler::UserID user) const
{
    throw wns::Exception("not implemented");
}
*/
unsigned long int
BypassQueue::numBitsForCid(wns::scheduler::ConnectionID cid) const
{
    throw wns::Exception("not implemented");
}

wns::scheduler::QueueStatusContainer
BypassQueue::getQueueStatus(bool forFuture) const
{
    throw wns::Exception("not implemented");
}

void
BypassQueue::setColleagues(wns::scheduler::RegistryProxyInterface* registry)
{
    colleagues_.registry = registry;
}

void
BypassQueue::setFUN(wns::ldk::fun::FUN*)
{
}

std::string
BypassQueue::printAllQueues()
{
    throw wns::Exception("not implemented");
}

wns::ldk::Receptor*
BypassQueue::getReceptor() const
{
    return hasReceptor_->getReceptor();
}

std::queue<wns::ldk::CompoundPtr> 
BypassQueue::getQueueCopy(wns::scheduler::ConnectionID cid)
{ 
    wns::Exception("You should not call getQueueCopy of the BypassQueue."); 
}
