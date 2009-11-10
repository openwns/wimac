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

#ifndef WIMAC_SERVICES_SCANNINGSTRATEGY_SCANNINGSTRATEGY_HPP
#define WIMAC_SERVICES_SCANNINGSTRATEGY_SCANNINGSTRATEGY_HPP

#include <WIMAC/services/scanningStrategy/Interface.hpp>
#include <WIMAC/controlplane/Scanning.hpp>

#include <WNS/ldk/fcf/NewFrameProviderObserver.hpp>

namespace wimac {
    namespace service {
        namespace scanningStrategy{

            /**
             * @brief Scanning Strategy Basic Class
             * @author Markus Grauer <gra@comnets.rwth-aachen.de>
             */
            class ScanningStrategy:
                public Interface,
                public wimac::controlplane::ScanningCallBackInterface,
                public wns::ldk::fcf::NewFrameObserver
            {
            public:

                typedef ConnectionIdentifier::Frames Frames;
                typedef wimac::CIRMeasureInterface::MeasureValues MeasureValues;


                ScanningStrategy( VersusInterface* versusUnit,
                                  Component* layer,
                                  const wns::pyconfig::View& config );

                virtual
                ~ScanningStrategy();

                /**
                 * @brief scanningStrategy::Interface Implementation
                 */
                virtual void
                controlRSP() = 0;

                /**
                 * @brief scanningStrategy::Interface Implementation
                 */
                virtual void
                abortScanning() = 0;

                /**
                 * @brief scanningStrategy::Interface Implementation
                 */
                virtual void
                setup(const Stations stationsToScan);

                /**
                 * @brief scanningStrategy::Interface Implementation
                 */
                virtual void
                onFUNCreated();

                /**
                 * @brief ScanningCallBackInterface implementation
                 */
                virtual void
                resultScanning(const wimac::CIRMeasureInterface::MeasureValues& measureValuesOutput);

                /**
                 * @brief NewFrameObserver Interface
                 */
                virtual void
                messageNewFrame();


            protected:

                virtual void
                result(const MeasureValues& measureValues) = 0;

                void
                startScanning(Stations stationsToScan);

                /**
                 * @brief timer control function: start
                 */
                void
                timerStart(Frames time);

                /**
                 * @brief timer control function: stop
                 */
                void
                timerStop();

                /**
                 * @brief timer execute function
                 */
                virtual void
                timerExecute() = 0;

                Stations stationsToScan_;

                Component* layer_;
                VersusInterface* versusUnit_;

                int remainRetries_;
                Frames timer_;

                // Static values from PyConfig
                const int retries_;
                const Frames framesBetweenScanning_;

                struct {
                    std::string scanningProviderName;
                    std::string newFrameProviderName;

                    wimac::controlplane::ScanningSS* scanningProvider;
                    wns::ldk::fcf::NewFrameProvider* newFrameProvider;
                } friends_;
            };
        }
    }
}

#endif
