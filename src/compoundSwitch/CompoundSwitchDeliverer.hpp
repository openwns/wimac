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
	 * @brief
     * @author
	 *
	 *
	 */
	class CompoundSwitchDeliverer :
		public wns::ldk::Deliverer
	{
	public:
		typedef std::vector<wns::ldk::FunctionalUnit*> FunctionalUnitContainer;
		typedef std::list<FilterInterface*> Filters;

		~CompoundSwitchDeliverer();

		void addFilter(compoundSwitch::FilterInterface* filter);
		void onFUNCreated();
		FilterInterface* getFilter(const wns::ldk::CompoundPtr& compound) const;
		const Filters getAllFilter() const;

		virtual void add(wns::ldk::FunctionalUnit* fu);
		virtual void clear();
		virtual uint32_t size() const;
		virtual const Link::ExchangeContainer get() const;
		virtual void set(const Link::ExchangeContainer&);

		virtual wns::ldk::FunctionalUnit* getAcceptor(const wns::ldk::CompoundPtr& compound);

	private:
		FunctionalUnitContainer fus;
		Filters filters_;

	};
}}

#endif // NOT defined WIMAC_COMPOUNDSWITCH_COMPOUNDSWITCHDELIVERER_HPP


