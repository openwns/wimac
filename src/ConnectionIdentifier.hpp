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

/**
 * \file
 * \author Karsten Klagges <kks@comnets.rwth-aachen.de>
 */
#ifndef WIMAC_CONNECTIONIDENTIFIER_HPP
#define WIMAC_CONNECTIONIDENTIFIER_HPP

#include <WNS/ldk/ldk.hpp>
#include <WNS/SmartPtr.hpp>
#include <WNS/PowerRatio.hpp>
#include <WNS/Cloneable.hpp>

#include <WIMAC/ConnectionRule.hpp>
#include <WIMAC/Logger.hpp>

#include <iostream>
#include <sstream>
#include <string>

namespace dll{
	class upperConvergence;
}



namespace wimac {
	class Component;
	class ConnectionClassifier;
	class ConnectionRule;

	namespace service {
		class ConnectionManager;
	}

	namespace tests {
		class TestConnectionManager;
	}
/// ConnectionIdentifier hold information for one connection.
/**
 * \sa ConnectionManager
 */

class ConnectionIdentifier :
		public wns::RefCountable,
		public wns::Cloneable<ConnectionIdentifier>,
		public wns::IOutputStreamable
{
public:
	typedef wns::SmartPtr<ConnectionIdentifier> Ptr;
	typedef std::list<Ptr> List;

	typedef int32_t StationID;
	static const int StationID_notValid = -1;

	typedef long CID;
	static const int CID_notValid = -1;

	/// The type of the connection.
	enum ConnectionType {
		NoType = 0,
		InitialRanging = 1,
		Basic = 2,
		PrimaryManagement = 3,
		SecondaryManagement = 4,
		Data = 5
	};

	enum Direction {
		NoDirection = 0x0,
		Downlink = 0x1,
		Uplink = 0x2,
		Bidirectional = 0x3 // Downlink | Uplink
	};

	enum QoSCategory {
		NoQoS = 0,
		Signaling = 1,
		UGS = 2,
		rtPS = 3,
		nrtPS = 4,
		BE = 5,
		MaxQoSCategory = BE
	};

	typedef int Frames;




	ConnectionIdentifier (StationID baseStation,
						  StationID subscriberStation,
						  StationID remoteStation,
						  ConnectionType connectionType,
						  Direction direction,
						  QoSCategory qos)
		: baseStation_(baseStation),
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



	ConnectionIdentifier (StationID baseStation,
						  CID cid,
						  StationID subscriberStation,
						  StationID remoteStation,
						  ConnectionType connectionType,
						  Direction direction,
						  QoSCategory qos)
		: baseStation_(baseStation),
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



	ConnectionIdentifier( const ConnectionIdentifier& other ) :
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




	CID getID() const { return cid_; }

	std::string doToString() const
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

	friend class ConnectionManager;


	bool operator==( const ConnectionIdentifier& rhs ) const
	{
		return ( baseStation_ == rhs.baseStation_ )
			&& ( cid_ == rhs.cid_ );
	}

	bool operator<( const ConnectionIdentifier& rhs ) const
	{
		if ( baseStation_ == rhs.baseStation_ )
			return cid_ < rhs.cid_;
		return baseStation_ < rhs.baseStation_;
	}

	bool integrityCheck() const
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


	///****** Stored  informations about connection  *************/

	///Primary keys for singularity of ConnectionIdentifier object
	ConnectionIdentifier::StationID baseStation_;
	CID cid_;


	ConnectionIdentifier::StationID subscriberStation_;
	ConnectionIdentifier::StationID remoteStation_;
	ConnectionType connectionType_;
	Direction direction_;
	QoSCategory qos_;

	///Simulator specific values, used by Scanning
	Frames ciNotListening_;

	/// commandKeyClasses to get right Command
	struct {
		wimac::ConnectionClassifier* connectionClassifier;
	} commandKeyClasses_;

    // Is this a valid ConnectionIdentifier or an empty one?
	bool valid_;

private:
	explicit ConnectionIdentifier () :
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

	friend class wimac::service::ConnectionManager;
	friend class wimac::tests::TestConnectionManager;
};


}
#endif
