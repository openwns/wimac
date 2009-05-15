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

#include <WIMAC/Buffer.hpp>

#include <WNS/ldk/Layer.hpp>
#include <WNS/simulator/Bit.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/utils.hpp>



using namespace wimac;


STATIC_FACTORY_REGISTER_WITH_CREATOR(wimac::BufferDropping,
									 wns::ldk::FunctionalUnit,
									 "wimac.BufferDropping",
									 wns::ldk::FUNConfigCreator);


/********************* BufferDropping ***********************************************/
/**
 * @brief BufferDropping is inherit from wns::ldk::buffer::Dropping
 *        to add more Probes.
 *
 */

BufferDropping::BufferDropping(wns::ldk::fun::FUN* fun,
							   const wns::pyconfig::View& config)
	: wns::ldk::buffer::Dropping(fun, config),
	  wns::Cloneable<BufferDropping>()

{
    wns::probe::bus::ContextProviderCollection& cpc =
        fun->getLayer()->getContextProviderCollection();

	resetedBitsProbe_
        = wns::probe::bus::collector( cpc,
                                      config, "resetedBitsProbeName" );
	resetedCompoundsProbe_
        = wns::probe::bus::collector( cpc,
                                      config, "resetedCompoundsProbeName" );
}



BufferDropping::BufferDropping(const BufferDropping& other)
	: wns::ldk::CompoundHandlerInterface(other),
	  wns::ldk::CommandTypeSpecifierInterface(other),
	  wns::ldk::HasReceptorInterface(other),
	  wns::ldk::HasConnectorInterface(other),
	  wns::ldk::HasDelivererInterface(other),
	  wns::CloneableInterface(other),
	  wns::IOutputStreamable(other),
	  wns::PythonicOutput(other),
	  wns::ldk::FunctionalUnit(other),
	  wns::ldk::DelayedInterface(other),
	  wns::ldk::buffer::Dropping(other),
	  wns::Cloneable<BufferDropping>(other),
	  resetedBitsProbe_(other.resetedBitsProbe_),
	  resetedCompoundsProbe_(other.resetedCompoundsProbe_)
{
}



BufferDropping::~BufferDropping()
{
       Bit resetedBits = 0;
	for(wns::ldk::buffer::dropping::ContainerType::const_iterator it = buffer.begin();
	    it != buffer.end(); it++)
	{
		resetedBits += (*it)->getLengthInBits();
	}
	resetedBitsProbe_->put(resetedBits);
	resetedCompoundsProbe_->put(buffer.size());
}
