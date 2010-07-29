""" Module WiMAX Parameters - 802.16m
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    This module is a concentrate collection of all necessary configuration
    parameters for a WiMAX system TDD mode with 20 MHz bandwidth.

    The parameters, wich are commented out are actual not in use by
    the WiMAC module.

    The following parameters are the original one, used by ComNets.
    verified 0 = 16m Evaluation Methodology Document (EMD)
    verified 1 = 16m System Discription Document (SDD)
"""

from math import *
from openwns.pyconfig import Frozen
from openwns import dBm, dB

#TODO
def getSamplingFrequencyOFDMA(bandwidth, cyclicPrefix):
    """
        bandwidth in MHz
    """

    # sampling factor refering to IEEE802.16e 8.4.2.3
    if bandwidth == 20 or bandwidth == 10 or bandwidth == 5:
        n = 28/25.0;
    elif bandwidth == 7 or bandwidth == 8.75:
        n = 8/7.0;
    return floor(n * bandwidth * 1e+6 / 8000) * 8000

def getFftSize(BW):

    if BW == 5:
        fft = 512;
    elif BW == 7:
        fft = 1024;
    elif BW == 8.75:
        fft = 1024;
    elif BW == 10:
        fft = 1024;
    elif BW == 20:
        fft = 2048;
    else:
        fft = 0;

    return fft


def getNumberOfChannels(subcarrierAllocation, fftSize):

    if subcarrierAllocation == 'AMC':
        if fftSize == 2048:
            Nsub = 96;       #One PRU with 18 subcarriers in 1729 data subcarriers
        elif fftSize == 1024:
            Nsub = 48;
        elif fftSize == 512:
            Nsub = 24;
        else:
            Nsub = 0;
    else:
        # TODO implement this if PUSC/FUSC is used
        Nsub = 0;
    return Nsub

def getDataSubcarrierPerSubChannel(subcarrierAllocation, fftSize):

    if subcarrierAllocation == 'AMC':
        n = 16;
    else:
        # TODO implement this if PUSC/FUSC is used
        n = 0;

    return n


###############################################################################
# System Parameters                                                           #
###############################################################################
#TODO
class ParametersSystem(Frozen):
                                      # ---Comments----------------------------
    centerFrequency = 2510  # [MHz]   # Not used at the moment by the WiMAC
                                      # implicitely modeled by the propagation model

    # radius of the cell including relays!
    cellRadius  = 550  #577.35 [meter] # (verified 0)
    clusterOrder = 3                   # [cells per cluster]

    height = {}
    height['AP']  = 32.0  # [meter]        # (verified 0)
    height['FRS'] = 32.0  # [meter]
    height['UT']  = 1.5  # [meter]        # (verified 0)

    # in the future differentiate between Tx (SS:1) and Rx antennas (SS:2)
    numberOfAntennaRxTx = {}
    numberOfAntennaRxTx['AP']  = 2        # (verified 0)
    numberOfAntennaRxTx['FRS'] = 2        # (verified 0)
    numberOfAntennaRxTx['UT']  = 1        # (verified 0)

    #this has currently no influence, layout is read directly from config.py
    antennaArrayLayout = {}
    antennaArrayLayout['AP']  = "linear"  #  (verified 0)
    antennaArrayLayout['FRS'] = "linear"  #  (verified 0)
    antennaArrayLayout['UT']  = "linear"  #  (verified 0)

    # Will be changed for each scenario by scenario builder
    # need a concept here
    txPower = {}
    txPower['AP']  = dBm(30) # [dBm]      # (verified 0)
    txPower['FRS'] = dBm(30) # [dBm]      # (verified 0)
    txPower['UT']  = dBm(30) # [dBm]      # (verified 0)
    
    #txPower['AP']  = dBm(46) # [dBm]      # (verified 0)
    #txPower['UT']  = dBm(23) # [dBm]      # (verified 0)

    noiseFigure = {}
    noiseFigure['AP']  = dB(5) # [dB]     # (verified 0)
    noiseFigure['FRS'] = dB(5) # [dB]     # (verified 0)
    noiseFigure['UT']  = dB(7) # [dB]     # (verified 0)

#################################################################################
# OFDMA Parameters according to IEEE 802.16m (with 20MHz bandwidth and TDD mode)#
#################################################################################

class ParametersOFDMA(object):
    #fixme(bmw)
    #symbolDuration = 102.857 * 1e-6
    #subcarrierPerSubchannel = 16
    __name__ = "ParametersOFDMA"
    
    def __init__(self, _bandwidth):
        # OFDMA parameters not yet completed
        # check this before usage!!
        #########################################################################
        #       Primitive Parameters                                            #
        #########################################################################
        self.channelBandwidth = _bandwidth
        # [MHz]
        # [5, 7, 8.75, 10, 20]
        ###########

        cyclicPrefix = 1.0/8
        # Possible values: [1.0/4 1.0/8 1.0/16]
        ######

        fftSize = getFftSize(self.channelBandwidth)
        # Nfft
        ######

        samplingFrequency = getSamplingFrequencyOFDMA(self.channelBandwidth, cyclicPrefix)
        # [Hz]
        ######

        usefulSymbolTime = float(fftSize) / float(samplingFrequency)
        #usefulSymbolTime = 0.0000914 s # (for 5, 10, 20 MHz channel bandwidth)
        # [sec]   Tb=Nfft/Fp
        ######

        guardTime = float(usefulSymbolTime) * float(cyclicPrefix)
        # [sec]   (Tg=Tb*Cp)
        ######

        self.symbolDuration = float(usefulSymbolTime) + float(guardTime)
        # [sec]   (Ts=Tb+Tg)
        ######

        self.frameDuration = 0.005
        # [sec]   (Tf)
        ######

        self.symbolsFrame = 48 #int(floor(float(frameDuration) / float(symbolDuration)))
        # 47 useful symbols + 1.6 symbols for RTG/TTG (for 0.005s frame duration)
        # [symbols](Tf/Ts)
        # Symbols per frame
        ######

        #admentment 16m/D3 16.3.3.2.2 
        self.ttg = 105.714E-6
        self.rtg = 60.0E-6

        self.DL2ULratio = 4/4
        #[8:0, 6:2, 5:3, 4:4, or 3:5] valid for [5, 10, 20] MHz bandwidth
        #[3:2 or 2:3] for 7 MHz and [5:2, 4:3, or 3:4] for 8.75 MHz channel bandwidth
        
        self.symbolsDlSubframe = self.symbolsFrame * self.DL2ULratio
        #############
        self.symbolsUlSubframe = self.symbolsFrame - self.symbolsDlSubframe
        # number of OFDM symbols for UL data, contention phases, and TTG/RTG

        #TODO
        self.subcarrierAllocation = 'AMC'
        # ['PUSC', 'FUSC', 'AMC']
        # PUSC and FUSC not yet supported by WiMAC
        # default is PUSC
        #############

        
        self.subchannels = getNumberOfChannels(self.subcarrierAllocation, fftSize)
        
        self.subcarrierPerSubchannel = getDataSubcarrierPerSubChannel(self.subcarrierAllocation, fftSize)
        self.dataSubCarrier = self.subcarrierPerSubchannel * self.subchannels
        ###############################
        self.minimumBitsPerSymbol = 1/4.0 * self.subchannels * self.subcarrierPerSubchannel
        
        subbandmode = False # gather 4 PRUs in one subchannel
        if subbandmode==True:
            self.subchannels = getNumberOfChannels(self.subcarrierAllocation, fftSize) / 4
            self.minimumBitsPerSymbol = self.minimumBitsPerSymbol * 4
        # QPSK 1/8:
        # PHY mode used to
        # transmit DL and UL MAP
        # maybe include repetition code (factor 2, 4, or 6)
        ##############################

        #TODO
        self.dlPreamble = 1
        self.frameHead = self.dlPreamble
        self.fch = 0
        # length of frame
        # head elements in
        # OFDM symbols
        # TODO: maybe include FCH
        # into MAP phases
        ###############

        self.mapBase = 1
        self.ie      = 1
        # TODO OFDMA
        # DL/UL MAP base size and
        # additional size for each
        # Information Element
        #################
        self.maximalBeams = 1
        self.beamforming = False
        
        self.adaptUTTxPower = False
        # If True: then per subchannel nominal power equals the max power divided by the number 
        # of subchannel and multiplied by the number of user terminals 
        # since we assume all UTs being served in parallel
        # If False: per subchannel nominal power equals the max power since we assume UTs only use few subchannels
        #################
        
#TODO
###############################################################
# MAC Parameters Frame Setup                                  #
###############################################################
class ParametersMAC(Frozen):

    dlUlRatio = 0.5
    # DL ratio of the data frame
    ######

    rangingSlots = 1
    rangingSlotLength = 1  # [symbols]
    # Ranging contention access configuration
    # Reminder: Ranging phase is located in the uplink frame phase
    ######

    bwReqSlots = 1
    bwReqSlotLength = 1 # [symbols]
    # Bandwidth contention access configuration
    # Reminder: Bandwidth phase is located in the uplink frame phase
    ######

    pduOverhead = 48
    # pduOverhead = 48 + 32 # including CRC
    # overhead due to MAC header

    subFrameRatio = 0
    #  sub frame ratio of the uplink frame phase
    #  Reminder: Sub frame is located in the uplink frame phase.
    #####

    useApplicationLoadGen = False
    
    import wimac.Services
    associationService = "BestAtGivenTime" # "Fixed"
    
    # Only used with "BestAtGivenTime"
    bestAssociationCriterion = wimac.Services.BestAtGivenTime.BestPathloss()
    associationDecisionTime = 0.00011
    # Associate after first FCH is received. Choose BS 
    # with lowest pathloss on link.
    # ToDo: Automatically set decision time to shortly after first 
    # FCH is received.
    ########


###############################################################################
#  Propagation Model                                                          #
###############################################################################

from openwns.interval import Interval
import rise.scenario.Propagation
import rise.scenario.Pathloss as Pathloss
import rise.scenario.Shadowing as Shadowing
import rise.scenario.FastFading as FastFading

class ParametersPropagation(Frozen):

    ###### Define Propagation Models ##########################################

    __ClLOS = rise.scenario.Propagation.Configuration(
        pathloss = Pathloss.SingleSlope(validFrequencies = Interval(2000, 6000),
                                    validDistances = Interval(2, 20000), # [m]
                                    offset = "41.9 dB",
                                    freqFactor = 0,
                                    distFactor = "23.8 dB",
                                    distanceUnit = "m", # nur fuer die Formel, nicht fuer validDistances
                                    minPathloss = "49.06 dB", # pathloss at 2m distance
                                    outOfMinRange = Pathloss.Constant("49.06 dB"), #Pathloss.FreeSpace(),
                                    outOfMaxRange = Pathloss.Deny()
                                    ),
        shadowing = Shadowing.No(),
        fastFading = FastFading.No())
    # (verified 2) Models suggested by hoy
    ######

    __ClNLOS = rise.scenario.Propagation.Configuration(
        pathloss = Pathloss.SingleSlope(validFrequencies = Interval(4000, 6000),
                                    validDistances = Interval(8, 5000), # [m]
                                    offset = "27.7 dB",
                                    freqFactor = 0,
                                    distFactor = "40.2 dB",
                                    distanceUnit = "m", # nur fuer die Formel, nicht fuer validDistances
                                    minPathloss = "64.0 dB", # pathloss at 8m distance
                                    outOfMinRange = Pathloss.Constant("64.0 dB"),
                                    outOfMaxRange = Pathloss.Deny()
                                    ),
        shadowing = Shadowing.No(),
        fastFading = FastFading.No())
    # (verified 2) Models suggested by hoy
    ######


    ##### Used Propagation Models   ############################################

    # AP <-> AP
    channelLinkAP2AP   = __ClLOS

    # AP <-> FRS
    channelLinkAP2FRS  = __ClLOS
    channelLinkFRS2AP  = __ClLOS

    # AP <-> UT
    channelLinkAP2UT   = __ClLOS
    channelLinkUT2AP   = __ClLOS

    # FRS <-> FRS
    channelLinkFRS2FRS = __ClLOS

    # FRS <-> UT
    channelLinkFRS2UT  = __ClLOS
    channelLinkUT2FRS  = __ClLOS

    # UT <-> UT
    channelLinkUT2UT   = __ClLOS



class ParametersPropagation_NLOS(Frozen):

    ###### Define Propagation Models ##########################################

    __ClNLOS = rise.scenario.Propagation.Configuration(
        pathloss = Pathloss.SingleSlope(validFrequencies = Interval(4000, 6000),
                                    validDistances = Interval(8, 5000), # [m]
                                    offset = "27.7 dB",
                                    freqFactor = 0,
                                    distFactor = "40.2 dB",
                                    distanceUnit = "m", # nur fuer die Formel, nicht fuer validDistances
                                    minPathloss = "64.0 dB", # pathloss at 8m distance
                                    outOfMinRange = Pathloss.Constant("64.0 dB"),
                                    outOfMaxRange = Pathloss.Deny()
                                    ),
        shadowing = Shadowing.No(),
        fastFading = FastFading.No())
    # (verified 2) Models suggested by hoy
    ######


    ##### Used Propagation Models   ############################################

    # AP <-> AP
    channelLinkAP2AP   = __ClNLOS

    # AP <-> FRS
    channelLinkAP2FRS  = __ClNLOS
    channelLinkFRS2AP  = __ClNLOS

    # AP <-> UT
    channelLinkAP2UT   = __ClNLOS
    channelLinkUT2AP   = __ClNLOS

    # FRS <-> FRS
    channelLinkFRS2FRS = __ClNLOS

    # FRS <-> UT
    channelLinkFRS2UT  = __ClNLOS
    channelLinkUT2FRS  = __ClNLOS

    # UT <-> UT
    channelLinkUT2UT   = __ClNLOS


