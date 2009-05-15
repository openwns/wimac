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

#include <WIMAC/tests/SchedulerTest.hpp>
#include <WIMAC/scheduler/Scheduler.hpp>

#include <WNS/ldk/fun/FUN.hpp>
#include <WNS/ldk/Classifier.hpp>
#include <WNS/Assure.hpp>
#include <vector>
#include <math.h>

using namespace wimac::schedulertest;
//using namespace wimac;

CPPUNIT_TEST_SUITE_REGISTRATION( SchedulerTest );
//- What do we mean with equal burstSize considering different phyModes (time or bytes)? ->switchable
//- Does the scheduler get or set the phyModes? ->get for instance from RLC
//- Do compounds have the same byteSize at this stage? ->yes

//const uint32 SchedulerTest::burstSize = 2;
//const uint32 SchedulerTest::noOfBursts = 3;
const double SchedulerTest::PDUtransmitDuration = 0.00008333333;
const double SchedulerTest::FrameDuration = 0.01;
MyKey::MyKey(wns::ldk::Layer* _layer, const wns::ldk::CompoundPtr compound, int/*not used*/)
{
	MyLayer* layer = dynamic_cast<MyLayer*>(_layer);
	assure(layer != NULL, " dynymic cast failed");
	wns::ldk::CommandPool* commandPool = compound->getCommandPool();
	//Buffers* mybuffers = dynamic_cast<Buffers*>(layer->buffers_);

	wns::ldk::fun::FUN* fun = layer->getFUN();
	ConnectionClassifier* clf = fun->findFriend<wimac::ConnectionClassifier*>("classifier");
	wns::ldk::ClassifierCommand* command = clf->getCommand( commandPool );

	id = command->peer.id;
} // MyKey

void SchedulerTest::setUp()
{

	std::stringstream ss;
	//Buffer\n
	ss<<"from openwns.Buffer import Dropping\n"
	  <<"buffer = Dropping(size = 4,\n"
	  <<"  sizeUnit = 'PDU',\n"
	  <<"  drop = 'Tail'\n"
	  <<")\n"
		//Scheduler\n
	  <<"transmitDuration = " << PDUtransmitDuration << "\n"
	  <<"burstSize = " << burstSize << "\n"
	  <<"sizeUnit = 'PDU'\n"
	  << "noOfBursts = " << noOfBursts << "\n";

	wns::pyconfig::Parser tmpConfig;
	tmpConfig.loadString(ss.str());

	layer.reset( new MyLayer );

	wns::pyconfig::Parser emptyConfig;
	upper.reset( new wns::ldk::tools::Stub( layer.get(), emptyConfig) );

	wns::ldk::fun::FUN fun = layer.getFUN();

	myClassifier.reset( new FunctionalUnitRLC( layer.get() ) );
	fun->addFunctionalUnit( "scheduler", scheduler );

	wns::pyconfig::View bufferConfig(tmpConfig, "buffer");
	myBuffers.reset( new Buffers( layer.get(), bufferConfig ) );

	myScheduler.reset( new Scheduler<MyLayer, RLCCommand>(layer.get(), tmpConfig) );
	fun->addFunctionalUnit( "scheduler", myScheduler.get() );

	lower.reset( new wns::ldk::tools::Stub( layer.get(), emptyConfig ) );

	ConnectionClassifier* classifier = new ConnectionClassifier( this );
	fun->addFunctionalUnit( "classifier", classifier );

	wns::pyconfig::View connectionManagerConfig = wns::pyconfig::View( config, "connectionManager" );
	connectionManager.reset( new ConnectionManager( this, connectionManagerConfig ) );
	fun->addFunctionalUnit( "classifier", classifier );

	fun->onFUNCreated();

	upper
		->connect( myBuffers.get() )
		->connect( myScheduler.get() )
		->connect( lower.get() );

	wns::simulator::getEventScheduler()->reset();
} // setUp

void SchedulerTest::tearDown()
{
// 	delete upper;
// 	upper = 0;
//	delete myBuffers;
// 	myBuffers = 0;

// 	delete myScheduler;
// 	myScheduler = 0;

// 	delete lower;
// 	lower = 0;

// 	delete layer;
// 	layer = 0;

} // tearDown

//The scheduler fills bursts of equal size with compounds.
//A burst is filled up with compounds of the same CID.
void SchedulerTest::testTransmitDuration()
{
	std::vector<wns::ldk::CompoundPtr> compound(1);
	std::vector<RLCCommand*> commandPool(2);

	compound[0] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[0] = myClassifier->activateCommand(compound[0]->getCommand());
	commandPool[0]->peer.id = 23;
	commandPool[0]->magic.pduID = 0;

	myScheduler->startPeriodicTimeout();
	upper->sendData(wns::IncRef(compound[0]));

	uint eventIterations = 10;

	std::vector<double> sample(0);

	for(uint i = 0; i < eventIterations; i++){
		wns::simulator::getEventScheduler()->processOneEvent();
		if(lower->sent.size() > sample.size()) {
			sample.push_back(wns::simulator::getEventScheduler()->getTime());
		}
		if(i == eventIterations - 1)
			myScheduler->cancelPeriodicTimeout();
	}
	//std::cout<<" calculatedSlotDuration: "<<sample[1] - sample[0]<<std::endl;
	CPPUNIT_ASSERT( fabs(PDUtransmitDuration-(sample[3] - sample[2])) < (0.001 * PDUtransmitDuration));
		//std::cout<<" calculatedFrameDuration: "<<sample[2] - sample[0]<<"/"<<frameDuration<<std::endl;
	CPPUNIT_ASSERT( fabs(frameDuration-(sample[3] - sample[0])) < (0.001 * frameDuration));
}

void SchedulerTest::testEmptyFrame()
{
	//case sizeType = 'PDU'
	bool assert = false;
	uint eventIterations = 10;
	myScheduler->startPeriodicTimeout();

	eventIterations = 10;
	for(uint i = 0; i < eventIterations; i++){
		wns::simulator::getEventScheduler()->processOneEvent();

		double currentTime = wns::simulator::getEventScheduler()->getTime();
		if ((fabs(floor(currentTime/frameDuration)-currentTime/frameDuration) <= 0.001 * PDUtransmitDuration) && !(currentTime <= 0.001 * PDUtransmitDuration)) {//FIXME
			assert = true;
			//std::cout<<"currentTime: "<<currentTime <<"\ lower->sent.size(): "<<lower->sent.size()<<std::endl;
			CPPUNIT_ASSERT((currentTime - lower->sent.size() * frameDuration) <= 0.001 * PDUtransmitDuration);
		}
		if(i == eventIterations - 1)
			myScheduler->cancelPeriodicTimeout();
	}
CPPUNIT_ASSERT(assert);
}

void SchedulerTest::testNumberOfBursts()
{

	std::vector<wns::ldk::CompoundPtr> compound(5);
	std::vector<RLCCommand*> commandPool(5);

	compound[0] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[0] = myClassifier->activateCommand(compound[0]->getCommand());
	commandPool[0]->peer.id = 23;
	commandPool[0]->magic.pduID = 0;

	compound[1] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[1] = myClassifier->activateCommand(compound[1]->getCommand());
	commandPool[1]->peer.id = 42;
	commandPool[1]->magic.pduID = 1;
	//maybe we use NS numbers from arqCommand instead as this ID has nothing to do with RLC

	compound[2] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[2] = myClassifier->activateCommand(compound[2]->getCommand());
	commandPool[2]->peer.id = 43;
	commandPool[2]->magic.pduID = 2;

	int noOfBursts = 3;
	burstSize = 1;

	myScheduler->startPeriodicTimeout();
	upper->sendData(wns::IncRef(compound[0]));
 	upper->sendData(wns::IncRef(compound[1]));
	upper->sendData(wns::IncRef(compound[2]));

	uint eventIterations = 10;

	for(uint i = 0; i < eventIterations; i++){
		wns::simulator::getEventScheduler()->processOneEvent();
		if(lower->sent.size() == 1){
			compound[0] = lower->sent[0];
			FCHCommand* fchCommand = myScheduler->getCommand(compound[0]->getCommand());
			//equals to da numbers of IEs or the bursts respectivly?
			//CPPUNIT_ASSERT(fchCommand->IEs.size() == noOfBursts);
			for(uint i = 0; i < noOfBursts; i++) {
				//std::cout<<"fchCommand->IEs[i].lengt"<<fchCommand->IEs[i].length<<std::endl;
				CPPUNIT_ASSERT(fchCommand->peer.IEs[i].length == burstSize);
			}
			myScheduler->cancelPeriodicTimeout();
			break;
		}
	}
}

void SchedulerTest::testAllBurstsFull()
{
	std::vector<wns::ldk::CompoundPtr> compound(5);
	std::vector<RLCCommand*> commandPool(5);

	compound[0] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[0] = myClassifier->activateCommand(compound[0]->getCommand());
	commandPool[0]->peer.id = 23;
	commandPool[0]->magic.pduID = 0;

	compound[1] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[1] = myClassifier->activateCommand(compound[1]->getCommand());
	commandPool[1]->peer.id = 42;
	commandPool[1]->magic.pduID = 1;
	//maybe we use NS numbers from arqCommand instead as this ID has nothing to do with RLC

	compound[2] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[2] = myClassifier->activateCommand(compound[2]->getCommand());
	commandPool[2]->peer.id = 43;
	commandPool[2]->magic.pduID = 2;

	compound[3] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[3] = myClassifier->activateCommand(compound[3]->getCommand());
	commandPool[3]->peer.id = 44;
	commandPool[3]->magic.pduID = 3;

	myScheduler->startPeriodicTimeout();
	upper->sendData(wns::IncRef(compound[0]));
 	upper->sendData(wns::IncRef(compound[1]));
	upper->sendData(wns::IncRef(compound[2]));
 	upper->sendData(wns::IncRef(compound[3]));

	uint const eventIterations = 20;
	bool assert1 = false;
	bool assert2 = false;

	double frameDuration = PDUtransmitDuration *( burstSize * noOfBursts + 1);

	for(uint i = 0; i < eventIterations; i++){
		wns::simulator::getEventScheduler()->processOneEvent();
		double currentTime = wns::simulator::getEventScheduler()->getTime();

		if ((fabs(currentTime - frameDuration) <= 0.0001 * PDUtransmitDuration) && !assert1) {//FIXME
			assert1 = true;
			//std::cout<<"first lower->sent.size()/currentTime: "<<lower->sent.size()<<"/"<<currentTime<<std::endl;
			CPPUNIT_ASSERT(lower->sent.size() == 4);
		}
		if ((fabs(currentTime -  2 * frameDuration) <= 0.0001 * PDUtransmitDuration) && !assert2){//FIXME
			assert2 = true;
			//std::cout<<"secind lower->sent.size(): "<<lower->sent.size()<<std::endl;
			CPPUNIT_ASSERT(lower->sent.size() == 6);
			myScheduler->cancelPeriodicTimeout();
			break;
		}
	}
	assure(assert1 && assert2, "Assert not executedt");
	compound[0] = lower->sent[5];
	commandPool[0] = myClassifier->getCommand(compound[0]->getCommand());
	CPPUNIT_ASSERT(commandPool[0]->magic.pduID == 3);//
}


void SchedulerTest::testFullBurst()
{
	std::vector<wns::ldk::CompoundPtr> compound(5);
	std::vector<RLCCommand*> commandPool(5);

	compound[0] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[0] = myClassifier->activateCommand(compound[0]->getCommand());
	commandPool[0]->peer.id = 22;
	commandPool[0]->magic.pduID = 0;

	compound[1] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[1] = myClassifier->activateCommand(compound[1]->getCommand());
	commandPool[1]->peer.id = 21;
	commandPool[1]->magic.pduID = 1;
	//maybe we use NS numbers from arqCommand instead as this ID has nothing to do with RLC

	compound[2] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[2] = myClassifier->activateCommand(compound[2]->getCommand());
	commandPool[2]->peer.id = 23;
	commandPool[2]->magic.pduID = 2;

	compound[3] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[3] = myClassifier->activateCommand(compound[3]->getCommand());
	commandPool[3]->peer.id = 23;
	commandPool[3]->magic.pduID = 3;

	compound[4] = wns::IncRef(new wns::ldk::Compound(layer->getProxy()->createCommandPool()));
	commandPool[4] = myClassifier->activateCommand(compound[4]->getCommand());
	commandPool[4]->peer.id = 23;
	commandPool[4]->magic.pduID = 4;

	myScheduler->startPeriodicTimeout();
	upper->sendData(wns::IncRef(compound[0]));
 	upper->sendData(wns::IncRef(compound[1]));
	upper->sendData(wns::IncRef(compound[2]));
 	upper->sendData(wns::IncRef(compound[3]));
	upper->sendData(wns::IncRef(compound[4]));

	uint const eventIterations = 20;
	bool assert1 = false;
	bool assert2 = false;
	double frameDuration = PDUtransmitDuration * (burstSize * noOfBursts + 1);

	for(uint i = 0; i < eventIterations; i++){
		wns::simulator::getEventScheduler()->processOneEvent();
		double currentTime = wns::simulator::getEventScheduler()->getTime();
		if ((fabs(currentTime - frameDuration) <= 0.0001 * PDUtransmitDuration)&& !assert1) {//FIXME
			assert1 = true;
			//std::cout<<"first lower->sent.size()/currentTime: "<<lower->sent.size()<<"/"<<currentTime<<std::endl;
			CPPUNIT_ASSERT(lower->sent.size() == 5);
		}
		if ((fabs(currentTime -  2 * frameDuration) <= 0.0001 * PDUtransmitDuration) && !assert2){//FIXME
			assert2 = true;
			//std::cout<<"secind lower->sent.size(): "<<lower->sent.size()<<std::endl;
			CPPUNIT_ASSERT(lower->sent.size() == 7);
			myScheduler->cancelPeriodicTimeout();
			break;
		}
	}
	assure(assert1 && assert2, "Assert not executed");

	compound[0] = lower->sent[0];
	FCHCommand* fchCommand = myScheduler->getCommand(compound[0]->getCommand());
	//CPPUNIT_ASSERT(fchCommand->IEs.size() == 1); //FIXME not implemented in zhe Scheduler yet

	compound[0] = lower->sent[1];
	commandPool[0] = myClassifier->getCommand(compound[0]->getCommand());
	CPPUNIT_ASSERT(commandPool[0]->magic.pduID == 0);//
	compound[0] = lower->sent[2];
	commandPool[0] = myClassifier->getCommand(compound[0]->getCommand());
	CPPUNIT_ASSERT(commandPool[0]->magic.pduID == 1);//
	compound[0] = lower->sent[3];
	commandPool[0] = myClassifier->getCommand(compound[0]->getCommand());
	CPPUNIT_ASSERT(commandPool[0]->magic.pduID == 2);//
	compound[0] = lower->sent[4];
	commandPool[0] = myClassifier->getCommand(compound[0]->getCommand());
	CPPUNIT_ASSERT(commandPool[0]->magic.pduID == 3);//

	compound[0] = lower->sent[5];
	fchCommand = myScheduler->getCommand(compound[0]->getCommand());
	//CPPUNIT_ASSERT(fchCommand->IEs.size() == 1);

	compound[0] = lower->sent[6];
	commandPool[0] = myClassifier->getCommand(compound[0]->getCommand());
	CPPUNIT_ASSERT(commandPool[0]->magic.pduID == 4);//
}

//FIXME test order of compounds and if they have same CID in a burst


// void SchedulerTest::testPeriodicity()
// {
// 	setPtr(compound, NULL);
// } // testOutgoing


