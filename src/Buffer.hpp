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


#ifndef WIMAC_BUFFER_HPP
#define WIMAC_BUFFER_HPP

#include <WNS/ldk/buffer/Dropping.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <WNS/Cloneable.hpp>


namespace wimac{

/********************* BufferDropping****************************************/
/**
* @brief BufferDropping is inherit from wns::ldk::buffer::dropping::Dropping
*        to add more Probes.
*
*/

	class BufferDropping :
		public wns::ldk::buffer::Dropping,
		public wns::Cloneable<BufferDropping>
	{
	public:
		BufferDropping(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config);
		BufferDropping(const BufferDropping& other);
		virtual ~BufferDropping();

		virtual wns::CloneableInterface* clone() const
			{
				return wns::Cloneable<BufferDropping>::clone();
			}


	private:
		//Probes putter
		wns::probe::bus::ContextCollectorPtr resetedBitsProbe_;
		wns::probe::bus::ContextCollectorPtr resetedCompoundsProbe_;
	};


}

#endif
