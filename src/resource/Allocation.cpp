cd/*******************************************************************************
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

#include <WNS/resource/Allocation.hpp>

#include <WNS/Assure.hpp>

using namespace wimac::resource;

Allocation::Allocation(const wns::SmartPtr<wimac::resource::Job> job):
    job_(job)
{
    unallocated_ = job_->length();
}

void
Allocation::allocate(int unitID, const Bit& size)
{
    //units_[unitID] += size;
    unallocated_ -= size;
}

Bit
Allocation::unallocated() const
{
    return unallocated_;
}
