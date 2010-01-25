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


#ifndef WIMAC_COMPOUNDSWITCH_FILTER_FILTERNONE_HPP
#define WIMAC_COMPOUNDSWITCH_FILTER_FILTERNONE_HPP

#include <WNS/pyconfig/View.hpp>
#include <WNS/logger/Logger.hpp>

#include <WIMAC/compoundSwitch/Filter.hpp>
#include <WIMAC/compoundSwitch/CompoundSwitch.hpp>

namespace wimac { namespace compoundSwitch { namespace filter {


            /**
             * @brief This filter match at any compound.
             * @author Markus Grauer <gra@comnets.rwth-aachen.de>
             *
             */
            class FilterNone :
                public Filter
            {
            public:
                FilterNone(CompoundSwitch* compoundSwitch, wns::pyconfig::View& config);

                ~FilterNone();

                virtual bool
                filter(const wns::ldk::CompoundPtr& compound) const;


            };

        }}}

#endif


