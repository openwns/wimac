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


/**
 * @file
 * @author Markus Grauer <gra@comnets.rwth-aachen.de>
 */

#include <WIMAC/services/FUReseter.hpp>

#include <WNS/StaticFactory.hpp>
#include <WNS/service/dll/StationTypes.hpp>
#include <WNS/ldk/FUNConfigCreator.hpp>

#include <WIMAC/Component.hpp>

using namespace wimac::service;

STATIC_FACTORY_REGISTER_WITH_CREATOR( wimac::service::FUReseter,
                                      wns::ldk::ManagementServiceInterface,
                                      "wimac.services.FUReseter",
                                      wns::ldk::MSRConfigCreator );



FUReseter::FUReseter(wns::ldk::ManagementServiceRegistry* msr, const wns::pyconfig::View& config ) :
    ManagementService( msr ),
    fun_( dynamic_cast<Component*>( getMSR()->getLayer() )
          ->getFUN() )
{
    for ( int i = 0; i < config.len("flowSeparators"); ++i )
    {
        friends_.flowSeparatorNames.push_back(
            config.get<std::string>("flowSeparators", i ) );
    }

    for ( int i = 0; i < config.len("fusForReset"); ++i )
    {
        friends_.resetFUNames.push_back(
            config.get<std::string>("fusForReset", i ) );
    }
}



void
FUReseter::onMSRCreated()
{
    for(std::list<std::string>::const_iterator it = friends_.flowSeparatorNames.begin();
        it != friends_.flowSeparatorNames.end(); ++it)
    {
        LOG_INFO(fun_->getName(),": add  FlowSeparator:", *it);
        friends_.flowSeparators.push_back(
            fun_->findFriend<wns::ldk::FlowSeparator*>(*it) );
    }

    for(std::list<std::string>::const_iterator it = friends_.resetFUNames.begin();
        it != friends_.resetFUNames.end(); ++it)
    {
        LOG_INFO(fun_->getName(),": add FunctionalUnit:", *it);
        friends_.resetFUs.push_back(
            fun_->findFriend<FUResetInterface*>(*it) );
    }
}



void
FUReseter::resetFlowSeparator(wimac::ConnectionIdentifier::Ptr ci)
{
    wns::ldk::ConstKeyPtr cKey(new ConnectionKey(ci->cid_));

    LOG_INFO(fun_->getName(),
             ": try to reset Instances/Flows with Key: ",cKey->str());

    // store ProbeID QoSCategory of the Node
    assure(false, "fix above code if this is used");

    /// \warning Its a workaround to probe only the outgoing buffer size
    int stationType =
        dynamic_cast<Component*>(getMSR()->getLayer())
        ->getStationType();
    if(   (stationType == wns::service::dll::StationTypes::AP()
           && ci->direction_ == ConnectionIdentifier::Downlink)
          || (stationType != wns::service::dll::StationTypes::AP()
              && ci->direction_ == ConnectionIdentifier::Uplink) ) {
        // Only Probe outgoing Connections
        // set the QoSCategory of the CID which should reset
        //probeQoSCategoryIDProvider_->set(ci->qos_);
        assure(false, "fix above code if this is used");
    }
    else {
        // set the QoSCategory to NoQoS
        //probeQoSCategoryIDProvider_->set(0);
        assure(false, "fix above code if this is used");
    }

    // reset FlowSeparator for special CID
    for(std::list<wns::ldk::FlowSeparator*>::const_iterator it
            = friends_.flowSeparators.begin();
        it != friends_.flowSeparators.end(); ++it)
    {
        if( (*it)->getInstance(cKey) )
            (*it)->removeInstance( cKey );
    }

    // restore probe QoSCategory of the Node
    //probeQoSCategoryIDProvider_->set(oldQoSCategory);
    assure(false, "fix above code if this is used");
}



void
FUReseter::resetCID(wimac::ConnectionIdentifier::Ptr ci)
{
    LOG_INFO(fun_->getName(), ": try to reset FUs for CID: ", ci->cid_);

    // store ProbeID QoSCategory of the Node
    //uint32_t oldQoSCategory = probeQoSCategoryIDProvider_->get();
    assure(false, "fix above code if this is used");

    /// \warning Its a workaround to probe only the outgoing buffer size
    int stationType =
        dynamic_cast<Component*>(getMSR()->getLayer())
        ->getStationType();
    if(   (stationType == wns::service::dll::StationTypes::AP()
           && ci->direction_ == ConnectionIdentifier::Downlink)
          || (stationType != wns::service::dll::StationTypes::AP()
              && ci->direction_ == ConnectionIdentifier::Uplink) )
    { // Only Probe outgoing Connections
        // set the QoSCategory of the CID which should reset
        //probeQoSCategoryIDProvider_->set(ci->qos_);
        assure(false, "fix above code if this is used");

    }else
    {
        // set the QoSCategory to NoQoS
        //probeQoSCategoryIDProvider_->set(0);
        assure(false, "fix above code if this is used");
    }

    // reset FUResetInterface for special CID
    for(std::list<FUResetInterface*>::const_iterator it
            = friends_.resetFUs.begin();
        it != friends_.resetFUs.end(); ++it)
    {
        (*it)->resetCID( ci->cid_ );
    }

    // restore probe QoSCategory of the Node
    //probeQoSCategoryIDProvider_->set(oldQoSCategory);
    assure(false, "fix above code if this is used");
}



