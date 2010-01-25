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


#include <WIMAC/Logger.hpp>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif

const std::string
wimac::demangledTypename( const std::string& nativeName)
{
#ifdef _MSC_VER
    return nativeName;
#else
    const size_t max = 1024;
    char buf[max + 1];
    size_t length = max;
    int result;

    abi::__cxa_demangle( nativeName.c_str(), buf, &length, &result);
    buf[length] = '\0';
    return buf;
#endif
}

