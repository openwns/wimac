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
 * \file
 * \author Karsten Klagges <kks@comnets.rwth-aachen.de>
 */

#ifndef WIMAC_ACKSWITCH_HPP
#define WIMAC_ACKSWITCH_HPP

#include <WNS/ldk/ldk.hpp>

#include <WNS/Cloneable.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/Command.hpp>
#include <WNS/ldk/Classifier.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>

namespace wns { namespace ldk { namespace arq {
    class ARQ;
}}}

namespace wimac {

namespace service {
    class ConnectionManagerInterface;
}

    /**
     * \brief The Command for the ACKSwitch.
     */
    class AckSwitchCommand
            : public wns::ldk::Command
    {
    public:
        struct {
            ConnectionIdentifier::CID originalCID;
        } peer;

        struct {
        } local;

        struct {
        } magic;
    };

    /*
     * \brief Switches ARQ ACKs to the control connection of an associated SS.
     */
    class ACKSwitch :
            public wns::ldk::CommandTypeSpecifier<AckSwitchCommand>,
            public wns::ldk::HasConnector<>,
            public wns::ldk::HasReceptor<>,
            public wns::ldk::HasDeliverer<>,
            public wns::ldk::Processor<ACKSwitch>,
            public wns::Cloneable<ACKSwitch>
    {
    public:
        /**
         * \brief Constructor to create an ACKSwitch in a FUN.
         *
         * The ACKSwitch does not need any special configuration
         * options in the PyConfig.
         */
        ACKSwitch( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );

        /**
         * \brief Mandatory reimplementation from Processor.
         */
        void processIncoming( const wns::ldk::CompoundPtr& compound );

        /*
         * \brief Mandatory reimplementation from Processor.
         */
        void processOutgoing( const wns::ldk::CompoundPtr& compound );

        void onFUNCreated();

    private:
        // friends
        wns::ldk::arq::ARQ* arq_;
        wns::ldk::CommandTypeSpecifier< wns::ldk::ClassifierCommand >* classifier_;
        service::ConnectionManagerInterface* connectionManager_;
    };
}

#endif
