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


#ifndef WIMAC_ERRORMODELLINGTEST_HPP
#define WIMAC_ERRORMODELLINGTEST_HPP

#include <WNS/ldk/tools/Stub.hpp>
#include <WNS/ldk/tools/Forwarder.hpp>


#include <WIMAC/ErrorModelling.hpp>
#include <WNS/cppunit/extensions/HelperMacros.h>
#include <stdexcept>
#include <WNS/ldk/fun/Main.hpp>


namespace wimac{

    using namespace wns::ldk;

    class PHYModeProvider;

    /**
     * @todo the error modelling has changed, modify this test to make it fit to the changes
     */
    class ErrorModellingTest :
        public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(ErrorModellingTest);
        CPPUNIT_TEST(TestPassThrough);
        CPPUNIT_TEST(TestPHYModeBPSK12);
        CPPUNIT_TEST(TestPHYModeQPSK12);
        CPPUNIT_TEST(TestPHYModeQPSK34);
        CPPUNIT_TEST(TestPHYModeQAM16_12);
        CPPUNIT_TEST(TestPHYModeQAM16_34);
        CPPUNIT_TEST(TestPHYModeQAM64_23);
        CPPUNIT_TEST(TestPHYModeQAM64_34);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();
        void tearDown();

        // Tests
        void TestPassThrough();
        void TestPHYModeBPSK12();
        void TestPHYModeQPSK12();
        void TestPHYModeQPSK34();
        void TestPHYModeQAM16_12();
        void TestPHYModeQAM16_34();
        void TestPHYModeQAM64_23();
        void TestPHYModeQAM64_34();


    private:
        friend class ErrorModelling;

        wns::ldk::Layer* layer;
        wns::ldk::fun::Main* fun;
        wns::ldk::tools::Stub* upper;
        wimac::ErrorModelling* errormodelling;
        wimac::PHYModeProvider* phymodeprovider;
        wns::ldk::tools::Stub* cirprovider;
        wns::ldk::tools::Stub* lower;

        std::map<double, double> cir2ser_BPSK12_;
        std::map<double, double> cir2ser_QPSK12_;
        std::map<double, double> cir2ser_QPSK34_;
        std::map<double, double> cir2ser_QAM16_12_;
        std::map<double, double> cir2ser_QAM16_34_;
        std::map<double, double> cir2ser_QAM64_23_;
        std::map<double, double> cir2ser_QAM64_34_;

        void setUpUpper();
        void setUpPHYModeProvider();
        void setUpCirProvider();
        void setUpErrorModelling();
        void setUpLower();
        void ImportMappings();
        void CreateSendCompound(int PacketSize, wimac::PHYTools::PHYMode PHYMode, double cir);
        void PrintResults();
        double Calculate(int CompoundNr);
        void Test(int PacketSize, wimac::PHYTools::PHYMode PHYMode, int MapLength);

        int PacketSize_;
        int MapLength_;

    };

    class PHYModeProvider:
        public virtual wns::ldk::FunctionalUnit,
        public wns::ldk::CommandTypeSpecifier< wimac::timing2::TimingCommand >,
        public wns::ldk::HasReceptor<>,
        public wns::ldk::HasConnector<>,
        public wns::ldk::HasDeliverer<>,
        public wns::Cloneable<PHYModeProvider>

    {
    public:
        PHYModeProvider( wns::ldk::fun::FUN* fun, wns::pyconfig::View& )
            : CommandTypeSpecifier< wimac::timing2::TimingCommand >(fun)
        {}
        virtual ~PHYModeProvider() {}
        virtual void onFUNCreated() {}
        virtual bool doIsAccepting(const wns::ldk::CompoundPtr) { return false;}
        virtual void doOnData(wns::ldk::CompoundPtr) {}
        virtual void doSendData(wns::ldk::CompoundPtr) {}
        virtual void doWakeup() {}
    };

}

#endif
