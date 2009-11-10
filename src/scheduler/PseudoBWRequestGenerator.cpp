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


#include <WIMAC/Logger.hpp>
#include <WIMAC/Component.hpp>

#include <WIMAC/scheduler/PseudoBWRequestGenerator.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/Classifier.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/StationManager.hpp>

#include <WNS/StaticFactory.hpp>

#include <WNS/scheduler/SchedulerTypes.hpp>
#include <WNS/ldk/helper/FakePDU.hpp>
#include <WNS/scheduler/queue/QueueInterface.hpp>

#include <map>

using namespace wns;
using namespace wns::ldk;
using namespace wimac;
using namespace wimac::scheduler;

PseudoBWRequestGenerator::PseudoBWRequestGenerator( const wns::pyconfig::View& config ) :
    Cloneable<PseudoBWRequestGenerator>(),
    component_(0),
    friends_(),
    packetSize(config.get<int>("packetSize"))
{
    //ip overhead of 20 byte
    packetSize += 8 * 20;
    //overhead due to MAC header (48 bits without CRC)
    packetSize += config.get<int>("pduOverhead");

    friends_.connectionManagerName = config.get<std::string>("connectionManager");
    friends_.classifierName = config.get<std::string>("classifier");

    friends_.connectionManager = NULL;
    friends_.ulScheduler = NULL;
    friends_.classifier = NULL;
}

void PseudoBWRequestGenerator::setFUN(wns::ldk::fun::FUN* fun)
{
    component_ = dynamic_cast<wimac::Component*>(fun->getLayer());
    assure(component_, "Could not get Layer2 from FUN");
}

void
PseudoBWRequestGenerator::setScheduler(wimac::scheduler::Interface* scheduler)
{
    friends_.connectionManager = component_
        ->getManagementService<service::ConnectionManager>
        (friends_.connectionManagerName);

    friends_.ulScheduler = dynamic_cast<Scheduler*>(scheduler);
    assureNotNull(friends_.ulScheduler);

    friends_.classifier = component_->getFUN()->findFriend<wimac::ConnectionClassifier*>
        (friends_.classifierName);
}

/** @brief Indicates upgoing connections.
 *
 * This functor returns true if the given connection is a uplink connection and
 * the subscriber is equal to myID.
 */
struct UpgoingConnection
    : public std::unary_function< wimac::ConnectionIdentifierPtr, bool>
{
    explicit UpgoingConnection(int myID): myID_( myID) {}
    bool operator()(const wimac::ConnectionIdentifierPtr& ptr)
    {
        return ptr->subscriberStation_ == myID_;
    }
private:
    int myID_;
};

/**
 * @brief On doWakeup(), a list of all CIDs of registered connections is retrieved
 * from the ConnectionManager. Then, one "Fake"-PDU is created for every
 * user and given to the lower FU which should be a generic scheduler operating
 * in UL-mode.
 */
void
PseudoBWRequestGenerator::wakeup() {
    // Delete all old fake packets from last generation, because fake pdus (aquivalent to peerQueues) is generated every frame.
    friends_.ulScheduler->resetAllQueues();

    ConnectionIdentifiers allBasicConnIDs =
        friends_.connectionManager->getAllBasicConnections ();

    allBasicConnIDs.remove_if( UpgoingConnection( component_->getID()) );

    std::map<wns::scheduler::UserID, ConnectionIdentifier::CID> userIDs;
    userIDs.clear();

    // FIXME BWrequest shortcut.
    std::map<wns::scheduler::UserID, int> peerQueuePDUSize;
    peerQueuePDUSize.clear();

    // first get a list of users having registered connections and one of the
    // respective CIDs.

    for (ConnectionIdentifier::List::const_iterator iter = allBasicConnIDs.begin();
         iter != allBasicConnIDs.end(); ++iter) {
        ConnectionIdentifier::Ptr cidPtr = *iter;

        // No bandwidth request for subscriber stations, which aren't listening
        if(cidPtr->ciNotListening_ > 0)
            continue;

        ConnectionIdentifier::StationID peerStationId = cidPtr->subscriberStation_;

        Component* peerComponent = dynamic_cast<wimac::Component*>(
            TheStationManager::getInstance()->getStationByID(peerStationId) );

        assure(peerComponent, "Invalid peer layer pointer");
        assure(peerComponent->getNode(), "No valid Node pointer in peer FUN");

        //FIXME BWrequest shortcut
        wimac::ConnectionIdentifiers cis =
            component_->getManagementService<service::ConnectionManager>
            ("connectionManager")->getIncomingConnections(peerStationId);
        int queueSize = peerComponent->getNumberOfQueuedPDUs(cis);
        if(queueSize == 0)
            continue;
        peerQueuePDUSize[(peerComponent->getNode())] = queueSize;

        userIDs[(peerComponent->getNode())] = cidPtr->getID();
    }

    // then create PDUs for every user and hand them down to the lower layer
    for (std::map<wns::scheduler::UserID, ConnectionIdentifier::CID>::const_iterator iter = userIDs.begin();
         iter != userIDs.end(); ++iter) {
        LOG_INFO(component_->getName(), " PseudoBWReqGenerator: is going to generate ",
                 peerQueuePDUSize[iter->first], " Fake PDUs for CID ", iter->second);
        for( int i = 1; i <= peerQueuePDUSize[iter->first]; ++i)
        {
            CompoundPtr pdu =
                CompoundPtr(new Compound(component_->getFUN()->getProxy()->createCommandPool(),
                                         wns::ldk::helper::FakePDUPtr(new helper::FakePDU(packetSize))));

            wns::ldk::ClassifierCommand* command =
                friends_.classifier->activateCommand(pdu->getCommandPool());
            command->peer.id = iter->second;

            if(friends_.ulScheduler->isAccepting(pdu)){
                friends_.ulScheduler->schedule(pdu);
                LOG_TRACE(component_->getFUN()->getName(), " PseudoBWRequestGenerator: generated a Fake PDU for CID ", iter->second);
            }
            else
                break;
        }
    }
}
