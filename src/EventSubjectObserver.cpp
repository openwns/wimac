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


#include <WIMAC/EventSubjectObserver.hpp>

#include <WNS/Assure.hpp>

using namespace wimac;

EventSubject::EventSubject(std::string stationName) :
    observersToNotify_(),
    eventObservers_(),
    stationName_(stationName),
    logger_("WIMAC", "EventSubject")
{
}

EventSubject::~EventSubject()
{
    for(EventObservers::iterator iter = eventObservers_.begin();
        iter != eventObservers_.end(); ++iter)
    {
        (*iter)->setEventSubject(NULL);
    }
}

void
EventSubject::attachObserver(EventObserver* eventObserver)
{
    assure(std::find(eventObservers_.begin(),
                     eventObservers_.end(),
                     eventObserver)
           == eventObservers_.end(),
           "EventObserver is already added to EventSubject");

    MESSAGE_BEGIN(NORMAL, logger_, m, stationName_ );
    m << ": Attach observer! "
      << eventObserver->getObserverName();
    MESSAGE_END();

    eventObservers_.push_back(eventObserver);
    eventObserver->setEventSubject(this);
}

void
EventSubject::detachObserver(EventObserver* eventObserver)
{
    assure(std::find(eventObservers_.begin(),
                     eventObservers_.end(),
                     eventObserver)
           != eventObservers_.end(),
           "unknown EventObserver");

    MESSAGE_BEGIN(NORMAL, logger_, m, stationName_ );
    m << ": Detach observer! "
      << eventObserver->getObserverName();
    MESSAGE_END();

    eventObserver->setEventSubject(NULL);
    eventObserver->eventSubjectDeleted();
    eventObservers_.remove(eventObserver);

    // remove observer from list observersToNotify_, because it is detached
    if( std::find(observersToNotify_.begin(), observersToNotify_.end(),
                  eventObserver) != eventObservers_.end() )
        observersToNotify_.remove(eventObserver);

}

void
EventSubject::notifyEventObservers(std::string event)
{
    // Copy of list is necessary, because elements of List could deleted while
    // iterating throw the list
    observersToNotify_ = eventObservers_;

    // Send NewFrame message to observer
    MESSAGE_BEGIN(NORMAL, logger_, m, stationName_ );
    m << ": Notify all observers! eventObservers_.size():" << observersToNotify_.size();
    MESSAGE_END();

    while(!observersToNotify_.empty())
    {
        EventObserver* observer = observersToNotify_.front();
        observersToNotify_.pop_front();
        observer->event(event);
    }
}

EventObserver::EventObserver(std::string observerName):
    observerName_(observerName),
    eventSubject_(NULL)
{}

EventObserver::~EventObserver()
{
    if (eventSubject_)
    {
        eventSubject_->detachObserver(this);
        eventSubject_ = NULL;
    }

}

void
EventObserver::setEventSubject(EventSubject* eventSubject)
{
    eventSubject_ = eventSubject;
}

void
EventObserver::eventSubjectDeleted()
{
}
