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


#include <WIMAC/frame/TimingControl.hpp>

#include <utility>

#include <WNS/pyconfig/View.hpp>
#include <WNS/ldk/Layer.hpp>
#include <WNS/ldk/fcf/FrameBuilder.hpp>
#include <WNS/ldk/fcf/CompoundCollector.hpp>
#include <WNS/probe/bus/ContextProviderCollection.hpp>
#include <WNS/probe/bus/ContextCollector.hpp>
#include <boost/bind.hpp>

#include <WIMAC/Logger.hpp>
#include <WIMAC/parameter/PHY.hpp>

STATIC_FACTORY_REGISTER_WITH_CREATOR(
   wimac::frame::TimingControl,
   wns::ldk::fcf::TimingControlInterface,
   "wimac.frame.TimingControl",
   wns::ldk::fcf::FrameBuilderConfigCreator );

using namespace wimac::frame;

namespace wimac { namespace frame {


        class TriggerActivationStart
        {
        public:
            TriggerActivationStart(TimingControl* timingControl) :
                timingControl_(timingControl) {}

            void operator()()
            {
                timingControl_->startProcessingActivations();
            }

        private:
            TimingControl* timingControl_;
        };
    }
}

TimingControl::TimingControl( wns::ldk::fcf::FrameBuilder* fb, const wns::pyconfig::View& config) :
    wns::ldk::fcf::TimingControlInterface(),
    frameBuilder_(fb),
    running_(false),
    config_(config),
    frameStartupDelay_(config.get<wns::simulator::Time>("frameStartupDelay"))
{
    assure( config.knows("activations"),
            "Activations are not specified in TimingControl" );

    wns::probe::bus::ContextProviderCollection& cpc =
        getFrameBuilder()->getFUN()->getLayer()->getContextProviderCollection();

    cpc.addProvider(wns::probe::bus::contextprovider::Callback(
        "OffsetFromFrameStart",
        boost::bind(&TimingControl::getOffset, this)));

}

void TimingControl::start()
{
    activeCC_ = activations_.end();
    running_ = true;

    if ( !this->hasPeriodicTimeoutSet() )
        this->startPeriodicTimeout(parameter::ThePHY::getInstance()->getFrameDuration(),
            parameter::ThePHY::getInstance()->getFrameDuration());

    // to allow first frame to begin immediately do:
    // this->periodically();
}

void TimingControl::pause()
{
    running_ = false;
    activeCC_ = activations_.end();
}

void TimingControl::stop()
{
    this->cancelPeriodicTimeout();
    this->cancelTimeout();
    activeCC_ = activations_.end();
    running_ = false;
}


void TimingControl::configure()
{
    onFUNCreated();
}

void TimingControl::onFUNCreated()
{
    double sumDuration = 0.0;

    for ( int i = 0; i < config_.len("activations"); ++i ) {
        wns::pyconfig::View activationConfig( config_, "activations", i );

        int mode = activationConfig.get<int>("mode.mode");
        double duration = activationConfig.get<double>("duration");
        int action = activationConfig.get<int>("action.action");

        wns::ldk::fcf::CompoundCollector* compoundCollector;

        if(action != TimingControl::Pause){

            std::string ccName = activationConfig.get<std::string>("compoundCollector");

            compoundCollector = ( getFrameBuilder()->getFUN()->
                                  findFriend<wns::ldk::fcf::CompoundCollector*>(ccName));

            if (!compoundCollector) {
                std::stringstream ss;
                ss << "FU " << ccName << " is not a compound collector, "
                   << "unable to register action for " << ccName;
                throw wns::Exception(ss.str());
            }

            compoundCollector->setFrameBuilder( getFrameBuilder() );

            LOG_INFO( getFrameBuilder()->getFUN()->getName(),
                      ": new activation entry for CC: ",
                      compoundCollector->getName(), ", with mode: ",
                      wns::ldk::fcf::CompoundCollector::mode2String(mode),
                      ", action: ", action, ", and duration: ", duration);
        }
        else if(action == TimingControl::Pause){
            // there is no compound collector needed for Pause Activations
            compoundCollector = NULL;

            LOG_INFO( getFrameBuilder()->getFUN()->getName(),
                      ": new Pause activation entry with mode: ",
                      wns::ldk::fcf::CompoundCollector::mode2String(mode),
                      ", action: ", action, ", and duration: ", duration);
        }

        ActivationEntry entry;
        entry.mode = mode;
        entry.action = action;
        entry.compoundCollector = compoundCollector;
        entry.duration = duration;

        if(action == TimingControl::Start && action || action == TimingControl::Pause){
            // only these actions are really time consuming
            sumDuration += duration;
        }

        activations_.push_back( entry );

    }
    LOG_INFO( getFrameBuilder()->getFUN()->getName(),  " ", activations_.size(),
              " activations registered at timing control, sum duration: ",
              sumDuration, ", frame duration is: ",
              parameter::ThePHY::getInstance()->getFrameDuration());

    LOG_INFO( "difference is ",
              parameter::ThePHY::getInstance()->getFrameDuration() - sumDuration );

    assure(sumDuration <= parameter::ThePHY::getInstance()->getFrameDuration(),
           "the sum of all phases does not fit into the frame duration");

    activeCC_ = activations_.end();
    //timeoutIsForMe = activations_.end();
}


#ifndef WNS_NDEBUG
void TimingControl::finishedPhase(wns::ldk::fcf::CompoundCollectorInterface* collector)
#else
void TimingControl::finishedPhase(wns::ldk::fcf::CompoundCollectorInterface*)
#endif
{
    assure(activeCC_->compoundCollector == collector,
           "An inactive compound collector has reported to have finished.");

    //assure(timeoutIsForMe == activeCC_, "current timeout is not right");

    assure(this->hasTimeoutSet(),
           "the current phase (the last one?) finished after the final phase!?");

    if ( activeCC_ == activations_.end() )
    {
        std::stringstream ss;
        ss << "timing inconsistency" << std::endl;
        ss << "nextPhase() called, but no active compound collector in  ";
        ss << getFrameBuilder()->getFUN()->getLayer()->getName() << std::endl;
        throw wns::Exception( ss.str() );
    }

    LOG_INFO( getFrameBuilder()->getFUN()->getLayer()->getName(), " current phase has finished");

    //if you want that the phases start subsequently do:
    // this->cancelTimeout();
    // this->onTimeout();
}

void
TimingControl::periodically()
{
    //Notify NewFrame-Observers about newFrame
    frameBuilder_->notifyNewFrameObservers();

    frameStartTime_ = wns::simulator::getEventScheduler()->getTime();

    // Only continue if running == true
    if ( !running_ )
        return;

    TriggerActivationStart event (this);

    wns::simulator::getEventScheduler()
        ->schedule(event, frameStartTime_ + frameStartupDelay_ );
}

void
TimingControl::startProcessingActivations()
{
    // Frame duration ends before last timing node is called
    if ( activeCC_ != activations_.end())
    {
        LOG_WARN( getFrameBuilder()->getFUN()->getLayer()->getName(),
                  ": FrameBuilder has not yet finished current frame");
    }

    LOG_INFO( getFrameBuilder()->getFUN()->getName(),  ": Starting Frame");

    activeCC_ = activations_.begin();
    processOneActivation();
}

void
TimingControl::onTimeout()
{
    LOG_INFO(getFrameBuilder()->getFUN()->getName()," TimingControl received timeout");

    if (activeCC_->compoundCollector){
        // pause activations do not have a compound collector
        activeCC_->compoundCollector->stop();
    }

    ++activeCC_;

    if ( activeCC_ != activations_.end() ){
        LOG_TRACE( getFrameBuilder()->getFUN()->getName(), ": Next Activation" );
        processOneActivation();

    } else {
        LOG_INFO( getFrameBuilder()->getFUN()->getName(),
                  ": Last compound collector finished");
    }
}

void
TimingControl::processOneActivation()
{
    assure( activeCC_ != activations_.end(), "cannot process empty activation" );

    while ( activeCC_->action == TimingControl::StartCollection ||
            activeCC_->action == TimingControl::FinishCollection ) {

        switch ( activeCC_->action ) {
        case TimingControl::StartCollection:
            activeCC_->compoundCollector->setMaximumDuration( activeCC_->duration );
            activeCC_->compoundCollector->startCollection( activeCC_->mode );
            break;
        case TimingControl::FinishCollection:
            activeCC_->compoundCollector->finishCollection();
            break;
        default:
            throw wns::Exception("Unknown activation in Timing Control");
        }
        ++activeCC_;
    }

    if ( activeCC_ == activations_.end() )
        return;

    switch ( activeCC_->action ) {

    case TimingControl::Start:
        activeCC_->compoundCollector->setMaximumDuration( activeCC_->duration );
        activeCC_->compoundCollector->start( activeCC_->mode );

        //set timeout to end of this phase
        this->setTimeout( (*activeCC_).duration );

        LOG_INFO( getFrameBuilder()->getFUN()->getName(),  ": next phase activated with a duration of ",
                  (*activeCC_).duration );
        break;

    case TimingControl::Pause:
        //set timeout to end of this phase
        this->setTimeout( (*activeCC_).duration );

        LOG_INFO( getFrameBuilder()->getFUN()->getName(),  ": pause for a duration of ",
                  (*activeCC_).duration );
        break;
    default:
        throw wns::Exception("Illegal activation entry in " +
                             getFrameBuilder()->getFUN()->getName());
    }
}

int
TimingControl::getOffset()
{
    simTimeType now = wns::simulator::getEventScheduler()->getTime();
    double offset = now - frameStartTime_;
    int symOffset = int(offset / parameter::ThePHY::getInstance()->getSymbolDuration());
    return symOffset;
}
