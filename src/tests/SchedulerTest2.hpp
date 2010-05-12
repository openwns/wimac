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

#ifndef SCHEDULERTEST_HPP
#define SCHEDULERTEST_HPP


#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/Classifier.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/OldFlowSeparator.hpp>
#include <WNS/ldk/tools/Stub.hpp>
#include <WNS/ldk/Delayed.hpp>
#include <WNS/ldk/buffer/Dropping.hpp>
#include <WNS/pyconfig/Parser.hpp>
#include <WNS/cppunit/extensions/HelperMacros.h>
#include <stdexcept>

namespace wimac{ namespace schedulertest {


        class RLCCommand :
        public wns::ldk::ClassifierCommand
        {
        public:
            RLCCommand()
            {
                peer.id = 0;
                magic.pduID = 0;
            }

            struct {
                int pduID;
            } magic;
        };

        class MyLayer :
        public wns::ldk::Layer
        {

        public:
            std::auto_ptr<ConnectionManager> connectionManager_;
            Scheduler<MyLayer, RLCCommand>* scheduler;

        };

        class MyKey
        {
        public:
            MyKey(wns::ldk::Layer* _layer, const wns::ldk::CompoundPtr compound, int);
            bool operator<(const MyKey& other) const
            {
                return id < other.id;
            }

            std::string str()	// FIXME(fds) should be streaming
            {
                std::stringstream ss;
                ss << " id :" << id;
                return ss.str();
            }

            int id;
        };

        typedef wns::ldk::OldFlowSeparator<wns::ldk::buffer::Dropping, MyKey> Buffers;

        class SchedulerTest : public CppUnit::TestFixture  {
            CPPUNIT_TEST_SUITE(SchedulerTest);
            CPPUNIT_TEST(testTransmitDuration);
            CPPUNIT_TEST(testEmptyFrame);
            CPPUNIT_TEST(testNumberOfBursts);
            CPPUNIT_TEST(testAllBurstsFull);
            CPPUNIT_TEST(testFullBurst);
            CPPUNIT_TEST_SUITE_END();
        public:
            void setUp();
            void tearDown();
            void testTransmitDuration();
            void testEmptyFrame();
            void testNumberOfBursts();
            void testFullBurst();
        private:
            static const double FrameDuration;
            static const double PDUtransmitDuration;

            std::auto_ptr<MyLayer> layer;
            std::auto_ptr<wns::ldk::tools::Stub> upper;
            std::auto_ptr<Buffers> mybuffers;
            std::auto_ptr<Scheduler<MyLayer, RLCCommand>> myScheduler;
            std::auto_ptr<wns::ldk::tools::Stub> lower;
            std::auto_ptr<wns::ldk::Classifier> myClassifier;
            std::auto_ptr<ConnectionManager> myConnectionManager;
            std::auto_ptr<ConnectionManager> myULMapWriter;
            std::auto_ptr<ConnectionManager> myDLMapWriter;
        };
    }
}
#endif

