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

#include <WIMAC/frame/DataCollector.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>
#include <WNS/pyconfig/View.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
	wimac::frame::DataCollector,
	wns::ldk::FunctionalUnit,
	"wimac.frame.DataCollector",
	wns::ldk::FUNConfigCreator );

using namespace wimac::frame;


DataCollector::DataCollector(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
	wns::ldk::fcf::CompoundCollector(config),
	wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>(fun)
{
	if (!config.isNone("txScheduler"))
	{
		std::string txSchedulerName = config.get<std::string>("txScheduler.__plugin__");
		wns::pyconfig::View txSchedulerConfig
			( config.getView("txScheduler") );
		wimac::scheduler::SchedulerCreator* txSchedulerCreator
			( wimac::scheduler::SchedulerFactory::creator(txSchedulerName) );
		txScheduler.reset(txSchedulerCreator->create(fun, txSchedulerConfig));
	}

	if (!config.isNone("rxScheduler"))
	{
		std::string rxSchedulerName = config.get<std::string>("rxScheduler.__plugin__");
		wns::pyconfig::View rxSchedulerConfig
			( config.getView("rxScheduler") );
		wimac::scheduler::SchedulerCreator* rxSchedulerCreator
			( wimac::scheduler::SchedulerFactory::creator(rxSchedulerName) );
		rxScheduler.reset(rxSchedulerCreator->create(fun, rxSchedulerConfig));
	}
}

DataCollector::DataCollector(const DataCollector& rhs) :
    wns::ldk::fcf::CompoundCollectorInterface(rhs),
    wns::ldk::CompoundHandlerInterface<FunctionalUnit>(rhs),
	wns::ldk::CommandTypeSpecifierInterface(rhs),
	wns::ldk::HasReceptorInterface(rhs),
	wns::ldk::HasConnectorInterface(rhs),
	wns::ldk::HasDelivererInterface(rhs),
	wns::CloneableInterface(rhs),
	wns::IOutputStreamable(rhs),
	wns::PythonicOutput(rhs),
	wns::ldk::FunctionalUnit(rhs),
	wns::ldk::fcf::CompoundCollector(rhs),
	wns::ldk::CommandTypeSpecifier<wns::ldk::EmptyCommand>(rhs),
	wns::ldk::HasConnector<wns::ldk::SingleConnector>(rhs),
	wns::ldk::HasReceptor<wns::ldk::SingleReceptor>(rhs),
	wns::ldk::HasDeliverer<wns::ldk::SingleDeliverer>(rhs),
	wns::Cloneable<wimac::frame::DataCollector>(rhs),
	wns::events::CanTimeout(rhs)
{
	txScheduler.reset(dynamic_cast<wimac::scheduler::SchedulerInterface*>
			  (dynamic_cast<wns::CloneableInterface*>
			   (rhs.txScheduler.get())->clone()));

	rxScheduler.reset(dynamic_cast<wimac::scheduler::SchedulerInterface*>
			  (dynamic_cast<wns::CloneableInterface*>
			   (rhs.rxScheduler.get())->clone()));
}

void
DataCollector::onFUNCreated()
{
	if (txScheduler.get())
	{
		txScheduler->setFUN(getFUN());
		txScheduler->setReceptor(getReceptor());
	}
	if (rxScheduler.get())
	{
		rxScheduler->setFUN(getFUN());
		rxScheduler->setReceptor(getReceptor());
	}
}

void
DataCollector::doOnData(const wns::ldk::CompoundPtr& compound)
{
	getDeliverer()->getAcceptor(compound)->onData(compound);
}

void
DataCollector::doSendData(const wns::ldk::CompoundPtr& compound)
{
	if ( rxScheduler.get() && rxScheduler->isAccepting(compound) )
		rxScheduler->schedule(compound);
	else if ( txScheduler.get() && txScheduler->isAccepting(compound) )
		txScheduler->schedule(compound);
	else
		throw wns::Exception("No scheduler accepts the compound as requested");
}

void
DataCollector::doStartCollection(int /*mode*/)
{
	getCurrentScheduler()->setDuration(getMaximumDuration());

	getCurrentScheduler()->startScheduling();
}

void
DataCollector::doStart(int /*mode*/)
{
	getCurrentScheduler()->deliverSchedule(getConnector());
	setTimeout( getMaximumDuration() );
}

simTimeType
DataCollector::getCurrentDuration() const
{
	return getCurrentScheduler()->getDuration();
}

bool
DataCollector::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
	return (txScheduler.get() && txScheduler->isAccepting(compound))
		|| ( rxScheduler.get() && rxScheduler->isAccepting(compound) );
}

void
DataCollector::onTimeout()
{
	getFrameBuilder()->finishedPhase( this );
}

void
DataCollector::finishCollection()
{
	getCurrentScheduler()->finishScheduling();
}

wimac::scheduler::SchedulerInterface*
DataCollector::getCurrentScheduler() const
{
	switch (getMode())
	{
	case Sending:
		return txScheduler.get();
	case Receiving:
		return rxScheduler.get();
	default:
		throw wns::Exception("Unknown activation mode in DataCollector");
	}
}


