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

#ifndef WIMAC_PHYUSERCOMMAND_HPP
#define WIMAC_PHYUSERCOMMAND_HPP

#include <functional>

#include <WNS/ldk/Command.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>
#include <WNS/service/phy/power/PowerMeasurement.hpp>
#include <WIMAC/CIRProvider.hpp>
#include <WIMAC/PhyAccessFunc.hpp>
#include <WIMAC/PhyModeProviderCommand.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

namespace wimac {
    class PhyUser;
    class Component;


    /**
     * \brief The PhyUserCommand is the command of the PhyUser functional unit
     */
    class PhyUserCommand :
            public wns::ldk::Command,
            public wimac::PhyModeProviderCommand,
            public wimac::CIRProviderCommand
    {
    public:
        struct {
			wns::Power rxPower_;
			wns::Power interference_;

            std::auto_ptr<PhyAccessFunc> pAFunc_;

        } local;

        struct {
            wns::node::Interface* source_;
            wns::node::Interface* destination_;
            int cellID_;
            wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface> phyModePtr;
            bool measureInterference_;
            wns::CandI estimatedCandI_;
        } peer;

        struct {
            wimac::Component* sourceComponent_;
            bool contentionAccess_;
            bool frameHead_;
        } magic;


        PhyUserCommand()
        {
            peer.measureInterference_ = false;
            peer.source_ = 0;
            peer.destination_ = 0;
            peer.cellID_ = 0;
            peer.estimatedCandI_ = wns::CandI();
            magic.sourceComponent_ = 0;
            magic.contentionAccess_ = false;
            magic.frameHead_ = false;
        }

        PhyUserCommand(const PhyUserCommand& other) :
            wns::ldk::Command(),
            wimac::PhyModeProviderCommand(),
            wimac::CIRProviderCommand()
        {
			local.rxPower_            = other.local.rxPower_;
			local.interference_       = other.local.interference_;
            if (other.local.pAFunc_.get())
                local.pAFunc_.reset(dynamic_cast<wimac::PhyAccessFunc*>(other.local.pAFunc_->clone()));
            peer.measureInterference_ = other.peer.measureInterference_;
            peer.source_              = other.peer.source_;
            peer.destination_         = other.peer.destination_;
            peer.cellID_              = other.peer.cellID_;
            peer.phyModePtr           = other.peer.phyModePtr;
            peer.estimatedCandI_      = other.peer.estimatedCandI_;

            magic.sourceComponent_       = other.magic.sourceComponent_;
            magic.contentionAccess_   = other.magic.contentionAccess_;
            magic.frameHead_          = other.magic.frameHead_;
        }

        const wns::service::phy::phymode::PhyModeInterface& getPhyMode() const { return *(peer.phyModePtr); }
        const wns::service::phy::phymode::PhyModeInterface* getPhyModePtr() const { return peer.phyModePtr.getPtr(); }

        void setPhyMode( const wns::service::phy::phymode::PhyModeInterface& _phyMode )
        {
            peer.phyModePtr = wns::SmartPtr<const wns::service::phy::phymode::PhyModeInterface>
                (dynamic_cast<const wns::service::phy::phymode::PhyModeInterface*>(_phyMode.clone()));
        }

		wns::Ratio getCIR() const
		{
			return wns::Ratio::from_dB( local.rxPower_.get_dBm() - local.interference_.get_dBm() );
		}

        wns::Power getEstimatedIintra() const { return peer.estimatedCandI_.sdma.iIntra; }

    };
}
#endif
