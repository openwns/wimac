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


#include <cppunit/extensions/HelperMacros.h>
#include <frame/SingleCompoundCollector.hpp>
#include <node/Node.hpp>

namespace wimac { namespace frame { namespace tests {

            class SingleCompoundCollectorTest :
                public CppUnit::TestFixture
            {
                CPPUNIT_TEST_SUITE( SingleCompoundCollectorTest );
                //CPPUNIT_TEST( scheduleCompound );
                CPPUNIT_TEST_SUITE_END();
            public:
                void setUp();
                void tearDown();
            };

            void SingleCompoundCollectorTest::setUp()
            {
                wns::pyconfig::Parser config;
                config.loadString(
                    "import wimac.Services\n"
                    "import wns.Node\n"
                    "class TestNode(wns.Node.Node):\n"
                    "  dll = None\n"
                    "  def __init__(self):\n"
                    "    super(TestNode, self).__init__(\"TestNode\")\n"
                    "    self.dll = wimac.Stations.BaseStation(self)\n"
                    "node = TestNode()\n"

                    );

                wns::node::Node* node = new wns::node::Node( registry_.get(), config.getView("node") );
                registry_->insert( "node", node );

            }

        }
    }
}

