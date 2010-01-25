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


#ifndef WIMAC_COMPOUNDSWITCH_COMPOUNDSWITCH_HPP
#define WIMAC_COMPOUNDSWITCH_COMPOUNDSWITCH_HPP

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Processor.hpp>

#include <WNS/Cloneable.hpp>
#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>

#include <WIMAC/compoundSwitch/CompoundSwitchConnector.hpp>
#include <WIMAC/compoundSwitch/CompoundSwitchDeliverer.hpp>


namespace wimac { namespace compoundSwitch {

    class FilterInterface;
    class Filter;

    /**
     * @brief Command for the FU CompoundSwitch and its filters
     *
     * @author Markus Grauer <gra@comnets.rwth-aachen.de>
     */
    class CompoundSwitchCommand :
        public wns::ldk::Command
    {
    public:
        CompoundSwitchCommand()
        {
            local.filterName = "";
        }

        struct {
            // Useed by filter::FilterByFilterName
            std::string filterName;
        } local;
        struct {} peer;
        struct {} magic;

    };


    /**
     * @brief This FU switch compounds by a filter to a specified FU connection.
     *
     * @author Markus Grauer <gra@comnets.rwth-aachen.de>
     *
     * The CompoundSwitch direct the compound to the first FU connection
     * which filter matches. These functionality is implemented for compound
     * processed by the Deliverer/onData and the Connector/sendData.
     *
     * Configuring:
     * The filter for the processed compounds are defined in the PyConfig.
     * There are lists for onData and sendData filters. All possible filters are
     * related to compoundSwitch::filter.
     * The order of the FU connections must require the order of the filters in
     * the filterlist to bind the filter to the right FU connection.
     *
     * @todo (gra): It should migrate to the LDK, after it is unit
     * tested and some more detailed documentation.
     */
    class CompoundSwitch :
        public virtual wns::ldk::FunctionalUnit,
        public wns::ldk::CommandTypeSpecifier<CompoundSwitchCommand>,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<CompoundSwitchConnector>,
        public wns::ldk::HasDeliverer<CompoundSwitchDeliverer>,
        public wns::Cloneable<CompoundSwitch>
    {
    public:
        CompoundSwitch(wns::ldk::fun::FUN* fuNet, const wns::pyconfig::View& config);

        ~CompoundSwitch();

        virtual void onFUNCreated();

        wns::ldk::FunctionalUnit*
        findFUNFriend( std::string friendName);

        wns::ldk::CommandProxy*
        getCommandProxy()
        {
            return getFUN()->getProxy();
        }


    private:

        typedef std::list<Filter*> Filters;

        virtual void
        doSendData(const wns::ldk::CompoundPtr& compound);

        virtual void
        doOnData(const wns::ldk::CompoundPtr& compound);

        virtual bool
        doIsAccepting(const wns::ldk::CompoundPtr& compound) const;

        virtual void
        doWakeup();

        /**
         * \brief Log output of association between Filter and Functional Unit
         */
        void
        printFilterAssociation();

        struct Friends {
        } friends_;

        wns::logger::Logger logger_;
		bool mustAccept_;
    };
}}
#endif


