/*******************************************************************************
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2009
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

#include <WIMAC/tests/PHYToolsTest.hpp>
#include <WIMAC/PHYTools.hpp>
#include <WIMAC/Logger.hpp>
#include <vector>

using namespace wimac;

CPPUNIT_TEST_SUITE_REGISTRATION( PHYToolsTest );

void PHYToolsTest::setUp()
{
}

void PHYToolsTest::tearDown()
{
}

void PHYToolsTest::testFeatures()
{
	std::vector<double> snr = std::vector<double>(7);
	snr[0] = 24.4;
	snr[1] = 22.7;
	snr[2] = 18.2;
 	snr[3] = 16.4;
 	snr[4] = 11.2;
 	snr[5] = 9.4;
	snr[6] = 6.4;

	//SNR -> PHYMODE
	std::vector<PHYTools::PHYMode> phymode = std::vector<PHYTools::PHYMode>(7);
	for( int i = 0; i < 7; ++i )
	{
		phymode[i] = PHYTools::getPHYModeBySNR( snr[i] );
		LOG_INFO( " snr ", snr[i], " -> PHYMode ", phymode[i]," i ", i );
	}

	CPPUNIT_ASSERT( phymode[0] == PHYTools::QAM64_34 );
	CPPUNIT_ASSERT( phymode[1] == PHYTools::QAM64_23 );
	CPPUNIT_ASSERT( phymode[2] == PHYTools::QAM16_34 );
	CPPUNIT_ASSERT( phymode[3] == PHYTools::QAM16_12 );
	CPPUNIT_ASSERT( phymode[4] == PHYTools::QPSK34 );
	CPPUNIT_ASSERT( phymode[5] == PHYTools::QPSK12 );
	CPPUNIT_ASSERT( phymode[6] == PHYTools::BPSK12 );

	std::vector<int> bitsPerSymbol = std::vector<int>(7);
	//PHYMODEa -> BitsPerSymbol
	for( int i = 0; i < 7; ++i )
	{
		bitsPerSymbol[i] = PHYTools::getBitsPerSymbol( phymode[i] );
		//LOG_INFO( " phymode ", phymode[i] , " -> BitsPerSymbol ", bitsPerSymbol[i], " i ", i );
		//LOG_INFO( " -> Datarate : ", bitsPerSymbol[i] / 0.0000138888888888 );
	}

	CPPUNIT_ASSERT( bitsPerSymbol[6] == 96 );
	CPPUNIT_ASSERT( bitsPerSymbol[5] == 192 );
	CPPUNIT_ASSERT( bitsPerSymbol[4] == 288 );
	CPPUNIT_ASSERT( bitsPerSymbol[3] == 384 );
	CPPUNIT_ASSERT( bitsPerSymbol[2] == 576 );
	CPPUNIT_ASSERT( bitsPerSymbol[1] == 768 );
	CPPUNIT_ASSERT( bitsPerSymbol[0] == 864 );

					//BitsPerSymbol -> PHYMODEb
	for( int i = 0; i < 7; ++i )
	{
		phymode[i] = PHYTools::getPHYModeByBitsPerSymbol( bitsPerSymbol[i] );
	}
					//a==b?
	CPPUNIT_ASSERT( phymode[0] == PHYTools::QAM64_34 );
	CPPUNIT_ASSERT( phymode[1] == PHYTools::QAM64_23 );
	CPPUNIT_ASSERT( phymode[2] == PHYTools::QAM16_34 );
	CPPUNIT_ASSERT( phymode[3] == PHYTools::QAM16_12 );
	CPPUNIT_ASSERT( phymode[4] == PHYTools::QPSK34 );
	CPPUNIT_ASSERT( phymode[5] == PHYTools::QPSK12 );
	CPPUNIT_ASSERT( phymode[6] == PHYTools::BPSK12 );
}

