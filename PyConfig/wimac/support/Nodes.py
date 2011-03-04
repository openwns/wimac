import openwns.geometry.position
import openwns.node
from openwns import dBm, dB
import constanze.traffic

import tcp.TCP

import rise.Mobility
import ip.Component

import ofdmaphy.Station

import wimac.Stations
import wimac.Rang

from wimac.support.Transceiver import Transceiver

import scenarios.interfaces
import scenarios.channelmodel


def getOFDMAComponent(node, typeString, _config):
    transceiver = Transceiver(_config)
    if(_config.parametersPhy.beamforming and typeString == "AP"):
        phyStation = ofdmaphy.Station.OFDMABFStation([transceiver.receiver[typeString]],
                                [transceiver.transmitter[typeString]], eirpLimited = _config.parametersPhy.eirpLimited)
    else:
        phyStation = ofdmaphy.Station.OFDMAStation([transceiver.receiver[typeString]],
                                [transceiver.transmitter[typeString]], eirpLimited = _config.parametersPhy.eirpLimited)
    # The following three are default values changed in a later step
    # by the scenario module
    phyStation.txFrequency = _config.centerFrequency # MHz
    phyStation.rxFrequency = _config.centerFrequency # MHz
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

        if _config.parametersMAC.useApplicationLoadGen:
            self.tl = tcp.TCP.UDPComponent(self, "udp", self.nl.dataTransmission, self.nl.notification)

            self.tl.addFlowHandling(
                _dllNotification = self.dll.notification,
                _flowEstablishmentAndRelease = self.dll.flowEstablishmentAndRelease)

            import applications
            import applications.component
            self.load = applications.component.Client(self, "applications")
            self.load.logger.enabled = True
        else:
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

    def setChannelModel(self, channelmodelConfigurations):
        for entry in channelmodelConfigurations: 
            p1 = entry.transceiverPair[0]
            p2 = entry.transceiverPair[1]
            self.phy.ofdmaStation.receiver[0].propagation.setPair(
                    p1, p2).pathloss = entry.channelmodel.pathloss
            self.phy.ofdmaStation.receiver[0].propagation.setPair(
                    p1, p2).shadowing = entry.channelmodel.shadowing
            self.phy.ofdmaStation.receiver[0].propagation.setPair(
                    p1, p2).fastFading = entry.channelmodel.fastFading

            self.phy.ofdmaStation.transmitter[0].propagation.setPair(
                    p1, p2).pathloss = entry.channelmodel.pathloss
            self.phy.ofdmaStation.transmitter[0].propagation.setPair(
                    p1, p2).shadowing =  entry.channelmodel.shadowing
            self.phy.ofdmaStation.transmitter[0].propagation.setPair(
                    p1, p2).fastFading = entry.channelmodel.fastFading


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
    beamforming = False

    def __init__(self, _id, _config):
        super(BaseStation, self).__init__("BS"+str(_id))
        transceiver = Transceiver(_config)
        self.setProperty("Type", "BS")        

        # create the WIMAC DLL
        self.dll = wimac.Stations.BaseStation(self, _config)
        self.dll.setStationID(_id)
        self.phy = getOFDMAComponent(self, "AP", _config)
        self.beamforming = _config.parametersPhy.beamforming 
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
        if(self.beamforming):
            self.phy.ofdmaStation.beamformingAntenna.coordOffset = self.phy.ofdmaStation.antennas[0].coordOffset
        pass

    def addTraffic(self, binding, load):
        assert false, "Don't deploy traffic to base stations"

    def setChannelModel(self, channelmodelConfigurations):
        for entry in channelmodelConfigurations: 
            p1 = entry.transceiverPair[0]
            p2 = entry.transceiverPair[1]
            self.phy.ofdmaStation.receiver[0].propagation.setPair(
                    p1, p2).pathloss = entry.channelmodel.pathloss
            self.phy.ofdmaStation.receiver[0].propagation.setPair(
                    p1, p2).shadowing = entry.channelmodel.shadowing
            self.phy.ofdmaStation.receiver[0].propagation.setPair(
                    p1, p2).fastFading = entry.channelmodel.fastFading

            self.phy.ofdmaStation.transmitter[0].propagation.setPair(
                    p1, p2).pathloss = entry.channelmodel.pathloss
            self.phy.ofdmaStation.transmitter[0].propagation.setPair(
                    p1, p2).shadowing =  entry.channelmodel.shadowing
            self.phy.ofdmaStation.transmitter[0].propagation.setPair(
                    p1, p2).fastFading = entry.channelmodel.fastFading

class RANG(openwns.node.Node):
    dll = None
    nl  = None
    load = None
    ipAddress = None

    def __init__(self, config, name = "RANGWiMAX", id = 1):
        super(RANG, self).__init__(name) 
        self.setProperty("Type", "RANG")        
        # create dll for Rang
        self.dll = wimac.Rang.RANG(self)
        self.dll.stationID = 256 * 255 - id
        self.ipAddress = "192.168.254." + str(255 - id)
        # create Network Layer and Loadgen
        domainName = "RANG" + str(id) + ".wimax.wns.org"
        self.nl = ip.Component.IPv4Component(self, domainName + ".ip", domainName, useDllFlowIDRule = True)

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

        if config.parametersMAC.useApplicationLoadGen:
            self.tl = tcp.TCP.UDPComponent(self, "udp", self.nl.dataTransmission, self.nl.notification)

            import applications
            import applications.component
            self.load = applications.component.Server(self, "applications")
            self.load.logger.enabled = True
        else:
            self.load = constanze.node.ConstanzeComponent(self, "constanze")
