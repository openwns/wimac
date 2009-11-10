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


#include <WIMAC/compoundSwitch/CompoundSwitch.hpp>
#include <WIMAC/compoundSwitch/Filter.hpp>

#include <WNS/pyconfig/View.hpp>
#include <WNS/Assure.hpp>


using namespace wimac;
using namespace wimac::compoundSwitch;


STATIC_FACTORY_REGISTER_WITH_CREATOR(CompoundSwitch,
                                     wns::ldk::FunctionalUnit,
                                     "wimac.compoundSwitch.CompoundSwitch",
                                     wns::ldk::FUNConfigCreator);

CompoundSwitch::CompoundSwitch(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
    wns::ldk::CommandTypeSpecifier<CompoundSwitchCommand>(fun),
    wns::ldk::HasReceptor<>(),
    wns::ldk::HasConnector<CompoundSwitchConnector>(),
    wns::ldk::HasDeliverer<CompoundSwitchDeliverer>(),
    wns::Cloneable<CompoundSwitch>(),
    friends_(),
    logger_("WNS","CompoundSwitch")
{

    // configure all onData Filter
    int nOnDataFilter = config.len("onDataFilters");

    MESSAGE_BEGIN(NORMAL, logger_, m, "");
    m << getFUN()->getName()
      << ": configuring onDataFilters.   Number of onDataFilter:   " << nOnDataFilter;
    MESSAGE_END();

    for ( int i = 0; i < nOnDataFilter; ++i )
    {
        wns::pyconfig::View onDataFilterConfig( config, "onDataFilters", i );
        std::string pluginName
            = onDataFilterConfig.get<std::string>("__plugin__");

        Filter* onDataFilter( FilterFactory::creator(pluginName)
                              ->create( this, onDataFilterConfig ) );

        getDeliverer()->addFilter(onDataFilter);
    }


    // configure all sendData Filter
    int nSendDataFilter = config.len("sendDataFilters");

    MESSAGE_BEGIN(NORMAL, logger_, m, "");
    m << getFUN()->getName()
      << ": configuring sendDataFilters. Number of sendDataFilter: " << nSendDataFilter;
    MESSAGE_END();

    for ( int i = 0; i < nSendDataFilter; ++i )
    {
        wns::pyconfig::View sendDataFilterConfig( config, "sendDataFilters", i );
        std::string pluginName
            = sendDataFilterConfig.get<std::string>("__plugin__");

        Filter* sendDataFilter( FilterFactory::creator(pluginName)
                                ->create( this, sendDataFilterConfig ) );

        getConnector()->addFilter(sendDataFilter);
    }

}

CompoundSwitch::~CompoundSwitch()
{

}

void
CompoundSwitch::onFUNCreated()
{
    getDeliverer()->onFUNCreated();
    getConnector()->onFUNCreated(mustAccept_);

    printFilterAssociation();

}

wns::ldk::FunctionalUnit*
CompoundSwitch::findFUNFriend(std::string friendName)
{
    return getFUN()->findFriend<FunctionalUnit*>(friendName);
}

void
CompoundSwitch::doSendData(const wns::ldk::CompoundPtr& compound)
{
    MESSAGE_BEGIN(NORMAL, logger_, m, "");
    m << getFUN()->getName() << ": Outgoing compound catch by filter: "
        << getConnector()->getFilter(compound)->getName();
    MESSAGE_END();
    getConnector()->getAcceptor(compound)->sendData(compound);
}

void
CompoundSwitch::doOnData(const wns::ldk::CompoundPtr& compound)
{
    MESSAGE_BEGIN(NORMAL, logger_, m, "");
    m << getFUN()->getName() << ": Incoming compound catch by filter: "
        << getDeliverer()->getFilter(compound)->getName();
    MESSAGE_END();
    getDeliverer()->getAcceptor(compound)->onData(compound);
}

bool
CompoundSwitch::doIsAccepting(const wns::ldk::CompoundPtr& compound) const
{
    return getConnector()->hasAcceptor(compound);

}

void
CompoundSwitch::doWakeup()
{
    getReceptor()->wakeup();
}

void
CompoundSwitch::printFilterAssociation()
{
    // onData
    MESSAGE_BEGIN(NORMAL, logger_, m, "");
    m << getFUN()->getName()
      <<": onData association:   [ Filter, Functional Unit ]";
    MESSAGE_END();

    wns::ldk::Link<wns::ldk::IDelivererReceptacle>::ExchangeContainer fus = getDeliverer()->get();
    CompoundSwitchDeliverer::Filters filters = 
        getDeliverer()->getAllFilter();

    	if ( filters.size() != fus.size() ) {
		std::stringstream ss;
		ss << "Configuration Error" << std::endl
		   << "Number of onData Filters mismatch number of upper FUs in Compound Switch" << std::endl
		   << "Number of onData Filters: " << filters.size() << " Number of upper FUs: " << fus.size() << std::endl;
		throw wns::Exception(ss.str());
	}

    CompoundSwitchDeliverer::Filters::iterator itFilter = filters.begin();
    for(wns::ldk::Link<wns::ldk::IDelivererReceptacle>::ExchangeContainer::const_iterator itFU = fus.begin();
        itFU != fus.end();++itFU)
    {
        MESSAGE_BEGIN(NORMAL, logger_, m, "");
        m << getFUN()->getName() << "                        ( "
          << (*itFilter)->getName() << ", " <<(*itFU)->getFU()->getName() << " )";
        MESSAGE_END();
        ++itFilter;
    }

    //sendData
    MESSAGE_BEGIN(NORMAL, logger_, m, "");
    m << getFUN()->getName()
      << ": sendData association: [ Filter, Functional Unit ]";
    MESSAGE_END();

	wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer cons = getConnector()->get();
	filters = getConnector()->getAllFilter();

	if ( filters.size() != cons.size() ) {
		std::stringstream ss;
		ss << "Configuration Error" << std::endl
		   << "Number of sendData Filters mismatch number of lower FUs in Compound Switch" << std::endl
		   << "Number of sendData Filters: " << filters.size() << " Number of lower FUs: " << cons.size() << std::endl;
		throw wns::Exception(ss.str());
	}

    itFilter = filters.begin();
    for(wns::ldk::Link<wns::ldk::IConnectorReceptacle>::ExchangeContainer::const_iterator itFU = cons.begin();
        itFU != cons.end();++itFU)
    {
        MESSAGE_BEGIN(NORMAL, logger_, m, "");
        m << getFUN()->getName() << "                        ( "
          <<(*itFilter)->getName() << ", " <<(*itFU)->getFU()->getName() << " )";
        MESSAGE_END();
        ++itFilter;
    }

}




