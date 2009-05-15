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

#include <WIMAC/tests/ErrorModellingTest.hpp>

#include <WNS/pyconfig/Parser.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/helper/FakePDU.hpp>
#include <WNS/osi/PDU.hpp>

#include <WNS/speetcl/pdu.hpp>
#include <WNS/speetcl/evsched.hpp>

#include <deque>
#include <vector>

using namespace wimac;

CPPUNIT_TEST_SUITE_REGISTRATION( ErrorModellingTest );


void
ErrorModellingTest::setUp()
{
	PacketSize_ = 1056;
	MapLength_ = 22; // Max. CIR of CIR to SER Map

	ImportMappings();

	layer = new wns::ldk::Layer();
	fun = new wns::ldk::fun::Main(layer);

	setUpUpper();
	setUpCirProvider();
	setUpPHYModeProvider();
	setUpErrorModelling();
	setUpLower();

	upper
		->connect(errormodelling)
		->connect(lower);

	fun->addFunctionalUnit("upper",upper);
	fun->addFunctionalUnit("phyuser",cirprovider);
	fun->addFunctionalUnit("timing",phymodeprovider);
	fun->addFunctionalUnit("myErrorModelling",errormodelling);
	fun->addFunctionalUnit("lower",lower);

	fun->onFUNCreated();

	wns::simulator::getEventScheduler()->reset();

} //setUp

void
ErrorModellingTest::tearDown()
{

	delete fun;
	delete layer;

} // tearDown



/////////////////////////////// Tests /////////////////////////////////////////

void ErrorModellingTest::TestPassThrough()
{
	upper->sendData( CompoundPtr(new Compound(fun->getProxy()->createCommandPool()) ) );

	CPPUNIT_ASSERT_EQUAL( size_t(1), lower->sent.size() );

} //TestPassThrough


void ErrorModellingTest::TestPHYModeBPSK12()
{
	Test(PacketSize_, wimac::PHYTools::BPSK12, MapLength_);

} //TestPHYModeBPSK12


void ErrorModellingTest::TestPHYModeQPSK12()
{
	Test(PacketSize_, wimac::PHYTools::QPSK12, MapLength_);

} //TestPHYModeQPSK12


void ErrorModellingTest::TestPHYModeQPSK34()
{
	Test(PacketSize_, wimac::PHYTools::QPSK34, MapLength_);

} //TestPHYModeQPSK34


void ErrorModellingTest::TestPHYModeQAM16_12()
{
	Test(PacketSize_, wimac::PHYTools::QAM16_12, MapLength_);

} //TestPHYModeQAM16_12


void ErrorModellingTest::TestPHYModeQAM16_34()
{
	Test(PacketSize_, wimac::PHYTools::QAM16_34, MapLength_);

} //TestPHYModeQAM16_34


void ErrorModellingTest::TestPHYModeQAM64_23()
{
	Test(PacketSize_, wimac::PHYTools::QAM64_23, MapLength_);

} //TestPHYModeQAM64_23


void ErrorModellingTest::TestPHYModeQAM64_34()
{
	Test(PacketSize_, wimac::PHYTools::QAM64_34, MapLength_);

} //TestPHYModeQAM64_34




/////////////////////// Private ////////////////////////////////////////////////

void
ErrorModellingTest::setUpUpper()
{
	wns::pyconfig::Parser emptyConfig;
	upper = new tools::Stub(fun, emptyConfig);

} //setUpUpper


void
ErrorModellingTest::setUpPHYModeProvider()
{
	wns::pyconfig::Parser emptyConfig;
	phymodeprovider = new wimac::PHYModeProvider(fun, emptyConfig);


} //setUpPHYModeProvider


void
ErrorModellingTest::setUpCirProvider()
{
	//Not in use at the moment
	wns::pyconfig::Parser emptyConfig;
	cirprovider = new tools::Stub(fun, emptyConfig);

} //setUpCirProvider



void
ErrorModellingTest::setUpErrorModelling()
{
	std::stringstream ss;
	ss << "import wimac.ErrorModelling\n"
	   << "errormodelling = wimac.ErrorModelling.ErrorModelling( cirProvider = 'phyuser',"
	   << " phyModeProvider = 'timing')\n";

	wns::pyconfig::Parser all;
	all.loadString(ss.str());

	wns::pyconfig::View config(all, "errormodelling");
	errormodelling = new ErrorModelling(fun, config);

} //setUpErrorModelling


void
ErrorModellingTest::setUpLower()
{
	wns::pyconfig::Parser emptyConfig;
	lower = new tools::Stub(fun, emptyConfig);

} //setUpLower


void
ErrorModellingTest::ImportMappings()
{
	std::stringstream ss;
	ss << "import wimac.ErrorModelling\n"
	   << "errormodelling = wimac.ErrorModelling.ErrorModelling(PrintMappings=False)\n";

	wns::pyconfig::Parser all;
	all.loadString(ss.str());
	wns::pyconfig::View config = all.getView("errormodelling");


    // Import the Mappings from the pyconfig::Parser
	wns::pyconfig::View configview = config.getView("cir2ser_BPSK12");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_BPSK12_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

	configview = config.getView("cir2ser_QPSK12");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_QPSK12_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

	configview = config.getView("cir2ser_QPSK34");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_QPSK34_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

	configview = config.getView("cir2ser_QAM16_12");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_QAM16_12_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

	configview = config.getView("cir2ser_QAM16_34");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_QAM16_34_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

	configview = config.getView("cir2ser_QAM64_23");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_QAM64_23_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

	configview = config.getView("cir2ser_QAM64_34");
	for(int i = 0; i < configview.len("mapping"); ++i)
	{
		wns::pyconfig::View mo = configview.getView("mapping", i);
		cir2ser_QAM64_34_[mo.get<double>("cir")] = mo.get<double>("ser");
	}

} // ImportMappings


void
ErrorModellingTest::CreateSendCompound(int PacketSize, wimac::PHYTools::PHYMode PHYMode, double cir)
{
    // Create compound
	CompoundPtr compound =
		CompoundPtr(new Compound(fun->getProxy()->createCommandPool(),
								 wns::osi::PDUPtr(new wns::ldk::helper::FakePDU(Bit(PacketSize)))));

//	errormodelling->activateCommand( compound->getCommandPool() )
//		->local.cir.set_dB(cir);

	dynamic_cast<wimac::timing2::TimingCommand*>
		(phymodeprovider->activateCommand( compound->getCommandPool() ))
		->magic.phyMode_= PHYMode;

    //Send compound
	lower->onData(compound);

} //CreateSendCompound


void
ErrorModellingTest::PrintResults()
{
	int j = 0;
	for(std::deque<CompoundPtr>::iterator i = upper->received.begin(); i != upper->received.end(); ++i)
	{
		std::cout << " ErrorModellingTest: PHYMode="
				  << dynamic_cast<wimac::timing2::TimingCommand*>
			         (phymodeprovider->getCommand( (*i) ->getCommandPool() ))
			         ->magic.phyMode_

				  << "; CIR="
                  << errormodelling->getCommand((*i)->getCommandPool() )->local.cir
				  <<"  => PER="
				  << errormodelling->getCommand((*i)->getCommandPool() )->local.per;

		std::cout << "    Calculated Control PER=" <<  Calculate(j)
				  << std::endl;
		++j;
	}

}  //PrintResults


double
ErrorModellingTest::Calculate(int CompoundNr)
{
	// Reimplementation of ErrorModelling
	double cir; // Carry Interference Ratio
	double ser; // Symbols Error Rate
	double per; // Packet Error Rate
	int spp;    // Symbols per Packet
	wimac::PHYTools::PHYMode PHYMode;
	std::map<double, double>* cir2ser;


	//Get Cir
	cir = errormodelling->getCommand(upper->received[CompoundNr]->getCommandPool() )->local.cir.get_dB();

    // Get PHYMode
	PHYMode = phymodeprovider->getCommand(upper->received[CompoundNr]->getCommandPool() )->magic.phyMode_;

    // Get Mapping
	switch(PHYMode)
	{
	case ( wimac::PHYTools::BPSK12 ):
		cir2ser = &cir2ser_BPSK12_;
		break;
	case ( wimac::PHYTools::QPSK12 ):
		cir2ser = &cir2ser_QPSK12_;
        break;
	case ( wimac::PHYTools::QPSK34 ):
		cir2ser = &cir2ser_QPSK34_;
        break;
	case ( wimac::PHYTools::QAM16_12 ):
		cir2ser = &cir2ser_QAM16_12_;
        break;
	case ( wimac::PHYTools::QAM16_34 ):
		cir2ser = &cir2ser_QAM16_34_;
        break;
	case ( wimac::PHYTools::QAM64_23 ):
		cir2ser = &cir2ser_QAM64_23_;
		break;
	case ( wimac::PHYTools::QAM64_34 ):
		cir2ser = &cir2ser_QAM64_34_;
		break;
	default:
		assure(0, "ErrorModellingTest: Unknown PHYMode \n");
	}

    // Do the Mapping
	for (std::map<double, double>::iterator it = (*cir2ser).begin(); ((*it).first <= cir) && (it != (*cir2ser).end())  ; ++it)
	{
		ser = (*it).second;
	}

    // Calculate PER
	spp = int(ceil((double(upper->received[CompoundNr]->getLengthInBits()))
					/(double(PHYTools::getBitsPerSymbol(PHYMode)))));

    // FIXME: Wrongness because of truncation of the Symbols per Packet
	per=1-pow((1-ser),spp);

	return per;

} //Calculate


void ErrorModellingTest::Test(int PacketSize, wimac::PHYTools::PHYMode PHYMode, int MapLength) 
{
	upper->flush();

	for (int i=0; i <= MapLength; ++i)
	{
		CreateSendCompound(PacketSize,  PHYMode,  i);
	}

    // Should the results be printed?
//	PrintResults();

	int j = 0;
	for(std::deque<CompoundPtr>::iterator i = upper->received.begin(); i != upper->received.end(); ++i)
	{
			CPPUNIT_ASSERT_EQUAL( Calculate(j),
						  errormodelling->getCommand(
							  upper->received[j]->getCommandPool() )->local.per );
		++j;
	}

}



