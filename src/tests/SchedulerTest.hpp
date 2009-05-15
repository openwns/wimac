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

#ifndef SCHEDULERTEST_HPP
#define SCHEDULERTEST_HPP


#include <WIMAC/scheduler/Scheduler.hpp>
#include <WIMAC/Classifier.hpp>
#include <WNS/ldk/FunctionalUnit.hpp>
#include <WNS/ldk/OldFlowSeparator.hpp>
#include <WNS/ldk/tools/Stub.hpp>
#include <WNS/ldk/Delayed.hpp>
#include <WNS/ldk/buffer/Dropping.hpp>
#include <WNS/pyconfig/Parser.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include <stdexcept>

namespace wimac{ namespace schedulertest {


 	class RLCCommand :
 		public wns::ldk::ClassifierCommand
 	{
 	public:
 		RLCCommand()
 		{
 			peer.id = 0;
 			magic.pduID = 0;
 		};

		struct {
 			int pduID;
 		} magic;
 	};

	//class FunctionalUnitRLC;


	class MyLayer :
		public wns::ldk::Layer
	{

	public:
//		FunctionalUnitRLC* rlc;
//		wimac::ConnectionClassifier classifier;
//		wns::ldk::FunctionalUnit* buffers;
		std::auto_ptr<ConnectionManager> connectionManager_;
		Scheduler<MyLayer, RLCCommand>* scheduler;

	};

// 	class FunctionalUnitRLC :
// 		public wns::ldk::CommandTypeSpecifier<RLCCommand, MyLayer>,
// 		public wns::ldk::HasReceptor<>,
// 		public wns::ldk::HasConnector<>,
// 		public wns::ldk::HasDeliverer<>,
// 		public wns::Cloneable<FunctionalUnitRLC>
// 	//public Delayed
// 	{

// 	public:
// 		FunctionalUnitRLC(MyLayer* layer /*, wns::pyconfig::View&*/) :
// 				wns::ldk::CommandTypeSpecifier<RLCCommand, MyLayer>(layer),
// 				wns::ldk::HasReceptor<>(),
// 				wns::ldk::HasConnector<>(),
// 				wns::ldk::HasDeliverer<>(),
// 				wns::Cloneable<FunctionalUnitRLC>()
// 		{}

// 		virtual bool doIsAccepting(const wns::ldk::CompoundPtr& ){return false;};
// 		virtual void doSendData( const wns::ldk::CompoundPtr& ) {};
// 		virtual void doOnData( const wns::ldk::CompoundPtr& ){};
// 		virtual void doWakeup() {};
// 	};

	class MyKey
	{
	public:
		MyKey(wns::ldk::Layer* _layer, const wns::ldk::CompoundPtr compound, int);
		bool operator<(const MyKey& other) const
		{
			return id < other.id;
		} // <

		std::string str()	// FIXME(fds) should be streaming
		{
			std::stringstream ss;
			ss << " id :" << id;
			return ss.str();
		}

		int id;
	};

	typedef wns::ldk::OldFlowSeparator<wns::ldk::buffer::Dropping, MyKey> Buffers;

//      template < class LayerType >
// 	class Scheduler;
//	class WiMAC;
// 	namespace rlc {
// 		class MyKey;
// 		class RLC;
// 		class RLCCommand;
// 	}

	class SchedulerTest : public CppUnit::TestFixture  {
		CPPUNIT_TEST_SUITE(SchedulerTest);
		CPPUNIT_TEST(testTransmitDuration);
		CPPUNIT_TEST(testEmptyFrame);
		CPPUNIT_TEST(testNumberOfBursts);
		CPPUNIT_TEST(testAllBurstsFull);
		CPPUNIT_TEST(testFullBurst);
		CPPUNIT_TEST_SUITE_END();
	public:
		void setUp();
		void tearDown();
		void testTransmitDuration();
		void testEmptyFrame();
		void testNumberOfBursts();
		void testAllBurstsFull();
		void testFullBurst();
	private:
		static const uint32 burstSize;
		static const uint32 noOfBursts;
		static const double PDUtransmitDuration;

		std::auto_ptr<MyLayer> layer;
		std::auto_ptr<wns::ldk::tools::Stub> upper;
		std::auto_ptr<Buffers> mybuffers;
		std::auto_ptr< Scheduler<MyLayer, RLCCommand > > myScheduler;
		std::auto_ptr<wns::ldk::tools::Stub> lower;
		std::auto_ptr<ConnectionClassifier> myClassifier;
	};
}}
#endif //NOT defined SchedulerTest_HPP


