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


#ifndef WIMAC_CONNECTIONKEY_H
#define WIMAC_CONNECTIONKEY_H

#include <string>
#include <sstream>

#include <WNS/SmartPtr.hpp>
#include <WNS/ldk/Key.hpp>

namespace wns { namespace ldk {

    class Layer;
    class Compound;

}}
namespace wimac {

    class CIDKeyBuilder;
    class ConnectionClassifier;

    /**
     * @brief Key to separate flows in the layer.
     */
    class ConnectionKey :
        public wns::ldk::Key
    {
    public:
        ConnectionKey( const CIDKeyBuilder* builder, const wns::ldk::CompoundPtr compound,
                       int direction );

        // used by Handover.hpp
        ConnectionKey( int cid )
        {
            id = cid;
        }

        bool operator<(const wns::ldk::Key& other) const;
        std::string str() const;

    private:
        int id;
    };

    /**
     * @brief KeyBuilder for Flow Separators that use CIDs as keys.
     */
    class CIDKeyBuilder :
        public wns::ldk::KeyBuilder
    {
    public:
        CIDKeyBuilder( const wns::ldk::fun::FUN* fun, const wns::pyconfig::View& ) :
            fun_(fun) {}

        void onFUNCreated();

        virtual wns::ldk::ConstKeyPtr
        operator() (const wns::ldk::CompoundPtr& compound, int direction ) const
        {
            return wns::ldk::ConstKeyPtr(new ConnectionKey( this, compound, direction ));
        }

        const wns::ldk::fun::FUN* fun_;
        struct Friends {
            ConnectionClassifier* classifier_;
        } friends;
    };

}

#endif

