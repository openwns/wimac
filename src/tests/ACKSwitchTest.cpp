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


#include <WIMAC/ACKSwitch.hpp>
#include <WIMAC/services/ConnectionManager.hpp>

#include <WNS/ldk/tests/FUTestBase.hpp>
#include <WNS/ldk/Classifier.hpp>
#include <WNS/ldk/arq/CumulativeACK.hpp>
#include <WNS/pyconfig/Parser.hpp>
#include <WNS/pyconfig/Parser.hpp>
#include <cppunit/extensions/HelperMacros.h>

namespace wimac { namespace tests {

        class TestConnectionManager:
            public wimac::service::ConnectionManagerInterface,
            public wns::ldk::ManagementService
        {
        public:
            TestConnectionManager( wns::ldk::ManagementServiceRegistry* msr,
                                   wns::pyconfig::View /*config*/ ):
                ManagementService( msr )
            {}

            ConnectionIdentifierPtr
            virtual getBasicConnectionFor( const ConnectionIdentifier::CID cid )
            {
                ConnectionIdentifier::Ptr connection
                    = ConnectionIdentifier::Ptr( new ConnectionIdentifier() );
                connection->cid_ = cid * 7;
                return connection;
            }

            void onMSRCreated() {}
        };

        class ClassificationPolicy
        {
        public:
            ClassificationPolicy( wns::ldk::fun::FUN*) : counter(0){}
            wns::ldk::ClassificationID classify( const wns::ldk::CompoundPtr&) { return counter++ % 10; }
        private:
            int counter;
        };

        class ACKSwitchTest :
            public wns::ldk::tests::FUTestBase
        {
            CPPUNIT_TEST_SUITE( ACKSwitchTest );
            CPPUNIT_TEST( unprocessedPackets );
            CPPUNIT_TEST( processedPackets );
            CPPUNIT_TEST_SUITE_END();
        public:

            void
            unprocessedPackets();

            void
            processedPackets();

        private:
            virtual void
            setUpTestFUs();

            virtual void
            tearDownTestFUs();

            virtual wns::ldk::FunctionalUnit*
            getUpperTestFU() const;

            virtual wns::ldk::FunctionalUnit*
            getLowerTestFU() const;

            wimac::ACKSwitch* ackSwitch;
            wns::ldk::arq::CumulativeACK* arq;
            wns::ldk::Classifier<ClassificationPolicy>* classifier;
            TestConnectionManager* connectionManager;
        };

        CPPUNIT_TEST_SUITE_REGISTRATION( ACKSwitchTest );

        void
        ACKSwitchTest::setUpTestFUs()
        {
            wns::pyconfig::Parser emptyconfig;
            ackSwitch = new wimac::ACKSwitch(getFUN(), emptyconfig);

            std::stringstream ss;
            ss << "from openwns.ARQ import CumulativeACK\n"
               << "foo = CumulativeACK(\n"
               << "  windowSize = " << 4 << ",\n"
               << "  resendTimeout = 1.0)\n";
            wns::pyconfig::Parser all;
            all.loadString(ss.str());
            wns::pyconfig::View arqConfig(all, "foo");

            arq = new wns::ldk::arq::CumulativeACK(getFUN(), arqConfig);
            classifier = new wns::ldk::Classifier<ClassificationPolicy>(getFUN(), emptyconfig);
            connectionManager = new wimac::tests::TestConnectionManager(getFUN()->getLayer()->getMSR(), emptyconfig);

            getFUN()->addFunctionalUnit("testFU", ackSwitch);
            getFUN()->addFunctionalUnit("arq", arq);
            getFUN()->addFunctionalUnit("classifier", classifier);
            getFUN()->getLayer()->addManagementService("connectionManager", connectionManager);
        }

        void
        ACKSwitchTest::tearDownTestFUs()
        {
        }

        wns::ldk::FunctionalUnit*
        ACKSwitchTest::getUpperTestFU() const
        {
            return ackSwitch;
        }

        wns::ldk::FunctionalUnit*
        ACKSwitchTest::getLowerTestFU() const
        {
            return getUpperTestFU();
        }

        void
        ACKSwitchTest::unprocessedPackets()
        {
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsDelivered());

            wns::ldk::ClassificationID const cid = 3;
            //create first outgoing DATA Packet!
            wns::ldk::CompoundPtr compound(getFUN()->createCompound());
            arq->activateCommand(compound->getCommandPool());
            wns::ldk::arq::CumulativeACKCommand* arqCommand = (arq->getCommand(compound->getCommandPool()));
            arqCommand->peer.type = wns::ldk::arq::CumulativeACKCommand::I;

            classifier->activateCommand(compound->getCommandPool());
            wns::ldk::ClassifierCommand* classifierCommand = classifier->getCommand(compound->getCommandPool());
            classifierCommand->peer.id = cid;
            sendCompound(compound);

            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsDelivered());

            compound = getLowerStub()->sent[0];
            classifierCommand = classifier->getCommand(compound->getCommandPool());
            CPPUNIT_ASSERT_EQUAL( classifierCommand->peer.id , cid );

            //create second outgoing DATA Packet!
            wns::ldk::CompoundPtr compound2(getFUN()->createCompound());
            arq->activateCommand(compound2->getCommandPool());
            arqCommand = (arq->getCommand(compound2->getCommandPool()));
            arqCommand->peer.type = wns::ldk::arq::CumulativeACKCommand::I;

            classifier->activateCommand(compound2->getCommandPool());
            classifierCommand = classifier->getCommand(compound2->getCommandPool());
            classifierCommand->peer.id = cid;
            sendCompound(compound2);

            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsDelivered());

            compound = getLowerStub()->sent[1];
            classifierCommand = classifier->getCommand(compound->getCommandPool());
            CPPUNIT_ASSERT_EQUAL( classifierCommand->peer.id , cid );

            //receive incoming DATA Packet
            receiveCompound(getLowerStub()->sent[0]);

            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsDelivered());

            compound = getUpperStub()->received[0];
            classifierCommand = classifier->getCommand(compound->getCommandPool());
            CPPUNIT_ASSERT_EQUAL( classifierCommand->peer.id , cid );
        }

        void
        ACKSwitchTest:: processedPackets()
        {
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsDelivered());

            wns::ldk::ClassificationID const cid = 3;
            //create first outgoing DATA Packet!
            wns::ldk::CompoundPtr compound(getFUN()->createCompound());
            arq->activateCommand(compound->getCommandPool());
            wns::ldk::arq::CumulativeACKCommand* arqCommand = (arq->getCommand(compound->getCommandPool()));
            arqCommand->peer.type = wns::ldk::arq::CumulativeACKCommand::RR;

            classifier->activateCommand(compound->getCommandPool());
            wns::ldk::ClassifierCommand* classifierCommand = classifier->getCommand(compound->getCommandPool());
            classifierCommand->peer.id = cid;
            sendCompound(compound);

            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsDelivered());

            compound = getLowerStub()->sent[0];
            classifierCommand = classifier->getCommand(compound->getCommandPool());
            //"cid * 7" is a random factor given by the TestConnectionManager representing the managment cid.
            CPPUNIT_ASSERT_EQUAL( classifierCommand->peer.id , cid * 7);

            //create second outgoing DATA Packet!
            wns::ldk::CompoundPtr compound2(getFUN()->createCompound());
            arq->activateCommand(compound2->getCommandPool());
            arqCommand = (arq->getCommand(compound2->getCommandPool()));
            arqCommand->peer.type = wns::ldk::arq::CumulativeACKCommand::RR;

            classifier->activateCommand(compound2->getCommandPool());
            classifierCommand = classifier->getCommand(compound2->getCommandPool());
            classifierCommand->peer.id = cid;
            sendCompound(compound2);

            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)0, compoundsDelivered());

            compound = getLowerStub()->sent[1];
            classifierCommand = classifier->getCommand(compound->getCommandPool());
            CPPUNIT_ASSERT_EQUAL( classifierCommand->peer.id , cid * 7 );

            //incoming DATA Packet
            receiveCompound(getLowerStub()->sent[0]);

            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsAccepted());
            CPPUNIT_ASSERT_EQUAL((unsigned int)2, compoundsSent());
            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsReceived());
            CPPUNIT_ASSERT_EQUAL((unsigned int)1, compoundsDelivered());

            compound = getUpperStub()->received[0];
            classifierCommand = classifier->getCommand(compound->getCommandPool());
            CPPUNIT_ASSERT_EQUAL( classifierCommand->peer.id , cid );
        }
    }
}
