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

#ifndef WIMAC_COMPOUNDSWITCH_FILTER_FILTERCOMMAND_HPP
#define WIMAC_COMPOUNDSWITCH_FILTER_FILTERCOMMAND_HPP

#include <WNS/pyconfig/View.hpp>

#include <WIMAC/compoundSwitch/Filter.hpp>

namespace wimac { namespace compoundSwitch { namespace filter {


   	/**
	 * @brief This filter matches to the activated command of a Functional Unit.
     * @author Markus Grauer <gra@comnets.rwth-aachen.de>
	 *
	 * In the Pyconfig the related Functional Unit, which the filter is matching
	 * to, is configured.
	 *
	 */
	class FilterCommand :
		public Filter
	{
	public:
		FilterCommand(CompoundSwitch* compoundSwitch, wns::pyconfig::View& config);

		~FilterCommand();

		virtual void
		onFUNCreated();

		virtual bool
		filter(const wns::ldk::CompoundPtr& compound) const;


	private:

		struct Friends {
			std::string commandProviderName;

			wns::ldk::FunctionalUnit* commandProvider;
		} friends_;

	};

}}}

#endif // NOT defined WNS_LDK_COMPOUNDSWITCH_FILTER_FILTERCOMMAND_HPP


