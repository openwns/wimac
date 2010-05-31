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

/**
 * @file
 * @author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#ifndef WIMAC_ERRORMODELLING_HPP
#define WIMAC_ERRORMODELLING_HPP

#include <WNS/ldk/Forwarding.hpp>
#include <WNS/ldk/fu/Plain.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/ErrorRateProviderInterface.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/PowerRatio.hpp>

#include <WIMAC/PhyUser.hpp>
#include <WIMAC/Logger.hpp>

namespace wimac {

    /**
     * @brief The Command of the ErrorModelling.
     */
    class ErrorModellingCommand :
        public wns::ldk::Command,
        public wns::ldk::ErrorRateProviderInterface

    {
    public:
        ErrorModellingCommand()
        {
            local.per = 1;
        }

        virtual double getErrorRate() const
        {
            return local.per;
        }

        struct {
            double per;
        } local;
        struct {} peer;
        struct {} magic;

    };

    /**
     * @brief ErrorModelling implementation of the FU.
     *
     * It maps the SINR for a PhyMode
     * to the Packet Error Rate (PER)
     */
    class ErrorModelling :
        public wns::ldk::fu::Plain<ErrorModelling, ErrorModellingCommand>,
        public wns::ldk::Forwarding<ErrorModelling>
    {
    public:
        ErrorModelling(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

        // Forwarding Interface
        virtual void
        processOutgoing(const wns::ldk::CompoundPtr& componud);

        virtual void
        processIncoming(const wns::ldk::CompoundPtr& componud);

        virtual void
        onFUNCreated();


    private:
        std::string CIRProviderName_;
        std::string PHYModeProviderName_;

        struct Friends {
            FunctionalUnit* CIRProvider;
            FunctionalUnit* PHYModeProvider;
        } friends;
    };
}

#endif

