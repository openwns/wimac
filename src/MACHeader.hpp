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

#ifndef WIMAC_MACHEADER_HPP
#define WIMAC_MACHEADER_HPP

#include <WNS/ldk/ldk.hpp>
#include <WNS/ldk/Command.hpp>

namespace wimac {

class MACHeader
	: public wns::ldk::Command
{
public:
	enum HeaderType {
		GenericMACHeader       = 0x0,
		BandwidthRequestHeader = 0x1
	};

	enum SubHeaderType {
		FastFeedbackOrGrantManagement = 0x01,
		Packing                       = 0x02,
		Fragmentation                 = 0x04,
		ExtendedType                  = 0x08,
		ARQFeedbackPayload            = 0x10,
		MeshSubheader                 = 0x20
	};

	/// The size of the MACHeader is always 48 Bit.
	Bit getSize() const { return 48; }

	void setSubHeaderType( int _subHeaderType ) { subHeaderType = _subHeaderType; }
	int getSubHeaderType() const { return subHeaderType; }

	virtual ~MACHeader() = 0; // never instantiate this

struct {} local;

struct {
	HeaderType headerType;
	uint CID;
	SubHeaderType subHeaderType;
} peer;

struct {} magic;

private:

	uint CID;
	int subHeaderType;
};



class BandwidthRequestHeader
	: public MACHeader
{
public:
	enum BandwidthRequestType
	{
		incremental = 0,
		aggregate   = 1
	};

	/// Set the number of bytes of uplink bandwidth requested by the SS.
	void setBandwidthRequest( uint _bandwidthRequest ) 
	    { bandwidthRequest = _bandwidthRequest; }

    /// Get the number of bytes of uplink bandwidth requested by the SS.
	uint getBandwidthRequest() const { return bandwidthRequest; }


	void setBandwidthRequestType( int _bandwidthRequestType ) 
	    { bandwidthRequestType = _bandwidthRequestType; }

	int getBandwidthRequestType() const { return bandwidthRequestType; }

struct {} local;
struct {} peer;
struct {} magic;

private:
	uint bandwidthRequest;
	int bandwidthRequestType;
};


class GenericMACHeader
	: public MACHeader
{
public:
	/// Get the length of the payload in byte including MAC header and CRC if present. 
	uint getLength() const { return length; }

struct {} local;
struct {} peer;
struct {} magic;

private:
	uint length;
};



class MACSubHeader :
	public GenericMACHeader 
{
	virtual ~MACSubHeader() = 0; // never instantiate this

struct {} local;
struct {} peer;
struct {} magic;

};




class GrantManagementSubHeader
	: public MACSubHeader
{
public:


struct {} local;
struct {} peer;
struct {} magic;

private:
	bool pollMe;
	bool slipIndicator;

	uint piggyBackRequest;
};

class PackingSubheader
	: public MACSubHeader
{

public:

	enum FragmentationType
	{
		NoFragmentation = 0,
		LastFragment    = 1,
		FirstFragment   = 2,
		MiddleFragment  = 3
	};


struct {} local;
struct {} peer;
struct {} magic;


private:
	uint fragmentationType;

};




class MeshSubheader
	: public MACSubHeader
{
public:

struct {} local;
struct {} peer;
struct {} magic;

private:
	uint xmitNodeId;
};




class FASTFEEDBACKHSubHeader
	: public MACSubHeader
{
public:
	enum FeedbackType 
	{
		FastDLMeasurement        = 0,
		FastMIMOFeedbackAntenna0 = 1,
		FastMINOFeedbackAntenna1 = 2,
		MIMOModeAndPermitation   = 3
	};

	void setFeedbackType( int _feedbackType )
	{ feedbackType = _feedbackType; }

	int getFeedbackType() const { return feedbackType; }

struct {} local;
struct {} peer;
struct {} magic;


private: 

	uint allocationOffset;
	int feedbackType;
};





class MACManagementMessage :
	public wns::ldk::Command

{

public:

	enum ManagementMessageType
	{
		UCD          =  0,
		DCD          =  1,
		DL_MAP       =  2,
		UL_MAP       =  3,
		RNG_REQ      =  4,
		RNG_RSP      =  5,
		REG_REQ      =  6,
		REG_RSP      =  7,
		PKM_REQ      =  9,
		PKM_RSP      = 10,
		DSA_REQ      = 11,
		DSA_RSP      = 12,
		DSA_ACK      = 13,
		DSC_REQ      = 14,
		DSC_RSP      = 15,
		DSC_ACK      = 16,
		DSD_REQ      = 17,
		DSD_RSP      = 18,
		MCA_REQ      = 21,
		MCA_RSP      = 22,
		DBPC_REQ     = 23,
		DBPC_RSP     = 24,
		RES_CMD      = 25,
		SBC_REQ      = 26,
		SBC_RSP      = 27,
		CLK_CMP      = 28,
		DREG_CMD     = 29,
		DSX_RVD      = 30,
		TFTP_CPLT    = 31,
		TFTP_RSP     = 32,
		ARQ_FB       = 33,
		ARQ_DSC      = 34,
		ARQ_RST      = 35,
		REP_REQ      = 36,
		REP_RSP      = 37,
		FPC          = 38,
		MSH_NCFG     = 39,
		MSH_NENT     = 40,
		MSH_DSCH     = 41,
		MSH_CSCH     = 42,
		MSH_CSCF     = 43,
		AAS_FBCK_REQ = 44,
		AAS_FBCK_RSP = 45,
		AAS_BEAM_SEL = 46,
		AAS_BEAM_REQ = 47,
		AAS_BEAM_RSP = 48,
		DREG_REQ     = 49,
		MOB_SCN_REQ  = 54,
		MOB_SCN_RSP  = 55,
		MOB_MSHO_REQ = 57,
		MOB_BSHO_RSP = 58,
        MOB_HO_IND   = 59
	  };

	int managementMessageType;
	uint downlinkChannelID;
	uint configurationChangeCount;

struct {} local;

struct {
	ManagementMessageType managementMessageType;
} peer;

struct {} magic;


};
}

#endif

