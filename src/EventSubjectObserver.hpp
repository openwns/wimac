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

#ifndef WIMAC_EVENTSUBJECTOBSERVER_HPP
#define WIMAC_EVENTSUBJECTOBSERVER_HPP

#include <list>
#include <WNS/logger/Logger.hpp>


namespace wimac {

    class EventSubject;

    class EventObserver
    {
    public:
        EventObserver(std::string observerName);

        virtual
        ~EventObserver();

        void
        setEventSubject(EventSubject* eventSubject);

        virtual void
        eventSubjectDeleted();

        virtual void
        event(std::string event) = 0;

        std::string
        getObserverName() const {return observerName_;}

        const EventSubject*
        getEventSubject() const
        {
            return eventSubject_;
        }

    private:
        const std::string observerName_;

        EventSubject* eventSubject_;
    };



    class EventSubject
    {
    public:
        typedef std::list<EventObserver*> EventObservers;

        EventSubject(std::string stationName);

        ~EventSubject();

        void
        attachObserver(EventObserver* eventObserver);

        void
        detachObserver(EventObserver* eventObserver);

        void
        notifyEventObservers(std::string event);

        EventObservers
        testGetEventObservers_()
        {
            return eventObservers_;
        }


    private:

        void
        pl();

        EventObservers observersToNotify_;
        EventObservers eventObservers_;

        const std::string stationName_;
        wns::logger::Logger logger_;
    };
}

#endif
