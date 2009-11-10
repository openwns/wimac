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
#include <WIMAC/ConnectionIdentifier.hpp>

#include <iostream>
#include <sstream>

using namespace wimac;

ConnectionIdentifier::ConnectionIdentifier () :
    baseStation_(0),
    cid_(-1),
    subscriberStation_(0),
    remoteStation_(0),
    connectionType_(NoType),
    direction_(NoDirection),
    qos_(NoQoS),
    ciNotListening_(-1),
    valid_(false)
{
    commandKeyClasses_.connectionClassifier = NULL;
}


ConnectionIdentifier::ConnectionIdentifier(StationID baseStation,
        StationID subscriberStation,
        StationID remoteStation,
        ConnectionType connectionType,
        Direction direction,
        int qos) :
    baseStation_(baseStation),
    cid_(-1),
    subscriberStation_(subscriberStation),
    remoteStation_(remoteStation),
    connectionType_(connectionType),
    direction_(direction),
    qos_(qos),
    ciNotListening_(0),
    valid_(true)
{
    commandKeyClasses_.connectionClassifier = NULL;
}

ConnectionIdentifier::ConnectionIdentifier (StationID baseStation,
        CID cid,
        StationID subscriberStation,
        StationID remoteStation,
        ConnectionType connectionType,
        Direction direction,
        int qos):
    baseStation_(baseStation),
    cid_(cid),
    subscriberStation_(subscriberStation),
    remoteStation_(remoteStation),
    connectionType_(connectionType),
    direction_(direction),
    qos_(qos),
    ciNotListening_(0),
    valid_(true)
{
    commandKeyClasses_.connectionClassifier = NULL;
}

ConnectionIdentifier::ConnectionIdentifier( const ConnectionIdentifier& other ) :
    wns::CloneableInterface( other ),
    wns::RefCountable( other ),
    wns::Cloneable<ConnectionIdentifier>( other ),
    wns::IOutputStreamable( other ),
    baseStation_( other.baseStation_ ),
    cid_( other.cid_ ),
    subscriberStation_( other.subscriberStation_ ),
    remoteStation_( other.remoteStation_ ),
    connectionType_( other.connectionType_ ),
    direction_( other.direction_ ),
    qos_( other.qos_ ),
    ciNotListening_( other.ciNotListening_),
    valid_( other.valid_ )
{
    commandKeyClasses_.connectionClassifier = other.commandKeyClasses_.connectionClassifier;
}

std::string 
ConnectionIdentifier::doToString() const
{
    std::ostringstream log;
    log << " CID:" << cid_
        << "; Type:" << connectionType_
        << "," << direction_
        << "; QoSCategory:" << qos_
        << "; SS:" << subscriberStation_
        << "; BS:" << baseStation_;
    return log.str();
}

bool 
ConnectionIdentifier::integrityCheck() const
{

    if(   (connectionType_ == ConnectionIdentifier::InitialRanging)
        || (connectionType_ == ConnectionIdentifier::Basic)
        || (connectionType_ == ConnectionIdentifier::PrimaryManagement)
        || (connectionType_ == ConnectionIdentifier::SecondaryManagement) )
    {
        if(direction_ == ConnectionIdentifier::Bidirectional)
            if(qos_ == ConnectionIdentifier::Signaling)
                return true;

    } else if( connectionType_ == ConnectionIdentifier::Data )
    {
        if(   (direction_ == ConnectionIdentifier::Downlink)
            || (direction_ == ConnectionIdentifier::Uplink))
            if(   (qos_ == ConnectionIdentifier::UGS)
                || (qos_ == ConnectionIdentifier::rtPS)
                || (qos_ == ConnectionIdentifier::nrtPS)
                || (qos_ == ConnectionIdentifier::BE))
                return true;
    }

    return false;
}
