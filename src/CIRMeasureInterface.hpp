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


#ifndef WIMAC_CIRMEASUREINTERFACE_HPP
#define WIMAC_CIRMEASUREINTERFACE_HPP

#include <WNS/service/phy/ofdma/DataTransmission.hpp>

namespace wimac {
    class Component;

    class CIRMeasureInterface
    {
    public:

        struct MValue{
            MValue() :
                cir(wns::Ratio()),
                timeStamp(0),
                station(NULL),
                tune()
            {}

            bool operator<(MValue& other)
            {
                return cir < other.cir;
            }

            wns::Ratio cir;
            wns::simulator::Time timeStamp;
            wimac::Component* station;
            wns::service::phy::ofdma::Tune tune;
        };

        typedef std::list<MValue> MeasureValues;

        virtual void
        startMeasuring() = 0;

        virtual MeasureValues
        stopMeasuring() = 0;

        virtual void
        setRxFrequency(wns::service::phy::ofdma::Tune tune) = 0;

        virtual ~CIRMeasureInterface() {}
    };
}

#endif

