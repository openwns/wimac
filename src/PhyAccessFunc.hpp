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

#ifndef WIMAC_PHYACCESSFUNC_H
#define WIMAC_PHYACCESSFUNC_H

#include <WNS/ldk/Compound.hpp>
#include <WNS/service/phy/ofdma/Pattern.hpp>
#include <WNS/Cloneable.hpp>
#include <WNS/service/phy/phymode/PhyModeInterface.hpp>

namespace wns { namespace node {
    class Interface;
}}

namespace wimac {
    class PhyUser;

    /**
     * \defgroup phyAccessFunctors Physical Layers Access Functors
     * @{
     */

    /**
     * @brief The PhyAccessFunc provides an interface for accessing
     * the physical layer.
     *
     * The PhyAccessFunc is a base class for access to the physical
     * layer. Derive from this class and implement the operator()().
     */
    class PhyAccessFunc :
        public virtual wns::CloneableInterface
    {
    public:
        virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& compound) = 0;
        virtual ~PhyAccessFunc(){}
        PhyAccessFunc():
            transmissionStart_(-1.0),
            transmissionStop_(-1.0),
            subBand_(0),
            beam_(0),
            timeSlot_(-1),
            phyMode_()
        {}
        wns::simulator::Time transmissionStart_;
        wns::simulator::Time transmissionStop_;
        int subBand_;
        int beam_;
        int timeSlot_;
        wns::service::phy::phymode::PhyModeInterfacePtr phyMode_;
    };

    /**
     * @brief A transmission that starts and stops a broadcast
     * transmission.
     */
    class BroadcastPhyAccessFunc :
        public wimac::PhyAccessFunc,
        public wns::Cloneable<BroadcastPhyAccessFunc>
    {
    public:
        virtual void
        operator()( wimac::PhyUser* , const wns::ldk::CompoundPtr& );

    private:
    };

    /**
     * @brief A functor that starts and stops a unicast transmission.
     */
    class OmniUnicastPhyAccessFunc :
        public PhyAccessFunc,
        public wns::Cloneable<OmniUnicastPhyAccessFunc>
    {
    public:
        virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& );

        wns::node::Interface* destination_;
        wns::Power requestedTxPower_;
    };

    /**
     * @brief A functor that sets a beamforming pasttern and starts
     * and stops a transmission with the pattern.
     */
    class BeamformingPhyAccessFunc :
        public PhyAccessFunc,
        public wns::Cloneable<BeamformingPhyAccessFunc>
    {
    public:
        virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& );

        wns::node::Interface* destination_;
        wns::service::phy::ofdma::PatternPtr pattern_;
        wns::Power requestedTxPower_;
    };

    /**
     * @brief A functor that sets a beamforming pattern but does not
     * start or stop a transmission.
     */
    class PatternSetterPhyAccessFunc :
        public PhyAccessFunc,
        public wns::Cloneable<PatternSetterPhyAccessFunc>
    {
    public:
        virtual void operator()( PhyUser*, const wns::ldk::CompoundPtr& );

        wns::node::Interface* destination_;
        wns::simulator::Time patternStart_;
        wns::simulator::Time patternEnd_;
        wns::service::phy::ofdma::PatternPtr pattern_;
    };
    /* @} */

    /**
     * \defgroup Physical Layer Access Events
     * @{
     */

    /**
     * @brief An event to stop a transmission with the specified compound.
     */
    class StopTransmission
    {
    public:
        StopTransmission( wimac::PhyUser* phyUser, const wns::ldk::CompoundPtr& compound, int subBand = 0) :
            phyUser_( phyUser ),
            compound_( compound ),
            subBand_( subBand )
        {}

        void operator()();

    private:
        wimac::PhyUser* phyUser_;
        wns::ldk::CompoundPtr compound_;
        int subBand_;
    };
    /**
     * @brief An event to stop a broadcast transmission.
     */
    class StartBroadcastTransmission
    {
    public:
        StartBroadcastTransmission( wimac::PhyUser* phyUser,
                                    const wns::ldk::CompoundPtr& compound,
                                    wns::service::phy::phymode::PhyModeInterfacePtr phyMode,
                                    int subBand = 0):
            phyUser_( phyUser ),
            compound_( compound ),
            subBand_( subBand ),
            phyMode_( phyMode )
        {}

        void operator()();

    protected:
        wimac::PhyUser* phyUser_;
        wns::ldk::CompoundPtr compound_;
        int subBand_;
        const wns::service::phy::phymode::PhyModeInterfacePtr phyMode_;
    };

    /**
     * @brief An event to start a transmission with the given compound
     * to the destination station.
     */
    class StartTransmission
    {
    public:
        StartTransmission( wimac::PhyUser* phyUser,
                           const wns::ldk::CompoundPtr& compound,
                           wns::node::Interface* dstStation,
                           const wns::service::phy::phymode::PhyModeInterfacePtr phyMode,
                           int subBand,
                           wns::Power requestedTxPower) :
            phyUser_( phyUser ),
            compound_( compound ),
            dstStation_( dstStation ),
            subBand_( subBand ),
            phyMode_( phyMode ),
            requestedTxPower_( requestedTxPower )
        {}

        void operator()();

    protected:
        wimac::PhyUser* phyUser_;
        wns::ldk::CompoundPtr compound_;
        wns::node::Interface* dstStation_;
        const wns::service::phy::phymode::PhyModeInterfacePtr phyMode_;
        int subBand_;
        wns::Power requestedTxPower_;
    };

    /**
     * @brief An event to start a transmission with a given compound
     * and a given antenna pattern to the destination station.
     */
    class StartBeamformingTransmission :
        public StartTransmission
    {
    public:
        StartBeamformingTransmission( wimac::PhyUser* phyUser,
                                      const wns::ldk::CompoundPtr& compound,
                                      wns::node::Interface* dstStation,
                                      wns::service::phy::ofdma::PatternPtr pattern,
                                      int subBand,
                                      wns::Power requestedTxPower,
                                      wns::service::phy::phymode::PhyModeInterfacePtr phyMode) :
            StartTransmission( phyUser, compound, dstStation, phyMode, subBand,  requestedTxPower),
            pattern_(pattern)
        {}

        void operator()();

    protected:
        wns::service::phy::ofdma::PatternPtr pattern_;
    };

    /**
     * @brief An event to set an antenna pattern.
     */
    class SetPattern
    {
    public:
        SetPattern( wimac::PhyUser* phyUser,
                    wns::node::Interface* dstStation,
                    wns::service::phy::ofdma::PatternPtr pattern ) :
            phyUser_(phyUser),
            dstStation_(dstStation),
            pattern_(pattern)
        {}

        void operator()();

    protected:
        wimac::PhyUser* phyUser_;
        wns::node::Interface* dstStation_;
        wns::service::phy::ofdma::PatternPtr pattern_;
    };

    /**
     * @brief An event to remove the current antenna pattern for the
     * destination station.
     */
    class RemovePattern
    {
    public:
        RemovePattern( wimac::PhyUser* phyUser,
                       wns::node::Interface* dstStation) :
            phyUser_(phyUser),
            dstStation_(dstStation)
        {}

        void operator()();
    protected:
        wimac::PhyUser* phyUser_;
        wns::node::Interface* dstStation_;
    };
    /* @} */
}

#endif
