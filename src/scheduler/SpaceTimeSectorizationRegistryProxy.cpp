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


#include <WIMAC/scheduler/SpaceTimeSectorizationRegistryProxy.hpp>
#include <WIMAC/scheduler/RegistryProxyWiMAC.hpp>

#include <WIMAC/PhyUser.hpp>

#include <cmath>

using namespace wimac;
using namespace wimac::scheduler;

STATIC_FACTORY_REGISTER_WITH_CREATOR(
    SpaceTimeSectorizationRegistryProxy,
    wns::scheduler::RegistryProxyInterface,
    "SpaceTimeSectorizationRegistryProxy",
    wns::ldk::FUNConfigCreator);

SpaceTimeSectorizationRegistryProxy::SpaceTimeSectorizationRegistryProxy(wns::ldk::fun::FUN* fun, const wns::pyconfig::View& pyConfig)
	: RegistryProxyWiMAC(fun, pyConfig),
	  ofdmaProvider(NULL),
	  logger(pyConfig.get("logger")),
	  numberOfSectors(pyConfig.get<int>("numberOfSectors")),
	  numberOfSubsectors(pyConfig.get<int>("numberOfSubsectors")),
	  mutualAngleOfSubsectors(-1)
	//mutualAngleBetweenSubsectors(pyConfig.get<double>("angelBetweenSubsectors"))
{

	assure(numberOfSectors >= 1,"invalid value of numberOfSectors, must be  >= 1");
	assure(numberOfSubsectors >= 1,"invalid value of numberOfSubsectors, must be  >= 1");
	//assure(angelBetweenSubsectors <= 2 * M_PI , "angelBetweenSubsectors exceeded pi");

}

wns::scheduler::UserSet
SpaceTimeSectorizationRegistryProxy::filterReachable( wns::scheduler::UserSet users )
{
	wns::scheduler::UserSet filteredUsers;
	filteredUsers.clear();

	wns::simulator::Time frameDuration = frameBuilder->getFrameDuration();
	wns::simulator::Time currentTime = wns::simulator::getEventScheduler()->getTime();

	//A 'group' corresponds to SSs residing in the same sector.
	//example: if 'numberOfSectors' equals 2, 'group' can be {0, 1}
	//with corresponding DoA {0...179, 180...359}
	int group;
	group = static_cast<int>(std::floor(currentTime / frameDuration)) % numberOfSectors;

	//MESSAGE_BEGIN(NORMAL, logger, m, "filterReachable ");
	//m << "Getting DoA estimates for the following users (current time: " << currentTime
	//  << " , frame: " << int(currentTime / frameDuration) << ", Group " << group << "): \n";
	//MESSAGE_END();

	for (wns::scheduler::UserSet::const_iterator iter = users.begin(); iter != users.end(); ++iter)
	{
		wns::scheduler::UserID user = *iter;

		double doa = ofdmaProvider->estimateDoA(user);
		if (doa < 0.0)
		{
			doa += 2.0 * M_PI;
		}
		assure(doa >= 0.0, "Invalid DoA");
		assure(doa <= 2.0 * M_PI, "Invalid DoA");

		MESSAGE_BEGIN(NORMAL, logger, m, "filterReachable at ");
		m << getNameForUser(getMyUserID());
		m << " - User " << getNameForUser(user) << " @ "
		  << doa * 180.0 / M_PI << " degrees\n";
		MESSAGE_END();

		if( isUserinActiveGroup(doa, group) )
		{
			MESSAGE_BEGIN(NORMAL, logger, m, "filterReachable at ");
			m << getNameForUser(getMyUserID()) << " - User belonging to group" << group <<": " << getNameForUser(user) << "\n";
			MESSAGE_END();

			filteredUsers.insert(user);
		}
	}

	return filteredUsers;
}

bool
SpaceTimeSectorizationRegistryProxy::isUserinActiveGroup(double doa, int group) const
{
	bool ret = false;
	if(numberOfSectors == 1)
	{
		ret = true;
	}
	else if ((numberOfSectors > 1) && (numberOfSubsectors == 1))
	{
		double minAngleOfSector = (2.0 * M_PI / numberOfSectors) * group;
		assure(minAngleOfSector <= (2.0 * M_PI), "lower sector edge exceeded 2 pi");

		const double sectorWidth = (2.0 * M_PI / numberOfSectors);
		assure((minAngleOfSector + sectorWidth) <= (2.0 * M_PI), "upper sector edge exceeded 2 pi");

		if((doa >= minAngleOfSector) && (doa < minAngleOfSector + sectorWidth))
		{
			ret = true;
		}
	}
	else if ((numberOfSectors > 1) && (numberOfSubsectors > 1))
	{
		double subsectorWidth = (2.0 * M_PI / (numberOfSectors * numberOfSubsectors));
		double minAngleOfSector = subsectorWidth * group;
		double angleBetweenSectors = subsectorWidth * numberOfSectors;

		assure(minAngleOfSector <= (2.0 * M_PI), "lower sector edge exceeds 2 pi");
		assure(minAngleOfSector + subsectorWidth <= (2.0 * M_PI), "upper sector edge exceeds 2 pi");
		assure(angleBetweenSectors <= (2.0 * M_PI), "angle between sectors exceeds 2 pi");

		double subsectorBegin = 0.0;
		double subsectorEnd = 0.0;
		for(int i = 0; i < numberOfSubsectors; i++)
		{
			subsectorBegin = minAngleOfSector + (i*angleBetweenSectors);
			subsectorEnd = subsectorBegin + subsectorWidth;
			if((doa >= subsectorBegin) && (doa < subsectorEnd))
			{
				ret = true;
			}
		}
	}
	return ret;
}

void
SpaceTimeSectorizationRegistryProxy::setFUN(const wns::ldk::fun::FUN* _fun) 
{
	fun = const_cast<wns::ldk::fun::FUN*>(_fun);
	assure(fun, "SpaceTimeSectorizationRegistryProxy needs a FUN");

	LOG_INFO("SpaceTimeSectorizationRegistryProxy::setFUN called in station ", fun->getName(), " ");

	wimac::PhyUser* phyUser = fun->findFriend<wimac::PhyUser*>("phyUser");
	assure(phyUser, "Could not get PhyUser");

	ofdmaProvider = phyUser->getDataTransmissionService();
	assure(ofdmaProvider, "Could not get DataTransmissionService");

	frameBuilder = fun->findFriend<wns::ldk::fcf::FrameBuilder*>("frameBuilder");


	// call the super class function
	RegistryProxyWiMAC::setFUN(fun);
}


