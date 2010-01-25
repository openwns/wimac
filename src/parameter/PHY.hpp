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

#ifndef WIMAC_PARAMETER_PHY_HPP
#define WIMAC_PARAMETER_PHY_HPP

#include <WNS/Singleton.hpp>
#include <WNS/pyconfig/View.hpp>

namespace wimac {
    class WiMAC;
}

namespace wimac { namespace parameter {
        /**
         * @brief Basic WiMAC PHY parameters.
         */
        class PHY
        {
        public:

            double getChannelBandwidth() const { return channelBandwidth; }
            double getCyclicPrefix() const { return cyclicPrefix;}
            double getSamplingFrequency() const { return samplingFrequency; }
            int getFFTSize() const { return fftSize;}
            double getSubCarrierSpacing() const { return subCarrierSpacing; }
            double getUsefulSymbolTime() const { return usefulSymbolTime; }
            double getGuardTime() const { return guardTime; }
            double getSymbolDuration() const { return symbolDuration; }
            double getFrameDuration() const { return frameDuration; }
            int getSymbolsPerFrame() const { return symbolsPerFrame; }
            double getTTGDuration() const { return ttg; }
            double getRTGDuration() const { return rtg; }
            int getGuardSubCarrier() const {return guardSubCarrier; }
            int getDCDubCarrier() const { return dcSubCarrier; }
            int getPilotSubCarrier() const { return pilotSubCarrier; }
            int getDataSubCarrier() const {return dataSubCarrier; }
            int getSubCahnnels() const { return subChannels; }
            int getSubCarrierPerSubChannel() const { return subCarrierPerSubChannel; }
            int getMinimumBitsPerSymbol() const { return minimumBitsPerSymbol; }

            int getDLPreamble() const { return dlPreamble; }
            int getFCH() const { return fch; }
            int getFrameHead() const { return frameHead; }

            int getMapBase() const { return mapBase; }
            int getIE() const { return ie; }

        private:
            void init(const wns::pyconfig::View& config);

            double channelBandwidth;
            double cyclicPrefix;
            double samplingFrequency;
            int fftSize;
            double subCarrierSpacing;
            double usefulSymbolTime;
            double guardTime;
            double symbolDuration;
            double frameDuration;
            int symbolsPerFrame;
            double ttg;
            double rtg;
            int guardSubCarrier;
            int dcSubCarrier;
            int pilotSubCarrier;
            int dataSubCarrier;
            int subChannels;
            int subCarrierPerSubChannel;
            int minimumBitsPerSymbol;
            int dlPreamble;
            int fch;
            int frameHead;
            int mapBase;
            int ie;

            friend class wimac::WiMAC;
        };

        typedef wns::SingletonHolder<PHY> ThePHY;
    }
}
#endif



