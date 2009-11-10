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

#ifndef PHYTOOLSTEST_HPP
#define PHYTOOLSTEST_HPP

#include <stdexcept>
#include <WNS/cppunit/extensions/HelperMacros.h>

namespace wimac{

    class PHYToolsTest :
        public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(PHYToolsTest);
        CPPUNIT_TEST(testFeatures);
        CPPUNIT_TEST_SUITE_END();
    public:
        void setUp();
        void tearDown();
        void testFeatures();
    };
}

#endif

