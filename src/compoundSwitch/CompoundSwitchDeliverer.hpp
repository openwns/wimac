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


#ifndef WIMAC_COMPOUNDSWITCH_COMPOUNDSWITCHDELIVERER_HPP
#define WIMAC_COMPOUNDSWITCH_COMPOUNDSWITCHDELIVERER_HPP

#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Forwarding.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>


namespace wimac{ namespace compoundSwitch {

    class FilterInterface;

    /**
     * @brief The Deliverer of the CompoundSwitch functional unit.
     */
    class CompoundSwitchDeliverer :
        public wns::ldk::Deliverer
    {
    public:
        typedef std::vector<wns::ldk::IDelivererReceptacle*> FunctionalUnitContainer;
        typedef std::list<FilterInterface*> Filters;

        ~CompoundSwitchDeliverer();

        void addFilter(compoundSwitch::FilterInterface* filter);
        void onFUNCreated();
        FilterInterface* getFilter(const wns::ldk::CompoundPtr& compound) const;
        const Filters getAllFilter() const;

        virtual void add(wns::ldk::IDelivererReceptacle* fu);
        virtual void clear();
        virtual size_t size() const;
        virtual const wns::ldk::Link<wns::ldk::IDelivererReceptacle>::ExchangeContainer get() const;
        virtual void set(const wns::ldk::Link<wns::ldk::IDelivererReceptacle>::ExchangeContainer&);

        virtual wns::ldk::IDelivererReceptacle* getAcceptor(const wns::ldk::CompoundPtr& compound);

    private:
        FunctionalUnitContainer fus;
        Filters filters_;

    };
}}

/*namespace wns { namespace ldk {

    template<>
    HasDeliverer<wimac::compoundSwitch::CompoundSwitchDeliverer>::HasDeliverer();

    template<>
        HasDeliverer<wimac::compoundSwitch::CompoundSwitchDeliverer>::HasDeliverer(const HasDeliverer&);

    template<>
        HasDeliverer<wimac::compoundSwitch::CompoundSwitchDeliverer>::~HasDeliverer();

    template<>
    Deliverer*
        HasDeliverer<wimac::compoundSwitch::CompoundSwitchDeliverer>::getDeliverer() const;
}}
*/
#endif


