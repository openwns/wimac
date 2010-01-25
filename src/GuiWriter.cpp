/******************************************************************************
 * WiFiMac                                                                    *
 * This file is part of openWNS (open Wireless Network Simulator)
 * _____________________________________________________________________________
 *
 * Copyright (C) 2004-2007
 * Chair of Communication Networks (ComNets)
 * Kopernikusstr. 16, D-52074 Aachen, Germany
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

#include <WIMAC/GuiWriter.hpp>
//#include <WIFIMAC/convergence/PhyUser.hpp>
//#include <WIFIMAC/convergence/TxDurationSetter.hpp>
//#include <WIFIMAC/lowerMAC/Manager.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>
#include <WNS/module/Base.hpp>
#include <WNS/ldk/concatenation/Concatenation.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>

#include <boost/tuple/tuple.hpp>

#include <WIMAC/PhyUser.hpp>
#include <sstream>

using namespace wimac;

int GuiWriter::gui_station_counter_ = 0;

//STATIC_FACTORY_REGISTER_WITH_CREATOR(
//    wifimac::convergence::GuiWriter,
//    wns::ldk::FunctionalUnit,
//    "wifimac.convergence.GuiWriter",
//    wns::ldk::FUNConfigCreator);

GuiWriter::GuiWriter(wns::probe::bus::ContextCollectorPtr guiProbe, PhyUser* phyuser):
    first_created(true)
{
    gui_station_id_ = gui_station_counter_;
    gui_station_counter_ ++;

    guiProbe_ = guiProbe;
    phyuser_ = phyuser;

}
// GuiWriter



GuiWriter::~GuiWriter()
{ }


/*void GuiWriter::setManagerAndFun(wifimac::lowerMAC::Manager* manager, wns::ldk::fun::FUN* fun)
{
    amanager = manager;
    afun = fun;
}
*/

/*void GuiWriter::writeToProbe(int macaddr, int frametype, int aPhyMode, wns::simulator::Time frameTxDuration, int channel, int TxPower)
{
    if(!guiProbe->hasObserver())
        return;

    std::stringstream guiStr, guiStr1, guiStr2, guiStr3;

    if(first_created)
    {
        first_created = false;
        guiStr << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs STA: " << macaddr << " CREATED";
        guiProbe->put(0.0, boost::make_tuple("guiText", guiStr.str()));

        guiStr1 << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs RegistChannel: " << channel << "  S: " << macaddr << "  IEEE802.11 MAC Id: " << macaddr ;//<< " Lx: " <<  << " Ly: " <<  << " Lz: " << ;
        guiProbe->put(0.0, boost::make_tuple("guiText", guiStr1.str()));
//0.0000  Usecs STA: 0 CREATED
//0.0000  Usecs RegistChannel: 0  S: 0   IEEE802.11 MAC Id: 0 Lx: 0.010000  Ly: 0.010000  Lz: NaN
    }


    guiStr2 << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs P: " << macaddr << "  ";

    switch(frametype)
    {
        case 1:
            guiStr2<< "T: preamble";
            break;
        case 2:
            guiStr2<< "T: data";
            break;
        case 3:
            guiStr2<< "T: data_txop";
            break;
        case 4:
            guiStr2<< "T: ack";
            break;
        case 5:
            guiStr2<< "T: beacon";
            break;
        default:
            guiStr2<< "T: unknown";
            break;

    }

    guiProbe->put(0.0, boost::make_tuple("guiText", guiStr2.str()));



    guiStr3 << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs C: " << channel << "  S: " << macaddr << "  L: " << frameTxDuration * 1000000 << "  M: " << aPhyMode + 999 << "  P: " << TxPower;
//    guiStr1 << wns::simulator::getEventScheduler()->getTime() << " Usecs C: " << func->subBand << " S: " << friends.manager->getMACAddress() << " L: " << frameTxDuration * 1000000  << " M: " << friends.manager->getPhyMode(compound->getCommandPool()) << " P: " << 200.00;

    guiProbe->put(0.0, boost::make_tuple("guiText", guiStr3.str()));


} // writeToProbe
*/
void GuiWriter::writeToProbe(const wns::ldk::CompoundPtr& compound, int macaddr)
{
    if(!guiProbe_->hasObservers())
        return;

    std::stringstream guiStr, guiStr1, guiStr2, guiStr3;

//    wns::simulator::Time frameTxDuration = afun->getCommandReader(txDurationProviderCommandName)->
//       readCommand<wifimac::convergence::TxDurationProviderCommand>(compound->getCommandPool())->getDuration();
    unsigned int aPhyMode = 0 ;
    //yxn: PhyModeInterface is virtual class, need to be implemented.
    //((rise::plmapping::PhyMode* )command->local.pAFunc_->phyMode_)->getModulation();

    int channel = phyuser_->getCommand( compound->getCommandPool() )->local.pAFunc_->subBand_;
    int frametype = 0; //unknown type55
    double frameTxDuration =  phyuser_->getCommand( compound->getCommandPool() )->local.pAFunc_->transmissionStop_ - phyuser_->getCommand( compound->getCommandPool() )->local.pAFunc_->transmissionStart_;

    int TxPower = 200;

    if(first_created)
    {
        first_created = false;
        guiStr << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs STA: " << macaddr << " CREATED";
        guiProbe_->put(0.0, boost::make_tuple("guiText", guiStr.str()));

        guiStr1 << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs RegistChannel: " << channel << "  S: " << gui_station_id_ << "  IEEE802.11 MAC Id: " << macaddr ;//<< " Lx: " <<  << " Ly: " <<  << " Lz: " << ;
        guiProbe_->put(0.0, boost::make_tuple("guiText", guiStr1.str()));
//0.0000  Usecs STA: 0 CREATED
//0.0000  Usecs RegistChannel: 0  S: 0   IEEE802.11 MAC Id: 0 Lx: 0.010000  Ly: 0.010000  Lz: NaN
    }


    guiStr2 << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs P: " << gui_station_id_ << "  ";

    switch(frametype)
    {
        case 1:
            guiStr2<< "T: preamble";
            break;
        case 2:
            guiStr2<< "T: data";
            break;
        case 3:
            guiStr2<< "T: data_txop";
            break;
        case 4:
            guiStr2<< "T: ack";
            break;
        case 5:
            guiStr2<< "T: beacon";
            break;
        default:
            guiStr2<< "T: unknown";
            break;

    }

    guiProbe_->put(0.0, boost::make_tuple("guiText", guiStr2.str()));



    guiStr3 << wns::simulator::getEventScheduler()->getTime() * 1000000 << " Usecs C: " << channel << "  S: " << gui_station_id_ << "  L: " << frameTxDuration * 1000000 << "  M: " << aPhyMode << "  P: " << TxPower;
//    guiStr1 << wns::simulator::getEventScheduler()->getTime() << " Usecs C: " << func->subBand << " S: " << friends.manager->getMACAddress() << " L: " << frameTxDuration * 1000000  << " M: " << friends.manager->getPhyMode(compound->getCommandPool()) << " P: " << 200.00;

    guiProbe_->put(0.0, boost::make_tuple("guiText", guiStr3.str()));


} // writeToProbe

