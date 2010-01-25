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

#include <WIMAC/services/Dissociating.hpp>

using namespace wimac::service;


Dissociating::Dissociating( wimac::Component* layer,
                            DissociatingCallBackInterface* callBack,
                            const wns::pyconfig::View& config ):
    HandoverCallBackInterface(),
    remainRetries_(-1),
    targetBaseStations_(),
    layer_(layer),
    callBack_(callBack),
    retries_( config.get<int>("retries") ),
    friends_()
{
    friends_.handoverProviderName = config.get<std::string>("handoverProvider");
    friends_.handoverProvider = NULL;
}

void
Dissociating::start(const handoverStrategy::Interface::Stations
                    targetBaseStations)
{
    targetBaseStations_ = targetBaseStations;

    remainRetries_ = retries_;

    this->dissociating();
}



void
Dissociating::resultHandover(handoverStrategy::Interface::Stations
                             ackBaseStations)
{
    LOG_INFO( layer_->getName(),
              ": calling Dissociating::resultHandover().");

    if(ackBaseStations.empty())
    { // Handover failed
        this->dissociating();
        return;
    }

    remainRetries_ = -1;
    callBack_->resultDissociating(ackBaseStations);
}



void
Dissociating::onMSRCreated()
{
    friends_.handoverProvider = layer_->getFUN()->findFriend<wimac::controlplane::HandoverSS*>
        (friends_.handoverProviderName);
}


void
Dissociating::dissociating()
{
    assure(remainRetries_ >= 0,
           "Dissociating::dissociating(): Retries must be activated!");

    if( remainRetries_ == 0 )
    {  // go to initinal state
        LOG_INFO(layer_->getName(), ": Dissociating failed ", retries_,
                 " times.");

        remainRetries_ = -1;
        callBack_->resultDissociating(
            handoverStrategy::Interface::Stations() );
        return;
    }

    remainRetries_--;

    friends_.handoverProvider->start(targetBaseStations_,this);
}
