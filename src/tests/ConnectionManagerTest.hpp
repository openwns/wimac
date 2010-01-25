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


#ifndef WIMAC_TEST_CONNECTIONMANAGERTEST_H
#define WIMAC_TEST_CONNECTIONMANAGERTEST_H

#include <WNS/cppunit/extensions/HelperMacros.h>
#include <WNS/node/Registry.hpp>
#include <WNS/ldk/ldk.hpp>

namespace wimac {

    class ConnectionManagerTest : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE( ConnectionManagerTest );
        CPPUNIT_TEST( removalTest );
        CPPUNIT_TEST_SUITE_END();
    public:
        void setUp();
        void tearDown();
        void removalTest();

    private:
        std::auto_ptr<wns::node::Registry> registry_;
    };
}

#endif

