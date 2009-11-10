/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 5, D-52074 Aachen, Germany
 * phone: ++49-241-80-27910,
 * fax: ++49-241-80-22242
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

#include <WIMAC/PhyMode.hpp>
#include <sstream>
#include <cppunit/extensions/HelperMacros.h>
#include <WNS/pyconfig/Parser.hpp>


namespace wimac {
namespace test {

class PhyModeTest :
    public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(PhyModeTest);
    CPPUNIT_TEST(testPhyMode);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void testPhyMode();
};

CPPUNIT_TEST_SUITE_REGISTRATION( PhyModeTest );

void PhyModeTest::setUp()
{
}

void PhyModeTest::tearDown()
{
}

void PhyModeTest::testPhyMode()
{
    std::stringstream ss;
    ss << "import wimac.PhyMode\n"
       << "phymode = wimac.PhyMode.PhyMode()\n"
       << "phymode.modulation = wimac.PhyMode.QPSK\n"
       << "phymode.code = wimac.PhyMode.Code12\n";

    wns::pyconfig::View config( wns::pyconfig::Parser::fromString(ss.str()));
    PhyMode phymode(config.getView("phymode"));
}

}
}
