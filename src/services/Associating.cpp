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

#include <WIMAC/services/Associating.hpp>

using namespace wimac::service;

Associating::Associating( wimac::Component* layer,
                          AssociatingCallBackInterface* const callBack,
                          const wns::pyconfig::View& config ) :
    RangingCallBackInterface(),
    MessageExchangerCallBackInterface(),
    SetupConnectionCallBackInterface(),
    state_(None),
    remainRetries_(-1),
    qosCategory_( ConnectionIdentifier::NoQoS ),
    layer_(layer),
    callBack_(callBack),
    retries_( config.get<int>("retries") ),
    friends_()
{
    friends_.rangingProviderName = config.get<std::string>("rangingProvider");
    friends_.regristrationProviderName = config.get<std::string>("regristrationProvider");
    friends_.setupConnectionProviderName = config.get<std::string>("setupConnectionProvider");

    friends_.rangingProvider = NULL;
    friends_.regristrationProvider = NULL;
    friends_.setupConnectionProvider = NULL;
}



void
Associating::start( handoverStrategy::Interface::Station targetBaseStation, int qosCategory)
{
    assure( state_ == None,
            "wimac::Associating::start: Object is in wrong state! \n" );

    LOG_INFO( layer_->getName(),
              ": calling Associating::start().");

    targetBaseStation_ = targetBaseStation;
    qosCategory_ = qosCategory;

    // Do next step
    remainRetries_ = retries_;
    this->doNextStep(Ranging);
}

void
Associating::resultRanging(bool result)
{
    assure( state_ == Ranging,
            "wimac::Associating::resultRanging: Object is in wrong state! \n" );

    LOG_INFO( layer_->getName(),
              ": calling Associating::resultRanging().");

    if(result == false)
    { // Ranging failed
        this->doNextStep(Ranging);
        return;
    }


    // Do next step
    remainRetries_ = retries_;
    this->doNextStep(Regristration);
}



void
Associating::resultMessageExchanger(std::string name, bool result)
{
    LOG_INFO( layer_->getName(),
              ": calling Associating::resultMessageExchanger()  Name: ", name);

    if(friends_.regristrationProviderName == name)
    {
        assure( state_ == Regristration,
                "wimac::Associating::resultMessageExchanger: Object is in wrong state! \n" );

        if(result == false)
        { // Regristration failed
            this->doNextStep(Regristration);
            return;
        }


        //Do next step
        remainRetries_ = retries_;
        this->doNextStep(SetupConnection);

    } else
    {
        assure(0,"Associating::resultMessageExchanger: MessageExchanger isn't known!\n");
    }
}



void
Associating::resultSetupConnection(bool result)
{
    assure( state_ == SetupConnection,
            "wimac::Associating::resultSetupConnection: Object is in wrong state! \n" );

    LOG_INFO( layer_->getName(),
              ": calling Associating::resultSetupConnection().");

    if(result == false)
    { // SetupConnection failed
        this->doNextStep(SetupConnection);
        return;
    }

    // Do next step
    remainRetries_ = -1;
    this->result(true);
}



void
Associating::onMSRCreated()
{
    friends_.rangingProvider = layer_->getFUN()
        ->findFriend<wimac::controlplane::RangingSS*>
        (friends_.rangingProviderName);

    friends_.regristrationProvider = layer_->getFUN()
        ->findFriend<wimac::controlplane::MessageExchanger*>
        (friends_.regristrationProviderName);

    friends_.setupConnectionProvider = layer_->getFUN()
        ->findFriend<wimac::controlplane::SetupConnectionSS*>
        (friends_.setupConnectionProviderName);
}


void
Associating::doNextStep(State state)
{
    if( remainRetries_ == 0 )
    {  // go to initinal state
        LOG_INFO(layer_->getName(), ": state failed ", retries_, " times.");

        remainRetries_ = -1;
        this->result(false);
        return;
    }
    remainRetries_--;


    if(state == Ranging)
    {
        state_ = Ranging;
        friends_.rangingProvider->start(targetBaseStation_.tune,
                                        targetBaseStation_.id,
                                        this);
        return;

    } else if(state == Regristration)
    {
        state_ = Regristration;
        friends_.regristrationProvider->start(this);
        return;

    } else if(state == SetupConnection)
    {
        state_ = SetupConnection;
        friends_.setupConnectionProvider->start(qosCategory_, this);
        return;

    } else if(state == None)
    {
        state_ = None;
        return;

    } else
    {
        assure(0,"wimac::service::Associating::doNextStep: State unknown!");
        return;
    }
}

void
Associating::result(bool result)
{
    double failure = 0.0;

    if(state_ == Ranging)
        failure = 0.1;
    if(state_ == Regristration)
        failure = 0.2;
    if(state_ == SetupConnection)
        failure = 0.3;

    this->doNextStep(None);
    callBack_->resultAssociating(result, failure);
}


