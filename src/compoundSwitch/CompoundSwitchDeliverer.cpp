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

#include <WIMAC/compoundSwitch/CompoundSwitchDeliverer.hpp>

#include <WIMAC/compoundSwitch/Filter.hpp>

#include <algorithm>

using namespace wimac;
using namespace wimac::compoundSwitch;

CompoundSwitchDeliverer::~CompoundSwitchDeliverer()
{
	while ( !filters_.empty() )
	{
		delete *(filters_.begin());
		filters_.erase(filters_.begin());
	}
}

void
CompoundSwitchDeliverer::addFilter(compoundSwitch::FilterInterface* filter)
{
	// check for double entries
	for(Filters::iterator it = filters_.begin(); it != filters_.end(); ++it)
	{
		if ((*it)->getName() == filter->getName())
		{
			std::ostringstream errorLog;
			errorLog << "wns::ldk::compoundSwitch::CompoundSwitchDeliverer: Double entry found in UL or DL Filter list: " 
					 << filter->getName();
			assure( 0, errorLog.str() );
		}
	}

	// add Filter
	filters_.push_back( filter );
}



void
CompoundSwitchDeliverer::onFUNCreated()
{
	// Call onFUNCreated of every filter
	for(Filters::iterator it = filters_.begin();
		it != filters_.end(); ++it)
	{
		(*it)->onFUNCreated();
	}
}

FilterInterface*
CompoundSwitchDeliverer::getFilter(const wns::ldk::CompoundPtr& compound) const
{
	for(Filters::const_iterator it = filters_.begin();
		it != filters_.end(); ++it)
	{
		if( (*it)->filter(compound) )
		{
			return (*it);
			break;
		}
	}
	assure (0, "CompoundSwitchDeliverer::getFilter: No filter found for this compound. \n");
	return filters_.back();
} // getFilter



const CompoundSwitchDeliverer::Filters
CompoundSwitchDeliverer::getAllFilter() const
{
	Filters result;
	for(Filters::const_iterator it = filters_.begin();
		it != filters_.end(); ++it)
	{
		result.push_back(*it);
	}
	return result;
} // getAllFilter()



void
CompoundSwitchDeliverer::add(wns::ldk::FunctionalUnit* fu)
{
	fus.push_back(fu);
} // add


void
CompoundSwitchDeliverer::clear()
{
	fus.clear();
} // clear


uint32_t
CompoundSwitchDeliverer::size() const
{
	return fus.size();
} // size


const wns::ldk::Link::ExchangeContainer
CompoundSwitchDeliverer::get() const
{
	Link::ExchangeContainer result;

	for(Link::ExchangeContainer::const_iterator it = fus.begin();
		it != fus.end();
		++it) {
		result.push_back(*it);
	}

	return result;
} // get


void
CompoundSwitchDeliverer::set(const wns::ldk::Link::ExchangeContainer& src)
{
	fus.clear();

	for(Link::ExchangeContainer::const_iterator it = src.begin();
		it != src.end();
		++it) {
		fus.push_back(*it);
	}
} // set



wns::ldk::FunctionalUnit*
CompoundSwitchDeliverer::getAcceptor(const wns::ldk::CompoundPtr& compound)
{
	int i = 0;
	for(Filters::const_iterator it = filters_.begin();
		it != filters_.end(); ++it)
	{
		if( dynamic_cast<FilterInterface*>(*it)->filter(compound) )
		{
			return fus.at(i);
			break;
		}
		++i;
	}
	assure (0, "CompoundSwitchDeliverer::getAcceptor: No filter found for this compound. \n");
	return fus.back();
} // getAcceptor



