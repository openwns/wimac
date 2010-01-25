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
#pragma once

#include <WNS/pyconfig/View.hpp>

namespace wns { namespace ldk {
    class FunctionalUnit;
    }
}

namespace wimac {
    /**
     * @brief Creator implementation to be used with StaticFactory.
     *
     * Useful for constructors with a functional unit and
     * pyconfig::View parameter.
     *
     */
    template <typename T, typename KIND = T>
    struct FUConfigCreator :
            public FUConfigCreator<KIND, KIND>
    {
        virtual KIND* create(wns::ldk::FunctionalUnit* fu, const wns::pyconfig::View& config)
        {
            return new T(fu, config);
        }
    };

    template <typename KIND>
    struct FUConfigCreator<KIND, KIND>
    {
    public:
        virtual KIND* create(wns::ldk::FunctionalUnit*, const wns::pyconfig::View&) = 0;

        virtual ~FUConfigCreator()
        {}
    };

}
