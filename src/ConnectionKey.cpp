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


#include <WIMAC/ConnectionKey.hpp>

#include <WIMAC/Classifier.hpp>
#include <WIMAC/services/ConnectionManager.hpp>
#include <WIMAC/Component.hpp>

#include <WNS/ldk/Classifier.hpp>
#include <WNS/ldk/Layer.hpp>

using namespace wimac;

STATIC_FACTORY_REGISTER_WITH_CREATOR(CIDKeyBuilder, wns::ldk::KeyBuilder,
									 "wimac.CIDKeyBuilder",
									 wns::ldk::FUNConfigCreator);

ConnectionKey::ConnectionKey
(
	const CIDKeyBuilder* builder,
	const wns::ldk::CompoundPtr compound,
	int
	)
{
	wns::ldk::CommandPool* commandPool = compound->getCommandPool();
	wns::ldk::ClassifierCommand* command =
		builder->friends.classifier_->getCommand(commandPool);
	id = command->peer.id;
}

std::string ConnectionKey::str() const
{
	std::stringstream ss;
	ss << "CID: " << id;
	return ss.str();
}

bool ConnectionKey::operator<( const wns::ldk::Key& other ) const
{
	assure( typeid( other ) == typeid( ConnectionKey ),
			"comparing different key types");
	const ConnectionKey* otherID = static_cast<const ConnectionKey*>(&other);
	return id < otherID->id;
}

void CIDKeyBuilder::onFUNCreated()
{
	friends.classifier_ =
		fun_->findFriend<ConnectionClassifier*>("classifier");
}

