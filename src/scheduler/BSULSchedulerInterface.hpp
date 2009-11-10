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


#ifndef WIMAC_SCHEDULER_BSULSCHEDULERINTERFACE_HPP
#define WIMAC_SCHEDULER_BSULSCHEDULERINTERFACE_HPP

#include <WNS/ldk/Compound.hpp>

namespace wimac {
  namespace scheduler {
    ///pseodo data interface for outgoing UL fake pdus in BS or RS
    class BSULSchedulerInterface {
    public:
        virtual ~BSULSchedulerInterface() {};
      virtual void sendPseudoData(const wns::ldk::CompoundPtr& compound) = 0;
      virtual bool isAcceptingPseudoData(const wns::ldk::CompoundPtr& compound) const = 0;
      virtual void resetAllQueues() = 0;
    };
}}
#endif
