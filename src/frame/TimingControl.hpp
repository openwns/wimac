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

#ifndef WIMAC_FRAME_TIMINGCONTROL_H
#define WIMAC_FRAME_TIMINGCONTROL_H

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/fcf/TimingControl.hpp>
#include <WNS/events/CanTimeout.hpp>


NAMESPACE_BEGIN(wns)

NAMESPACE_BEGIN(pyconfig)

class View;

NAMESPACE_END

NAMESPACE_BEGIN(ldk)
NAMESPACE_BEGIN(fcf)

class FrameBuilder;
class CompoundCollectorInterface;

NAMESPACE_END
NAMESPACE_END
NAMESPACE_END

NAMESPACE_BEGIN(wimac)
NAMESPACE_BEGIN(frame)

/**
 * @brief WiMAC specific Timing Control.
 *
 * The Timing Control of the WiMAC module introduces abstract Activations
 * to the Frame Configuration Framework. Each CompoundCollector is activated
 * by three different types of Activations (Start, StartCollection,	FinishCollection).
 * The Activations specify the concrete action the CompoundCollector has to perform.
 * The chronologically ordered list of Activations is created from (/defined by) the PyConig file.
 */
class TimingControl :
	public virtual wns::ldk::fcf::TimingControlInterface,
	public wns::events::PeriodicTimeout,
	public wns::events::CanTimeout
{
public:

	enum Activation	{
		Start,
		StartCollection,
		FinishCollection,
		Pause
	};

	TimingControl( wns::ldk::fcf::FrameBuilder* fb, const wns::pyconfig::View& config );

	/// Inherited from TiminControlInterface
	void configure();
	void start();
	void pause();
	void stop();
	void getRole();
	void finishedPhase( wns::ldk::fcf::CompoundCollectorInterface* collector );
	wns::ldk::fcf::FrameBuilder* getFrameBuilder() const
	{
		return frameBuilder_;
	}


	/// Inherited from PeriodicTimeout
	void periodically();

	/// Inherited from CanTimeout
	void onTimeout();

	void onFUNCreated();

private:
	struct ActivationEntry
	{
		wns::ldk::fcf::CompoundCollectorInterface* compoundCollector;
		int mode;
		int action;
		double duration;
	};
	typedef std::list<ActivationEntry> Activations;

	void startProcessingActivations();
	void processOneActivation();

	/// cronologically ordered list of activations.
	/// For the modes sending and receiving a CompoundCollector has three
	/// Activations: startCollection, finishCollection, start
	/// For the mode pause only one activation is valid: pause
	Activations activations_;
	/// iterator through activations
	Activations::const_iterator activeCC_;

	/// iterator is used for validation only ( assure(timeoutIsForMe == activeCC_, ...) )
	//Activations::const_iterator timeoutIsForMe;

	wns::ldk::fcf::FrameBuilder* frameBuilder_;

	bool running_;
	wns::pyconfig::View config_;

	simTimeType frameStartupDelay_;

	friend class TriggerActivationStart;
};

NAMESPACE_END
NAMESPACE_END

#endif

