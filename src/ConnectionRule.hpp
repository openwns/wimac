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

#ifndef WIMAC_CONNECTION_RULE
#define WIMAC_CONNECTION_RULE

#include <WNS/ldk/Compound.hpp>
#include <WNS/ldk/ldk.hpp>
#include <WNS/service/dll/Address.hpp>
#include <WNS/Cloneable.hpp>

namespace dll {
	class UpperConvergence;
}
namespace wimac {


	/// Base for all ConnectionRules that may be added to the ConnectionManager.
	class ConnectionRule :
		public virtual wns::CloneableInterface
	{
	public:
		virtual ~ConnectionRule() {}

		virtual bool match( const wns::ldk::CompoundPtr& compound ) = 0;
	};

	/// Rule to match with the specified destination address.
	class DestinationIPRule
		: public ConnectionRule,
		  public wns::Cloneable<DestinationIPRule>
	{
	public:
		DestinationIPRule( /*uint32_t*/wns::service::dll::UnicastAddress destination, dll::UpperConvergence* upperC  );
		virtual bool match( const wns::ldk::CompoundPtr& compound );
	private:
		wns::service::dll::UnicastAddress address_;
		dll::UpperConvergence* upperConvergence_;
	};

	/// Rule to match everything.
	class MatchAllRule
		: public ConnectionRule,
		  public wns::Cloneable<MatchAllRule>
	{
		virtual bool match( const wns::ldk::CompoundPtr& ) 
		{
			return true;
		}
	};
}

#endif

