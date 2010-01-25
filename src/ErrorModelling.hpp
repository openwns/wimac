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

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/ldk/ErrorRateProviderInterface.hpp>
#include <WNS/ldk/RoundRobinConnector.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/PowerRatio.hpp>

#include <WIMAC/PhyUser.hpp>
#include <WIMAC/Logger.hpp>

namespace wimac {

    class ConnectionClassifier;

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
            local.destructorCalled = NULL;
            local.cir.set_dB(0);

        }

        ~ErrorModellingCommand()
        {
            if(NULL != local.destructorCalled)
                *local.destructorCalled = true;
        }


        virtual double getErrorRate() const
        {
            return local.per;
        }

        struct {
            double per;
            long *destructorCalled;
            wns::Ratio cir;
        } local;
        struct {} peer;
        struct {} magic;

    };

    /**
     * @brief ErrorModelling implementation of the FU.
     *
     * It maps the Carry Interference Ratio (CIR) for a PhyMode
     * to the Symbol Error Rate (SER) and calculate the
     * Packet Error Rate (PER).
     */
    class ErrorModelling :
        public virtual wns::ldk::FunctionalUnit,
        public wns::ldk::CommandTypeSpecifier< ErrorModellingCommand >,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<wns::ldk::RoundRobinConnector>,
        public wns::ldk::HasDeliverer<>,
        public wns::Cloneable< ErrorModelling >
    {
    public:
        ErrorModelling(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);

        virtual void
        doSendData(const wns::ldk::CompoundPtr& compound);

        virtual void
        doOnData(const wns::ldk::CompoundPtr& compound);

        virtual void
        onFUNCreated();

        void
        printMappings();


    private:
        virtual bool
        doIsAccepting(const wns::ldk::CompoundPtr& compound) const;

        virtual void
        doWakeup();

        std::map<double, double> cir2ser_BPSK12_;
        std::map<double, double> cir2ser_QPSK12_;
        std::map<double, double> cir2ser_QPSK34_;
        std::map<double, double> cir2ser_QAM16_12_;
        std::map<double, double> cir2ser_QAM16_34_;
        std::map<double, double> cir2ser_QAM64_23_;
        std::map<double, double> cir2ser_QAM64_34_;


        std::string CIRProviderName_;
        std::string PHYModeProviderName_;

        struct Friends {
            FunctionalUnit* CIRProvider;
            FunctionalUnit* PHYModeProvider;
        } friends;
    };
}

#endif

