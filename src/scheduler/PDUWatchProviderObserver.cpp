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


#include <WIMAC/scheduler/PDUWatchProviderObserver.hpp>

#include <WNS/Assure.hpp>


using namespace  wimac::scheduler;


/************ PDUWatchProvider **********************/

PDUWatchProvider::PDUWatchProvider(wns::ldk::fun::FUN* fun) :
	fun_(fun),
	pduWatchObservers_(),
	logger_("WIMAC", "PDUWatchProvider")
{
} //PDUWatchProvider



PDUWatchProvider::~PDUWatchProvider()
{
	for(PDUWatchObservers::iterator iter = pduWatchObservers_.begin();
		iter != pduWatchObservers_.end(); ++iter)
	{
		(*iter)->setPDUWatchProvider(NULL);
	}
}



void
PDUWatchProvider::attachObserver(PDUWatchObserver* pduWatchObserver)
{
	assure(std::find(pduWatchObservers_.begin(),
					 pduWatchObservers_.end(),
					 pduWatchObserver)
	       == pduWatchObservers_.end(),
	       "PDUWatchObserver is already added to PDUWatchProvider");

	MESSAGE_BEGIN(NORMAL, logger_, m, "" );
	m << this << ": Attach observer! "
	  << pduWatchObserver;
	MESSAGE_END();

	pduWatchObservers_.push_back(pduWatchObserver);
	pduWatchObserver->setPDUWatchProvider(this);
} //attachPDUWatchObserver



void
PDUWatchProvider::detachObserver(PDUWatchObserver* pduWatchObserver)
{
	assure(std::find(pduWatchObservers_.begin(),
					 pduWatchObservers_.end(),
					 pduWatchObserver)
		   != pduWatchObservers_.end(),
		  "unknown PDUWatchObserver");

	MESSAGE_BEGIN(NORMAL, logger_, m, "" );
	m << this << ": Detach observer! "
	  << pduWatchObserver;
	MESSAGE_END();

	pduWatchObserver->setPDUWatchProvider(NULL);
	pduWatchObserver->pduWatchProviderDeleted();
	pduWatchObservers_.remove(pduWatchObserver);

} //detachPDUWatchObserver



void
PDUWatchProvider::pduWatch(const wns::ldk::CompoundPtr& compound)
{
	for(PDUWatchObservers::iterator iter = pduWatchObservers_.begin();
		iter != pduWatchObservers_.end();)
	{
		//Copy iterator and increase before oObserver detach from 
		//pduWatchObserverlist
		PDUWatchObserver* observer = *iter;
		++iter;

		if(fun_->getProxy()->commandIsActivated( compound->getCommandPool(),
												 observer ) )
		{
			MESSAGE_BEGIN(NORMAL, logger_, m, "" );
			m << fun_->getName() << ": notify observer: " << observer->getName();
			MESSAGE_END();

			observer->notifyPDUWatch( wns::ldk::CommandPool(
										   *compound->getCommandPool() ) );
		}
	}

} // pduWatch




/************ PDUWatchObserver **********************/

PDUWatchObserver::PDUWatchObserver()
	: wns::ldk::FunctionalUnit(),
	  pduWatchProvider_(NULL)
{
} // PDUWatchObserver



PDUWatchObserver::~PDUWatchObserver()
{
	if (pduWatchProvider_)
	{
		pduWatchProvider_->detachObserver(this);
		pduWatchProvider_ = NULL;
	}

} // ~PDUWatchObserver



void
PDUWatchObserver::setPDUWatchProvider(PDUWatchProvider* pduWatchProvider)
{
	pduWatchProvider_ = pduWatchProvider;
} // setPDUWatchProvider



void
PDUWatchObserver::pduWatchProviderDeleted()
{
} //pduWatchProviderDeleted
