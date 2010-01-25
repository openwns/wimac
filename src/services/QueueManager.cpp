/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
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

#include <WIMAC/services/QueueManager.hpp>
#include <WIMAC/scheduler/Interface.hpp>


using namespace wimac::service;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    wimac::service::QueueManager,
    wns::ldk::ManagementServiceInterface,
    "wimac.services.QueueManager",
    wns::ldk::MSRConfigCreator);
            
QueueManager::QueueManager(wns::ldk::ManagementServiceRegistry* msr, 
    const wns::pyconfig::View& config) :
    wns::scheduler::queue::IQueueManager(msr, config),
    connectionManagerServiceName_(config.get<std::string>("connectionManagerServiceName")),
    connectionManager_(NULL),
    logger_(config.get("logger"))
{
    MESSAGE_BEGIN(NORMAL, logger_, m, "QueueManager");
    m << " Created QueueManager Service using ConnectionManagerService ";
    m << connectionManagerServiceName_;
    MESSAGE_END();
}

QueueManager::~QueueManager() 
{
}

void
QueueManager::onMSRCreated()
{
    connectionManager_ =  
        getMSR()->getLayer()->getManagementService<service::ConnectionManager>(
            connectionManagerServiceName_);
    assure(connectionManager_ != NULL, "QueueManager needs a ConnectionManager");

    MESSAGE_BEGIN(NORMAL, logger_, m, "QueueManager");
    m << " Found valid ConnectionManagerService ";
    m << connectionManagerServiceName_;
    MESSAGE_END();

}

wns::scheduler::queue::QueueContainer
QueueManager::getAllQueues()
{
    wns::scheduler::queue::QueueContainer queues;

    ConnectionIdentifiers ci;
    ci = connectionManager_->getAllDataConnections(ConnectionIdentifier::Uplink);
    if(ci.empty())
        return queues;

    ConnectionIdentifiers::iterator it;

    for(it = ci.begin(); it != ci.end(); it++)
    {
        wns::scheduler::queue::QueueInterface* queue;

        if(cache_.find((*it)->cid_) != cache_.end())
        {
            queue = cache_[(*it)->cid_];
        }
        else
        {
           queue = getQueue((*it)->subscriberStation_, (*it)->cid_);

            if(queue != NULL)
            {            
                MESSAGE_BEGIN(NORMAL, logger_, m, "QueueManager");
                m << " Storing Queue pointer for CID ";
                m << (*it)->cid_ << " in cache";
                MESSAGE_END();
                cache_[(*it)->cid_] = queue;
            }
        }
        if(queue != NULL)
            queues[(*it)->cid_] = queue;                
    }        
    return queues;
}

wns::scheduler::queue::QueueInterface*
QueueManager::getQueue(wns::scheduler::ConnectionID cid)
{
    wns::scheduler::queue::QueueInterface* queue;
    if(cache_.find(cid) != cache_.end())
    {
        queue = cache_[cid];
    }
    else
    {
        queue = getQueue(getStationID(cid), cid);
                 
        if(queue != NULL)
        {
            MESSAGE_BEGIN(NORMAL, logger_, m, "QueueManager");
            m << " Storing Queue pointer for CID ";
            m << cid << " in cache";
            MESSAGE_END();
            cache_[cid] = queue;
        }
    }
    return queue;
}

void
QueueManager::startCollection(wns::scheduler::ConnectionID cid)
{
    wimac::frame::DataCollector* dc;

    if(dcCache_.find(cid) != dcCache_.end())
        dc = dcCache_[cid];
    else
    {
        Component* peerComponent = dynamic_cast<wimac::Component*>(
            TheStationManager::getInstance()->getStationByID(getStationID(cid)) );
    
        wns::ldk::fun::Main* fun = peerComponent->getFUN();
        dc = fun->findFriend<wimac::frame::DataCollector*>("ulscheduler");
        dcCache_[cid] = dc;
    }

    MESSAGE_BEGIN(NORMAL, logger_, m, "QueueManager");
    m << " Starting data collection for CID ";
    m << cid;
    MESSAGE_END();

    assure(dc, "Cannot find DataCollector in FUN");
    dc->getTxScheduler()->startScheduling();
}

wns::scheduler::queue::QueueInterface*
QueueManager::getQueue(ConnectionIdentifier::StationID peerStationId, wns::scheduler::ConnectionID cid)
{
    if(peerStationId == -1)
        return NULL;

    wimac::frame::DataCollector* dc;

    if(dcCache_.find(cid) != dcCache_.end())
        dc = dcCache_[cid];
    else
    {
        Component* peerComponent = dynamic_cast<wimac::Component*>(
            TheStationManager::getInstance()->getStationByID(peerStationId) );
    
        wns::ldk::fun::Main* fun = peerComponent->getFUN();
        dc = fun->findFriend<wimac::frame::DataCollector*>("ulscheduler");
        dcCache_[cid] = dc;
    }

    assure(dc, "Cannot find DataCollector in FUN");
    return dc->getTxScheduler()->getQueue();
}

wimac::ConnectionIdentifier::StationID
QueueManager::getStationID(wns::scheduler::ConnectionID cid)
{
    ConnectionIdentifiers ci;
    ci = connectionManager_->getAllDataConnections(ConnectionIdentifier::Uplink);
    assure(!ci.empty(), "Tried to get StationID from CID but no CIDs are known yet");

    ConnectionIdentifiers::iterator it;

    for(it = ci.begin(); it != ci.end(); it++)
    {
        if((*it)->cid_ == cid)
            return (*it)->subscriberStation_;                
    }
    return -1;
    //assure(false, "Cannot find StationID for CID");
}

