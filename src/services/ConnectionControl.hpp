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

#ifndef WIMAC_SERVICES_CONNECTIONCONTROL_H
#define WIMAC_SERVICES_CONNECTIONCONTROL_H

#include <WNS/ldk/ControlServiceInterface.hpp>
#include <WIMAC/ConnectionIdentifier.hpp>

namespace wimac { namespace relay {
	class RelayMapper;
}}

namespace wimac { namespace service {

	class ConnectionManager;
	class ConnectionControl :
		public wns::ldk::ControlService
	{
	public:
		ConnectionControl( wns::ldk::ControlServiceRegistry* csr,
						   wns::pyconfig::View& config );


		void
		createRecursiveConnection( ConnectionIdentifier::CID basicCID,
								   ConnectionIdentifier::CID primaryCID,
								   ConnectionIdentifier::CID downlinkTransportCID,
								   ConnectionIdentifier::CID uplinkTransportCID,
								   ConnectionIdentifier::StationID remote,
								   ConnectionIdentifier::QoSCategory qosCategory);
		void onCSRCreated();

		void associateTo( dll::Layer2::StationIDType destination,
						  ConnectionIdentifier::QoSCategory category);

	private:
		struct {
			wimac::service::ConnectionManager* connectionManager;
		} friends_;

		dll::Layer2::StationIDType associatedWithID_;
	};


}}

#endif


