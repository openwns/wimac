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

/**
 * \file
 * \author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#ifndef WIMAC_SERVICES_CONTROLPLANEMANAGERINTERFACE_HPP
#define WIMAC_SERVICES_CONTROLPLANEMANAGERINTERFACE_HPP

#include <WNS/pyconfig/View.hpp>
#include <WNS/ldk/ManagementServiceInterface.hpp>

#include <WIMAC/Logger.hpp>
#include <WIMAC/Component.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>
#include <WIMAC/services/Dissociating.hpp>
#include <WIMAC/services/Associating.hpp>

namespace wimac { namespace service {


/**
* @brief ControlPlaneManagerInterface
*
*/

class ControlPlaneManagerInterface
//	: public wns::ldk::ManagementService
{
public:
	typedef ConnectionIdentifier::StationID StationID;
	typedef ConnectionIdentifier::QoSCategory QoSCategory;


	virtual
	~ControlPlaneManagerInterface(){};

	virtual void start(StationID associateTo, QoSCategory QoSCategory) = 0;

	//virtual void onFUNCreated() = 0;
};


}} // service::wimac

#endif // WIMAC_SERVICES_CONTROLPLANEMANAGERINTERFACE_HPP


