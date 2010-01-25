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

#ifndef WIMAC_SERVICES_SCANNINGSTRATEGY_PLAIN_HPP
#define WIMAC_SERVICES_SCANNINGSTRATEGY_PLAIN_HPP


#include <WIMAC/services/scanningStrategy/ScanningStrategy.hpp>


namespace wimac {
    namespace service {
        namespace scanningStrategy{

            /**
             * @brief Plain Scanning Strategy
             * @author Markus Grauer <gra@comnets.rwth-aachen.de>
             */
            class Plain :
                public ScanningStrategy
            {
            public:
                Plain( VersusInterface* versusUnit,
                       Component* layer,
                       const wns::pyconfig::View& config);

                virtual
                ~Plain(){}

                /**
                 * @brief ScanningStrategy Interface implementation
                 */
                virtual void
                controlRSP();

                /**
                 * @brief ScanningStrategy Interface implementation
                 */
                virtual void
                abortScanning()
                {
                    assure(0, "Plain::abortScanning: This functionality is not supported!");
                }

                /**
                 * @brief scanningStrategy::Interface Implementation
                 */
                virtual void
                setup(const Stations stationsToScan);

            private:

                /**
                 * @brief ScanningStrategy Interface implementation
                 */
                virtual void
                result(const MeasureValues& measureValuesOutput);

                void
                timerExecute();

                void
                scan();

                // Static values from PyConfig
            };
        }
    }
}

#endif


