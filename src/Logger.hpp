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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <WNS/ldk/ldk.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/module/Base.hpp>
#include <WNS/logger/Master.hpp>
#include <WNS/Singleton.hpp>

namespace wimac {

    const std::string demangledTypename( const std::string& nativeName);

    /**
     * @brief The WiMAC Logger.
     *
     * The WiMAC logger provides special functions to log events. It
     * is not necessary to create a logger instance in each
     * class. Instead, the WiMAC logger is created on demand. Inside
     * a method of a WiMAC class the logger can be called with three
     * different logging priorities.
     *
     * \li \c TRACE indicates lowest priority level logging messages.
     * \li \c INFO indicates medium priority level logging messages.
     * \li \c WARNING indicates high level priority logging messages.
     */
    class Logger :
            public wns::logger::Logger
    {
    public:
        explicit
        Logger() :
            wns::logger::Logger(),
            priority_(0)
        {}

        Logger(const std::string& loggerName):
            wns::logger::Logger("WiMAC",
                                loggerName,
                                wns::simulator::getMasterLogger()),
            priority_(0)
        {}

        Logger( int priority, const std::type_info& type ):
            wns::logger::Logger( "WiMAC", demangledTypename(type.name()),
                                 wns::simulator::getMasterLogger()),
            priority_(priority)
        {}


        Logger* prepareForMessage(int priority, const std::type_info& type)
        {
            priority_ = priority;
            lName = demangledTypename(type.name());
            return this;
        }

        virtual ~Logger()
        {}

#ifndef WNS_NO_LOGGING

        template<class T1>
        void send( T1 t1 )
        {
            wns::logger::Message m(t1, priority_);
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2>
        void send( T1 t1, T2 t2 )
        {
            wns::logger::Message m(t1, priority_);
            m << t2;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3>
        void send( T1 t1, T2 t2, T3 t3 )
        {
            wns::logger::Message m(t1, priority_);
            m << t2 << t3;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3, class T4>
        void send( T1 t1, T2 t2, T3 t3, T4 t4 )
        {
            wns::logger::Message m(t1, priority_);
            m << t2 << t3 << t4;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3, class T4, class T5>
        void send(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
        {
            wns::logger::Message m("", priority_);
            m << t1 << t2 << t3 << t4 << t5;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3, class T4, class T5, class T6>
        void send(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
        {
            wns::logger::Message m("", priority_);
            m << t1 << t2 << t3 << t4 << t5 << t6;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
        void send(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
        {
            wns::logger::Message m("", priority_);
            m << t1 << t2 << t3 << t4 << t5 << t6 << t7;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
        void send(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
        {
            wns::logger::Message m("", priority_);
            m << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
            wns::logger::Logger::send(m);
        }

        template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
        void send(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9)
        {
            wns::logger::Message m("", priority_);
            m << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
            wns::logger::Logger::send(m);
        }
#else

        template<class T1>
        static void DoNothing( T1 ){}

        template<class T1, class T2>
        static void DoNothing( T1, T2 ) {}

        template<class T1, class T2, class T3>
        static void DoNothing( T1, T2, T3 ) {}

        template<class T1, class T2, class T3, class T4>
        static void DoNothing( T1, T2, T3, T4 ) {}

        template<class T1, class T2, class T3, class T4, class T5>
        static void DoNothing(T1, T2, T3, T4, T5 ) {}

        template<class T1, class T2, class T3, class T4, class T5, class T6>
        static void DoNothing(T1, T2, T3, T4, T5, T6 ) {}

        template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
        static void DoNothing(T1, T2, T3, T4, T5, T6, T7 ) {}

        template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
        static void DoNothing(T1, T2, T3, T4, T5, T6, T7, T8 ) {}

        template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
        static void DoNothing(T1, T2, T3, T4, T5, T6, T7, T8, T9 ) {}

#endif

    private:
        int priority_;
    };

    typedef wns::SingletonHolder<Logger> WiMACLogger;

}

#ifndef WNS_NO_LOGGING

#define LOG_TRACE WiMACLogger::getInstance()->prepareForMessage(3, typeid(*this))->send
#define LOG_INFO WiMACLogger::getInstance()->prepareForMessage(2, typeid(*this))->send
#define LOG_WARN WiMACLogger::getInstance()->prepareForMessage(1, typeid(*this))->send


#else

#define LOG_TRACE wimac::Logger::DoNothing
#define LOG_INFO wimac::Logger::DoNothing
#define LOG_WARN wimac::Logger::DoNothing

#endif

#endif

