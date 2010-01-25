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
#include <WIMAC/services/InterferenceCache.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/pyconfig/Parser.hpp>
#include <WNS/ldk/Layer.hpp>
#include <WNS/ldk/tests/LayerStub.hpp>
#include <WNS/node/tests/Stub.hpp>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>


namespace wimac {

    class Component;

    namespace service { namespace tests {

            class InterferenceCacheTest :
               public CppUnit::TestFixture
            {
                CPPUNIT_TEST_SUITE( InterferenceCacheTest );
                CPPUNIT_TEST( writeData );
                CPPUNIT_TEST_SUITE_END();

            public:
                void setUp();
                void tearDown();
                void writeData();

			private:
				std::auto_ptr<wns::ldk::tests::LayerStub> layer_;
				std::auto_ptr<wns::ldk::ManagementServiceRegistry> msr_;
            };
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( wimac::service::tests::InterferenceCacheTest );

using namespace wimac::service;
using namespace wimac::service::tests;

void InterferenceCacheTest::setUp()
{
	layer_.reset( new wns::ldk::tests::LayerStub() );
	msr_.reset( new wns::ldk::ManagementServiceRegistry( layer_.get() ) );

	wns::pyconfig::Parser config;
	config.loadString(
	  "import wimac.Services\n"
	  "interferenceCache = wimac.Services.InterferenceCache( \"interferenceCache\", 0.2, 0.05)\n"
	  "interferenceCache.notFoundStrategy.averageCarrier = \"-96.0 dBm\"\n"
	  "interferenceCache.notFoundStrategy.averageInterference = \"-88.0 dBm\"\n"
	  "interferenceCache.notFoundStrategy.deviationCarrier = \"0.0 mW\"\n"
	  "interferenceCache.notFoundStrategy.deviationInterference = \"0.0 mW\"\n"
	  "interferenceCache.notFoundStrategy.averagePathloss = \"0.0 dB\"\n"
	  );
	layer_->addManagementService
		( "interferenceCache",
		  new InterferenceCache( layer_->getMSR(), wns::pyconfig::View(config, "interferenceCache") ) );
}

void InterferenceCacheTest::writeData()
{
	InterferenceCache* iCache =
		layer_->getManagementService<InterferenceCache>("interferenceCache");
	wns::node::Interface* nodeStub = new wns::node::tests::Stub();
	iCache->storeCarrier( nodeStub, wns::Power::from_dBm( -30.0 ), InterferenceCache::Local );
	iCache->storeCarrier( nodeStub, wns::Power::from_dBm( -40.0 ), InterferenceCache::Local );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( -30.86, iCache->getAveragedCarrier( nodeStub ).get_dBm(), 0.01 );

	iCache->storeInterference( nodeStub, wns::Power::from_dBm( -30.0 ), InterferenceCache::Local );
	iCache->storeInterference( nodeStub, wns::Power::from_dBm( -40.0 ), InterferenceCache::Local );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( -30.86, iCache->getAveragedInterference( nodeStub ).get_dBm(), 0.01 );

	iCache->storeCarrier( nodeStub, wns::Power::from_dBm( -30.0 ), InterferenceCache::Remote );
	iCache->storeCarrier( nodeStub, wns::Power::from_dBm( -40.0 ), InterferenceCache::Remote );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( -31.0, iCache->getAveragedCarrier( nodeStub ).get_dBm(), 0.01 );

	iCache->storeInterference( nodeStub, wns::Power::from_dBm( -30.0 ), InterferenceCache::Remote );
	iCache->storeInterference( nodeStub, wns::Power::from_dBm( -40.0 ), InterferenceCache::Remote );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( -31.0, iCache->getAveragedInterference( nodeStub ).get_dBm(), 0.01 );
}

void InterferenceCacheTest::tearDown()
{
	msr_.reset();
	layer_.reset();
}


