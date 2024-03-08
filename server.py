#! /usr/bin/python
#
# A simple UDP packet receiver with a twist.  This gets a potentially very large
# packet to demonstrate fragmentation of a UDP packet, so we can test packet
# reassembly

import socket
from struct import *
from paho.mqtt.client import Client

from ha_mqtt.mqtt_device_base import MqttDeviceSettings
from ha_mqtt.mqtt_thermometer import MqttThermometer
from ha_mqtt.mqtt_sensor import MqttSensor
from ha_mqtt.mqtt_device_base import MqttDeviceBase, MqttDeviceSettings
from ha_mqtt.util import HaDeviceClass
import datetime


class MqttSensorVoltage(MqttSensor):
    """
    subclass of MqttSensor, measures temperatures.
    The default unit is °C and can be changed in the constructor
    """

    def __init__(self, settings: MqttDeviceSettings, unit: str = "mV", send_only=True):
        super().__init__(settings, unit, HaDeviceClass.VOLTAGE, send_only)



class MqttSensorHumidity(MqttSensor):
    """
    subclass of MqttSensor, measures temperatures.
    The default unit is °C and can be changed in the constructor
    """

    def __init__(self, settings: MqttDeviceSettings, unit: str = "%", send_only=True):
        super().__init__(settings, unit, HaDeviceClass.HUMIDITY, send_only)

class MqttSensorMoisture(MqttSensor):
    """
    subclass of MqttSensor, measures temperatures.
    The default unit is °C and can be changed in the constructor
    """

    def __init__(self, settings: MqttDeviceSettings, unit: str = "%", send_only=True):
        super().__init__(settings, unit, HaDeviceClass.MOISTURE, send_only)

class MqttSensorLUX(MqttSensor):
    """
    subclass of MqttSensor, measures temperatures.
    The default unit is °C and can be changed in the constructor
    """

    def __init__(self, settings: MqttDeviceSettings, unit: str = "lx", send_only=True):
        super().__init__(settings, unit, HaDeviceClass.ILLUMINANCE, send_only)


class MqttSensorRSSI(MqttSensor):
    """
    subclass of MqttSensor, measures temperatures.
    The default unit is °C and can be changed in the constructor
    """

    def __init__(self, settings: MqttDeviceSettings, unit: str = "dBm", send_only=True):
        super().__init__(settings, unit, HaDeviceClass.SIGNAL_STRENGTH, send_only)

UDP_IP=""    # listen to anything IPv4 or IPv6
UDP_PORT=XXXXXX

sock = socket.socket( socket.AF_INET6, # Internet IPv4 or IPv6
                      socket.SOCK_DGRAM ) # UDP
sock.bind( (UDP_IP,UDP_PORT) )

client = Client("testscript")
client.connect("XXXXXXXX", 1883)
client.loop_start()

settings = MqttDeviceSettings("Thermometer 1", "temp1", client)
th = MqttThermometer(settings, unit="°C")

settings2 = MqttDeviceSettings("VoltageThingy91", "voltage1",client)
th2 = MqttSensorVoltage(settings2, unit="V")
#print(calcsize('BBBBBBBB?BHfHHfdIBBBB'))


try:
 while True:
  data, addr = sock.recvfrom( 1048576 ) # buffer size huge
  print("received message from ", addr,  "Lenth of data: ", len(data))
  print("byte converted to hexadecimal value:",data.hex())
  val = unpack('BBBBBBBB?BHfffff?Qff', data)
  name = "Farm-" + f'{int(val[6]):x}' + "-"+ f'{int(val[5]):x}' + "-"+ f'{int(val[4]):x}' + "-"+ f'{int(val[3]):x}' + "-"+ f'{int(val[2]):x}' + ""+ f'{int(val[1]):x}' +"-"
  nameTemp = name + "Temp"
  nameVoltage = name + "Voltage"
  nameHumi = name + "Humi"
  nameLUX = name + "LUX"
  nameMoisture = name + "Moisture"
  nameRSSI = name + "RSSI"
  if(int(val[0])!=0):
   s = int(val[17]) / 1000.0
   d0 = datetime.datetime.fromtimestamp(s)
   d2 = datetime.datetime.now() - datetime.timedelta(minutes=30)
   if(d2<d0):
    print("Inside time")
    settings = MqttDeviceSettings(nameTemp, nameTemp, client)
    th = MqttThermometer(settings, unit="°C")
    print(nameTemp)
    print(nameVoltage)
    print(f"publishing temperature: {val[11]} {th.unit_of_measurement}")
    th.publish_state(round(val[11],2))
    settings2 = MqttDeviceSettings(nameVoltage, nameVoltage, client)
    th2 = MqttSensorVoltage(settings2, unit="mV")
    th2.publish_state(val[10])

    settings3 = MqttDeviceSettings(nameHumi, nameHumi, client)
    th3 = MqttSensorHumidity(settings3, unit="%")
    th3.publish_state(val[12])

    settings4 = MqttDeviceSettings(nameMoisture, nameMoisture, client)
    th4 = MqttSensorMoisture(settings4, unit="%")
    th4.publish_state(val[13])

    settings5 = MqttDeviceSettings(nameLUX, nameLUX, client)
    th5 = MqttSensorLUX(settings5, unit="lx")
    th5.publish_state(val[14])

    settings6 = MqttDeviceSettings(nameRSSI, nameRSSI, client)
    th6 = MqttSensorLUX(settings6, unit="dBm")
    th6.publish_state(val[15])

   else:
    print("Outside")
  else:  
   settings = MqttDeviceSettings(nameTemp, nameTemp, client)
   th = MqttThermometer(settings, unit="°C")
   print(nameTemp)
   print(nameVoltage)
   print(f"publishing temperature: {val[11]} {th.unit_of_measurement}")
   th.publish_state(round(val[11],2))
   settings2 = MqttDeviceSettings(nameVoltage, nameVoltage, client)
   th2 = MqttSensorVoltage(settings2, unit="mV")
   th2.publish_state(val[10])

   settings3 = MqttDeviceSettings(nameHumi, nameHumi, client)
   th3 = MqttSensorHumidity(settings3, unit="%")
   th3.publish_state(val[12])

   settings4 = MqttDeviceSettings(nameMoisture, nameMoisture, client)
   th4 = MqttSensorMoisture(settings4, unit="%")
   th4.publish_state(val[13])

   settings5 = MqttDeviceSettings(nameLUX, nameLUX, client)
   th5 = MqttSensorLUX(settings5, unit="lx")
   th5.publish_state(val[14])

   settings6 = MqttDeviceSettings(nameRSSI, nameRSSI, client)
   th6 = MqttSensorLUX(settings6, unit="dBm")
   th6.publish_state(val[15])

finally:
    # close the device for cleanup. Gets marked as offline/unavailable in homeassistant
    th.close()
    client.loop_stop()
    client.disconnect()
