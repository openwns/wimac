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

#ifndef WIMAC_PHYACCESSFUNC_H
#define WIMAC_PHYACCESSFUNC_H

#include <WNS/ldk/Compound.hpp>
#include <WNS/service/phy/ofdma/Pattern.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

namespace wns { namespace node {
	class Interface;
}}

namespace wimac {
	class PhyUser;

	class PhyAccessFunc :
		public virtual wns::CloneableInterface
	{
	public:
		virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& compound) = 0;
		virtual ~PhyAccessFunc(){}
		PhyAccessFunc():
			transmissionStart_(-1.0),
			transmissionStop_(-1.0),
			subBand_(0),
			phyMode_()
		{}
		double transmissionStart_;
		double transmissionStop_;
		int subBand_;
		wns::service::phy::phymode::PhyModeInterfacePtr phyMode_;
	};

	class BroadcastPhyAccessFunc :
		public wimac::PhyAccessFunc,
		public wns::Cloneable<BroadcastPhyAccessFunc>
	{
	public:
		virtual void
		operator()( wimac::PhyUser* , const wns::ldk::CompoundPtr& );

	private:
	};

	class OmniUnicastPhyAccessFunc :
		public PhyAccessFunc,
		public wns::Cloneable<OmniUnicastPhyAccessFunc>
	{
	public:
		virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& );

		wns::node::Interface* destination_;
	};

	class BeamformingPhyAccessFunc :
		public PhyAccessFunc,
		public wns::Cloneable<BeamformingPhyAccessFunc>
	{
	public:
		virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& );

		wns::node::Interface* destination_;
		wns::service::phy::ofdma::PatternPtr pattern_;
		wns::Power requestedTxPower_;
	};

	class PatternSetterPhyAccessFunc :
		public PhyAccessFunc,
		public wns::Cloneable<PatternSetterPhyAccessFunc>
	{
	public:
		virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& );

		wns::node::Interface* destination_;
		double patternStart_;
		double patternEnd_;
		wns::service::phy::ofdma::PatternPtr pattern_;
	};

	/******************* Events ********************/

	class StopTransmission
	{
	public:
		StopTransmission( wimac::PhyUser* phyUser, const wns::ldk::CompoundPtr& compound, int subBand = 0) :
			phyUser_( phyUser ),
			compound_( compound ),
			subBand_( subBand )
		{}

		void operator()();

	private:
		wimac::PhyUser* phyUser_;
		wns::ldk::CompoundPtr compound_;
		int subBand_;
	};

	class StartBroadcastTransmission
	{
	public:
		StartBroadcastTransmission( wimac::PhyUser* phyUser,
									const wns::ldk::CompoundPtr& compound,
									wns::service::phy::phymode::PhyModeInterfacePtr phyMode,
									int subBand = 0):
			phyUser_( phyUser ),
			compound_( compound ),
			subBand_( subBand ),
			phyMode_( phyMode )
		{}

		void operator()();

	protected:
		wimac::PhyUser* phyUser_;
		wns::ldk::CompoundPtr compound_;
		int subBand_;
		const wns::service::phy::phymode::PhyModeInterfacePtr phyMode_;
	};

	class StartTransmission
	{
	public:
		StartTransmission( wimac::PhyUser* phyUser,
						   const wns::ldk::CompoundPtr& compound,
						   wns::node::Interface* dstStation,
						   const wns::service::phy::phymode::PhyModeInterfacePtr phyMode,
						   int subBand = 0
						   ) :
			phyUser_( phyUser ),
			compound_( compound ),
			dstStation_( dstStation ),
			subBand_( subBand ),
			phyMode_( phyMode )
		{}

		void operator()();

	protected:
		wimac::PhyUser* phyUser_;
		wns::ldk::CompoundPtr compound_;
		wns::node::Interface* dstStation_;
		int subBand_;
		const wns::service::phy::phymode::PhyModeInterfacePtr phyMode_;
	};

	class StartBeamformingTransmission :
		public StartTransmission
	{
	public:
		StartBeamformingTransmission( wimac::PhyUser* phyUser,
									  const wns::ldk::CompoundPtr& compound,
									  wns::node::Interface* dstStation,
									  wns::service::phy::ofdma::PatternPtr pattern,
									  int subBand,
									  wns::Power requestedTxPower,
									  wns::service::phy::phymode::PhyModeInterfacePtr phyMode) :
			StartTransmission( phyUser, compound, dstStation, phyMode ),
			pattern_(pattern),
			subBand_(subBand),
			requestedTxPower_(requestedTxPower)
		{}

		void operator()();

	protected:
		wns::service::phy::ofdma::PatternPtr pattern_;
		int subBand_;
		wns::Power requestedTxPower_;
	};

	class SetPattern
	{
	public:
		SetPattern( wimac::PhyUser* phyUser,
					wns::node::Interface* dstStation,
					wns::service::phy::ofdma::PatternPtr pattern ) :
			phyUser_(phyUser),
			dstStation_(dstStation),
			pattern_(pattern)
		{}

		void operator()();

	protected:
		wimac::PhyUser* phyUser_;
		wns::node::Interface* dstStation_;
		wns::service::phy::ofdma::PatternPtr pattern_;
	};

	class RemovePattern
	{
	public:
		RemovePattern( wimac::PhyUser* phyUser,
					   wns::node::Interface* dstStation) :
			phyUser_(phyUser),
			dstStation_(dstStation)
		{}

		void operator()();
	protected:
		wimac::PhyUser* phyUser_;
		wns::node::Interface* dstStation_;
	};
}

#endif
