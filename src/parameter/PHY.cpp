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

#include <WIMAC/parameter/PHY.hpp>
#include <WIMAC/Logger.hpp>

using namespace wimac::parameter;

void
PHY::init(const wns::pyconfig::View& config)
{
	LOG_INFO("Loading WiMAC PHY parameters from config");

	channelBandwidth = config.get<double>("channelBandwidth");
	cyclicPrefix = config.get<double>("cyclicPrefix");
	samplingFrequency = config.get<double>("samplingFrequency");
	fftSize = config.get<int>("fftSize");
	subCarrierSpacing = config.get<double>("subCarrierSpacing");
	usefulSymbolTime = config.get<double>("usefulSymbolTime");
	guardTime = config.get<double>("guardTime");
	symbolDuration = config.get<double>("symbolDuration");
	frameDuration = config.get<double>("frameDuration");
	symbolsPerFrame = config.get<int>("symbolsFrame");
	ttg = config.get<double>("ttg");
	rtg = config.get<double>("rtg");
	guardSubCarrier = config.get<int>("guardSubCarriers");
	dcSubCarrier = config.get<int>("dcSubCarrier");
	pilotSubCarrier = config.get<int>("pilotSubCarrier");
	dataSubCarrier = config.get<int>("dataSubCarrier");
	subChannels = config.get<int>("subchannels");
	subCarrierPerSubChannel = config.get<int>("subcarrierPerSubchannel");
	minimumBitsPerSymbol = config.get<int>("minimumBitsPerSymbol");
	dlPreamble = config.get<int>("dlPreamble");
	fch = config.get<int>("fch");
	frameHead = config.get<int>("frameHead");
	mapBase = config.get<int>("mapBase");
	ie = config.get<int>("ie");
}


