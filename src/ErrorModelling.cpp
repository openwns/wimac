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

/**
 * \file
 * \author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/ErrorModelling.hpp>
#include <WIMAC/CIRProvider.hpp>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::ErrorModelling,
	wns::ldk::FunctionalUnit,
	"wimac.ErrorModelling",
	wns::ldk::FUNConfigCreator);


ErrorModelling::ErrorModelling(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
	wns::ldk::CommandTypeSpecifier< ErrorModellingCommand >(fun),
	wns::ldk::HasReceptor<>(),
	wns::ldk::HasConnector<wns::ldk::RoundRobinConnector>(),
	wns::ldk::HasDeliverer<>(),
	wns::Cloneable< ErrorModelling >(),
	CIRProviderName_(config.get<std::string>("cirProvider")),
	PHYModeProviderName_( config.get<std::string>("phyModeProvider")),
	friends()
{
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

	// PrintMappings
	if ( config.get<bool>("PrintMappings") )
  	{
		PrintMappings();
  	}

}


bool
ErrorModelling::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
	return getConnector()->hasAcceptor(compound);
} // isAccepting


void
ErrorModelling::doSendData(const wns::ldk::CompoundPtr& compound)
{
        assure(isAccepting(compound),
			   "sendData called although the CRC is not accepting! \n");

	activateCommand(compound->getCommandPool());

		LOG_INFO( getFUN()->getName(), " ErrorModelling: Outgoing Compound passed!" );
		getConnector()->getAcceptor(compound)->sendData(compound);

} // doSendData


void
ErrorModelling::doOnData(const wns::ldk::CompoundPtr& compound)
{
	double cir=0.0; // Carrier Interference Ratio
	double cir1 = 0.0;
	double cir2 = 0.0;
	double ser=1.0; // Symbol Error Rate
	double ser1 = 1.0;
	double ser2 = 1.0;
	double per=1.0; // Packet Error Rate
	double spp=0.0;    // Symbols per Packet
	//wimac::PHYTools::PHYMode phyMode = wimac::PHYTools::ERROR;
	std::map<double, double>* cir2ser = NULL;


	//Get Cir

	//FIXME: To this: cir should appear vom CIRProvider
	cir= dynamic_cast<CIRProviderCommand*>
		(friends.CIRProvider->getCommand(compound->getCommandPool()))->getCIR().get_dB();



	// Get PHYMode
	const wns::service::phy::phymode::PhyModeInterface*	phyModePtr = dynamic_cast<wimac::PhyModeProviderCommand*>
		(friends.PHYModeProvider->getCommand(compound->getCommandPool()))
		->getPhyModePtr();

	// Get Mapping
	// [rs]: try to change the way the SINR2PER mapping is done...
	if (phyModePtr->nameMatches("BPSK-WIMAX-1/2")) {
		cir2ser = &cir2ser_BPSK12_;
	} else if (phyModePtr->nameMatches("QPSK-WIMAX-1/2")) {
		cir2ser = &cir2ser_QPSK12_;
	} else if (phyModePtr->nameMatches("QPSK-WIMAX-3/4")) {
		cir2ser = &cir2ser_QPSK34_;
	} else if (phyModePtr->nameMatches("QAM16-WIMAX-1/2")) {
		cir2ser = &cir2ser_QAM16_12_;
	} else if (phyModePtr->nameMatches("QAM16-WIMAX-3/4")) {
		cir2ser = &cir2ser_QAM16_34_;
	} else if (phyModePtr->nameMatches("QAM64-WIMAX-2/3")) {
		cir2ser = &cir2ser_QAM64_23_;
	} else if (phyModePtr->nameMatches("QAM64-WIMAX-3/4")) {
		cir2ser = &cir2ser_QAM64_34_;
	} else {
		assure(0, "ErrorModelling: Unknown PhyMode "<<phyModePtr->getString());
	}

    // Do the mapping by linear interpolation of logarithmic values
	for (std::map<double, double>::iterator it = (*cir2ser).begin(); (it != cir2ser->end()) && (it->first <= cir) ; ++it)
	{
		cir1 = it->first;
		ser1 = it->second;

		std::map<double, double>::iterator tmp = it;
		tmp++;

		if (tmp == cir2ser->end()) 
		{
			ser2 = 1E-10; // we assume 1E-10 as default for lowest SER
			cir2 = cir1+1.0;
		}
		else
		{
			ser2 = (tmp)->second;
			cir2 = (tmp)->first;
		}
	}

	if (ser1 < 1E-10)
		ser1 = 1E-10;

	if (ser2 < 1E-10)
		ser2 = 1E-10;

	double ser1log = log(ser1)/log(10.0);
	double ser2log = log(ser2)/log(10.0);

	double serlog = ser1log + (cir2 - cir1) * (ser2log - ser1log);

	ser = pow(10.0, serlog);


	// Calculate PER

	// PDUs are transmitted in bursts and bursts have to be multiples of OFDM
	// symbols. PDUs, especially ACK-PDUS (12bit) might be considerably smaller
	// so rounding them to full OFDMsymbols would introduce too high error
	// probabilities.

	spp = double(compound->getLengthInBits())
		/ phyModePtr->getBitsPerSymbol();


	// FIXME: Wrongness because of truncation of the Symbols per Packet
	per=1-pow((1-ser),spp);



	//  Output
	this->getCommand( compound->getCommandPool() )->local.per = per;

	LOG_INFO( getFUN()->getName(),
			  ": doOnData!  CIR=",cir," PHY Mode=", phyModePtr->getString(),
			  "; SER=",ser,
			  " ; PER=",getCommand( compound->getCommandPool() )->local.per);

	getDeliverer()->getAcceptor(compound)->onData(compound);

} //onData


void
ErrorModelling::doWakeup()
{
	getReceptor()->wakeup();

} // wakeup


void
ErrorModelling::onFUNCreated()
{
	friends.CIRProvider = getFUN()->findFriend<FunctionalUnit*>(CIRProviderName_);
	assure(friends.CIRProvider,
		   "ErrorModelling requires a PHYUser friend with name '"
		   + CIRProviderName_ + "' \n");
	friends.PHYModeProvider = getFUN()->findFriend<FunctionalUnit*>(PHYModeProviderName_);
	assure(friends.PHYModeProvider,
		   "ErrorModelling requires a PHYModeProvider friend with name '"
		   + PHYModeProviderName_ + "' \n");

} // onFUNCreated()


void
ErrorModelling::PrintMappings()
{
	std::map<double, double>::iterator it;

	std::cout << "ErrorModelling: cir2ser_BPSK12[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_BPSK12_.begin(); it != cir2ser_BPSK12_.end()  ; ++it)
	{
		std::cout <<"                           (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;

	std::cout << "ErrorModelling: cir2ser_QPSK12[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_QPSK12_.begin(); it != cir2ser_QPSK12_.end()  ; ++it)
	{
		std::cout <<"                           (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;


	std::cout << "ErrorModelling: cir2ser_QPSK34[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_QPSK34_.begin(); it != cir2ser_QPSK34_.end()  ; ++it)
	{
		std::cout <<"                           (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;

	std::cout << "ErrorModelling: cir2ser_QAM16_12[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_QAM16_12_.begin(); it != cir2ser_QAM16_12_.end()  ; ++it)
	{
		std::cout <<"                             (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;


	std::cout << "ErrorModelling: cir2ser_QAM16_34[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_QAM16_34_.begin(); it != cir2ser_QAM16_34_.end()  ; ++it)
	{
		std::cout <<"                             (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;

	std::cout << "ErrorModelling: cir2ser_QAM64_23[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_QAM64_23_.begin(); it != cir2ser_QAM64_23_.end()  ; ++it)
	{
		std::cout <<"                             (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;

	std::cout << "ErrorModelling: cir2ser_QAM64_34[cir, ser]\n";
	for (std::map<double, double>::iterator it = cir2ser_QAM64_34_.begin(); it != cir2ser_QAM64_34_.end()  ; ++it)
	{
		std::cout <<"                             (" << (*it).first << ", " << (*it).second << ")\n";
	}
	std::cout << std::endl;
}

