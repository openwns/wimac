import openwns.geometry.position
import openwns.node
from openwns import dBm, dB
import constanze.traffic
import rise.Mobility
import ip.Component

import ofdmaphy.Station

import wimac.Stations
import wimac.Rang

from wimac.support.Transceiver import Transceiver

import scenarios.interfaces

def getOFDMAComponent(node, typeString, _config):
    transceiver = Transceiver(_config)
    phyStation = ofdmaphy.Station.OFDMAStation([transceiver.receiver[typeString]],
                                [transceiver.transmitter[typeString]])
                                
    # The following three are default values changed in a later step
    # by the scenario module
    phyStation.txFrequency = 5470 # MHz
    phyStation.rxFrequency = 5470 # MHz
    phyStation.txPower = dBm(30)
    phyStation.numberOfSubCarrier = _config.parametersPhy.subchannels
    phyStation.bandwidth =  _config.parametersPhy.channelBandwidth
    phyStation.systemManagerName = 'ofdma'
    return ofdmaphy.Station.OFDMAComponent(node, "phy", phyStation)

class SubscriberStation(openwns.node.Node, scenarios.interfaces.INode):
    phy = None
    dll = None
    nl  = None
    load = None
    mobility = None

    def __init__(self, _id, _config):
        super(SubscriberStation, self).__init__("UT"+str(_id))
        self.setProperty("Type", "UE")        
        # create the WIMAX DLL
        self.dll = wimac.Stations.SubscriberStation(self, _config)#, ring = 2)
        self.dll.setStationID(_id)
        self.phy = getOFDMAComponent(self, "UT", _config)
        
        self.dll.setPhyDataTransmission(self.phy.dataTransmission)
        self.dll.setPhyNotification(self.phy.notification)
        # create Network Layer and Loadgen
        domainName = "SS" + str(_id) + ".wimax.wns.org"
        self.nl = ip.Component.IPv4Component(self, domainName + ".ip", domainName)
        self.nl.addDLL(_name = "wimax",
                       # Where to get IP Adresses
                       _addressResolver = ip.AddressResolver.VirtualDHCPResolver("WIMAXRAN"),
                       # Name of ARP zone
                       _arpZone = "WIMAXRAN",
                       # We cannot deliver locally to other UTs without going to the gateway
                       _pointToPoint = True,
                       _dllDataTransmission = self.dll.dataTransmission,
                       _dllNotification = self.dll.notification)

        self.nl.addRoute("0.0.0.0", "0.0.0.0", "192.168.254.254", "wimax")

        self.load = constanze.node.ConstanzeComponent(self, "constanze")
        self.mobility = rise.Mobility.Component(node = self,
                                                name = "mobility UT"+str(_id),
                                                mobility = rise.Mobility.No(openwns.geometry.position.Position())
                                                )

    def setPosition(self, position):
        self.mobility.mobility.setCoords(position)      

    def getPosition(self):
        return self.mobility.mobility.getCoords()

    def setAntenna(self, antenna):
        pass

    def addTraffic(self, binding, load):
        self.load.addTraffic(binding, load) 


class RemoteStation(openwns.node.Node):
    phy = None
    dll = None
    nl  = None
    load = None
    mobility = None

    def __init__(self, _id, _config):
        super(RemoteStation, self).__init__("UT"+str(_id))

        # create the WIMAX DLL
        self.dll = wimac.Stations.RemoteStation(self, _config)#, ring = 4)
        self.dll.setStationID(_id)
        
        self.phy = getOFDMAComponent(self, "UT", _config)
                
        self.dll.setPhyDataTransmission(self.phy.dataTransmission)
        self.dll.setPhyNotification(self.phy.notification)
        # create Network Layer and Loadgen
        domainName = "RS" + str(_id) + ".wimax.wns.org"
        self.nl = ip.Component.IPv4Component(self, domainName + ".ip", domainName)
        self.nl.addDLL(_name = "wimax",
                       # Where to get IP Adresses
                       _addressResolver = ip.AddressResolver.VirtualDHCPResolver("WIMAXRAN"),
                       # Name of ARP zone
                       _arpZone = "WIMAXRAN",
                       # We cannot deliver locally to other UTs without going to the gateway
                       _pointToPoint = True,
                       _dllDataTransmission = self.dll.dataTransmission,
                       _dllNotification = self.dll.notification)

        self.nl.addRoute("0.0.0.0", "0.0.0.0", "192.168.254.254", "wimax")

        self.load = constanze.node.ConstanzeComponent(self, "constanze")
        self.mobility = rise.Mobility.Component(node = self,
                                                name = "mobility UT"+str(_id),
                                                mobility = rise.Mobility.No(openwns.geometry.position.Position()))



class RelayStation(openwns.node.Node):
    phy = None
    dll = None
    mobility = None

    def __init__(self, _id, _config):
        super(RelayStation, self).__init__("FRS"+str(_id))
        self.setProperty("Type", "RN")        

        # create the WIMAX DLL
        self.dll = wimac.Stations.RelayStation(self, _config)
        self.dll.setStationID(_id)
        
        self.phy = getOFDMAComponent(self, "FRS", _config)
                
        self.dll.setPhyDataTransmission(self.phy.dataTransmission)
        self.dll.setPhyNotification(self.phy.notification)
        # create PHY
        self.mobility = rise.Mobility.Component(node = self,
                                                name = "mobility FRS"+str(_id),
                                                mobility = rise.Mobility.No(openwns.geometry.position.Position()))



class BaseStation(openwns.node.Node, scenarios.interfaces.INode):
    phy = None
    dll = None
    mobility = None

    def __init__(self, _id, _config):
        super(BaseStation, self).__init__("BS"+str(_id))
        transceiver = Transceiver(_config)
        self.setProperty("Type", "BS")        

        # create the WIMAC DLL
        self.dll = wimac.Stations.BaseStation(self, _config)
        self.dll.setStationID(_id)
        self.phy = getOFDMAComponent(self, "AP", _config)
                
        self.dll.setPhyDataTransmission(self.phy.dataTransmission)
        self.dll.setPhyNotification(self.phy.notification)
        self.mobility = rise.Mobility.Component(node = self,
                                                name = "mobility BS"+str(_id),
                                                mobility = rise.Mobility.No(openwns.geometry.position.Position()))

    def setPosition(self, position):
        self.mobility.mobility.setCoords(position)      

    def getPosition(self):
        return self.mobility.mobility.getCoords()

    def setAntenna(self, antenna):
        self.phy.ofdmaStation.antennas = [antenna]
        pass

    def addTraffic(self, binding, load):
        assert false, "Don't deploy traffic to base stations"





class RANG(openwns.node.Node):
    dll = None
    nl  = None
    load = None
    ipAddress = None

    def __init__(self, name = "RANGWiMAX", id = 1):
        super(RANG, self).__init__(name) 
        self.setProperty("Type", "RANG")        
        # create dll for Rang
        self.dll = wimac.Rang.RANG(self)
        self.dll.setStationID((256*255)-id)
        self.ipAddress = "192.168.254." + str(255 - id)
        # create Network Layer and Loadgen
        domainName = "RANG" + str(id) + ".wimax.wns.org"
        self.nl = ip.Component.IPv4Component(self, domainName + ".ip", domainName)

        self.nl.addDLL(_name = "tun",
                       # Where to get my IP Address
                       _addressResolver = ip.AddressResolver.FixedAddressResolver(self.ipAddress, "255.255.0.0"),
                       # ARP zone
                       _arpZone = "WIMAXRAN",
                       # We can deliver locally
                       _pointToPoint = False,
                       # DLL service names
                       _dllDataTransmission = self.dll.dataTransmission,
                       _dllNotification = self.dll.notification)

        self.load = constanze.node.ConstanzeComponent(self, "constanze")

