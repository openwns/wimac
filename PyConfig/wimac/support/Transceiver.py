from openwns.pyconfig import Sealed
from rise.Transmitter import Transmitter
import rise.scenario.Propagation
from ofdmaphy.Receiver import OFDMAReceiver

from rise.scenario.FTFading import *
#from rise.scenario.FTFadingConfiguration import *

class Transceiver(Sealed):
    propagation = None
    receiver = None
    transmitter = None

    def __init__(self, _config):
        self.receiver={}
        self.transmitter={}

	 ###### Set Propagation ######################################################
        self.propagation = rise.scenario.Propagation.Propagation()


        ##### Set Transceiver #########################################################
        self.receiver['AP'] = OFDMAReceiver( propagation = self.propagation,
                                             propagationCharacteristicName = "AP",
                                             FTFadingStrategy = FTFadingOff())
        self.receiver['AP'].logger.enabled = False

        self.receiver['FRS'] = OFDMAReceiver( propagation = self.propagation,
                                              propagationCharacteristicName = "FRS",
                                              FTFadingStrategy = FTFadingOff())
        self.receiver['FRS'].logger.enabled = False

        self.receiver['UT'] = OFDMAReceiver( propagation = self.propagation,
                                             propagationCharacteristicName = "UT",
                                             FTFadingStrategy = FTFadingOff())
        self.receiver['UT'].logger.enabled = False

        self.transmitter['AP'] = Transmitter( propagation = self.propagation,
                                              propagationCharacteristicName = "AP" )
        self.transmitter['AP'].logger.enabled = False

        self.transmitter['FRS'] = Transmitter( propagation = self.propagation,
                                               propagationCharacteristicName = "FRS" )
        self.transmitter['FRS'].logger.enabled = False

        self.transmitter['UT'] = Transmitter( propagation = self.propagation,
                                              propagationCharacteristicName = "UT" )
        self.transmitter['UT'].logger.enabled = False

