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
 * @author Karsten Klagges <kks@comnets.rwth-aachen.de>
 */
#ifndef WIMAC_CLASSIFIER_HPP
#define WIMAC_CLASSIFIER_HPP

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/CommandTypeSpecifier.hpp>
#include <WNS/ldk/HasReceptor.hpp>
#include <WNS/ldk/HasConnector.hpp>
#include <WNS/ldk/HasDeliverer.hpp>
#include <WNS/ldk/Processor.hpp>
#include <WNS/Cloneable.hpp>

#include <WNS/ldk/Classifier.hpp>


namespace wns {
    namespace ldk {

        class CommandPool;
    }
}

namespace wimac {
    class UpperConvergence;

    namespace service {
        class ConnectionManager;
    }

    class Component;

    /**
     * @brief The ConnectionClassifier classify compounds by using the
     * ConnectionManager.
     *
     * The ConnectionClassifier can't inherit from
     * wns::ldk::Classifier at the moment.
     *
     * \li The ClassificationPolicy use the ConnectionManager. => For
     * every compound, it calls the expensive getManagementService
     * function to get the ConnectionManager.
     *
     * \li We need two different classify methods for incomming and
     * outgoing compounds.
     */
    class ConnectionClassifier :
        public virtual wns::ldk::FunctionalUnit,
        public wns::ldk::CommandTypeSpecifier< wns::ldk::ClassifierCommand >,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<>,
        public wns::ldk::HasDeliverer<>,
        public wns::Cloneable< ConnectionClassifier >
    {
    public:
        ConnectionClassifier( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config );
        virtual void doOnData( const wns::ldk::CompoundPtr& compound);
        virtual void doSendData( const wns::ldk::CompoundPtr& compound);

        virtual
        wns::ldk::ClassificationID
        classifyIncoming( const wns::ldk::CompoundPtr& compound );

        virtual
        wns::ldk::ClassificationID
        classifyOutgoing( const wns::ldk::CompoundPtr& compound );

        virtual
        wns::ldk::CommandPool*
        createReply(const wns::ldk::CommandPool* original) const;

    private:
        virtual
        void
        doWakeup(){ getReceptor()->wakeup(); }

        virtual 
        bool
        doIsAccepting(const wns::ldk::CompoundPtr& compound) const;

        void
        onFUNCreated();

        struct {
            UpperConvergence* upperConvergence;
            service::ConnectionManager* connectionManager;
            Component* component;
        } friends_;

    };

    /**
     * @brief A Classifier mock, to get access to the ClassifierCommand.
     */
    class ClassifierMock :
        public ConnectionClassifier,
        public wns::Cloneable< ClassifierMock >
    {
    public:
        ClassifierMock( wns::ldk::fun::FUN* fun, const wns::pyconfig::View& config) :
            ConnectionClassifier(fun, config)
        {}

        virtual void
        doOnData( const wns::ldk::CompoundPtr&)
        {
            assure(0, "This is only a mock");
        }

        virtual void
        doSendData( const wns::ldk::CompoundPtr&)
        {
            assure(0, "This is only a mock");
        }

        virtual
        wns::ldk::ClassificationID
        classifyIncoming( const wns::ldk::CompoundPtr& )
        {
            assure(0, "This is only a mock");
            return wns::ldk::ClassificationID();
        }

        virtual
        wns::ldk::ClassificationID
        classifyOutgoing( const wns::ldk::CompoundPtr& )
        {
            assure(0, "This is only a mock");
            return wns::ldk::ClassificationID();
        }

        virtual
        wns::ldk::CommandPool*
        createReply(const wns::ldk::CommandPool*) const
        {
            assure(0, "This is only a mock");
            return NULL;
        }

        virtual
        wns::CloneableInterface*
        clone() const
        {
            return wns::Cloneable<ClassifierMock>::clone();
        }

    private:
        virtual 
        void
        doWakeup()
        {
            assure(0, "This is only a mock");
        }

        virtual
        bool
        doIsAccepting(const wns::ldk::CompoundPtr&) const
        {
            assure(0, "This is only a mock");
            return false;
        }

        void
        onFUNCreated(){}

        struct {
            UpperConvergence* upperConvergence;
            service::ConnectionManager* connectionManager;
            wimac::Component* layer;
        } friends_;
    };
}

#endif

