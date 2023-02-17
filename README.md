# Wireless-Sensors-Network-for-nursing-home-application

In this repo, a health monitoring system for
nursing home implemented by wireless sensor network is
proposed. The monitoring system is designed to monitoring the
heart beat rate of the patients moving in the yard, the sensor
node will send the gateway an alarm when abnormal value
detected. The network is self-organised and topology change
tolerated. Besides, the system is able to obtain the environment
information like the temperature, humidity and windspeed. A
gateway is built to collect and display all the information
obtained from the system.

# 1. Hardware

## MicroController

Zolertia mote: Lightweight and powerful IoT hardware development platform


![zolertia board](https://github.com/khanhongithub/Wireless-Sensors-Network-for-nursing-home-application/blob/master/Group_Project/20230216125522.png)

The RE-Mote in a nutshell packs:

* ISM 2.4-GHz IEEE 802.15.4 & Zigbee compliant radio.
* ISM 863-950-MHz ISM/SRD band IEEE 802.15.4 compliant radio.
* ARM Cortex-M3 32 MHz clock speed, 512 KB flash and 32 KB RAM (16 KB retention)
* AES-128/256, SHA2 Hardware Encryption Engine
* ECC-128/256, RSA Hardware Acceleration Engine for Secure Key Exchange
* User and reset button
* Consumption down to 150 nA using the shutdown mode.
* Programming over BSL without requiring to press any button to enter bootloader mode.
* Built-in battery charger (500 mA), facilitating Energy Harvesting and direct connection to Solar Panels and to standards LiPo batteries.
* Wide range DC Power input: 3.3-16 V.
* Small form-factor (73 x 40 mm).
* MicroSD (over SPI).
* On board RTCC (programmable real time clock calendar) and external watchdog timer (WDT).
* Programmable RF switch to connect an external antenna either to the 2.4 GHz or to the Sub 1 GHz RF interface through the RP-SMA connector.
* Supported in Open Source Operative Systems as Contiki, RIOT and OpenWSN (in progress).

9 motes are used as nodes in this network. 

The programming on the board is built based on Contiki OS, so Contiki OS enviroment needs to be set up first by follow this github repo:
https://github.com/contiki-os/contiki/wiki
## External sensors

### Pulse sensor
![](20230216155951.png)  

6 pulse sensors are used to measure heart beat rate. Each has one LED and one light sensor on it, whenever the sensor is connect to the board through wire cable, the LED is on, the light sensor can measure reflected light intensity come from the LED to the attached surface. As a result, pulses with small amplitudes can be detected and tracked. In this case, it is attached on human fingers, and heart beat rate can be measured.



### Humidity and Temperature sensor

![](20230216160252.png)  

To measure temperature and humidity of environment, an embedded Humidity/Temperature sensor is implemented. 
Two ports will generate a voltage corresponding to two environment metrics respectively. An ADC convert the analog voltage to 12 bit binary number, which is called "voltage ratio" - r. Then the temperature and humidity can be obtained buy functions of r given by datasheet.

### Wind speed sensor

![](20230216185621.png)  

![](20230216185746.png)  

The wind speed sensor consists of an anemometer and a cable for data transmission. 
Axis of the wind wheel, which plays as a switch in the circuit opening and closing according to the rotation angle. It will close twice each rotation round. When the switch is closed, GPIO pin is set to 5V, in contrast, when it opens, GPIO is set to GND, which is 0V.
Therefore, during a fixed time period, the more times upper edges from low to high are viewed, the higher the wind speed is measured. Wind speed can be obtained by a function in datasheet.

# 2. Network and Rounting Technique

We implemented our network based on Low-energy adaptive clustering hierarchy (LEACH) proposed by Heinzelman et al in 2000, and work done by Lewandowski et al in 2019.

http://www.njavaid.com/LEACH.pdf

https://www.mdpi.com/1424-8220/19/19/4060 

Overall, the distributed sensors node will self-organized into local cluster, each cluster will elect a cluster head, the cluster head will aggregate data from all cluster members and send to gateway or back to other cluster heads in case needs multi-hop transition.
The following features are provided:
* Self-Organisation:  The core idea of clustering is to form individual local networks. During the process of establishing networks, the nodes can completely decentralized contruct or join cluster. The nodes only take care about local cluster and conduct hierarchically between inter and intra cluster communication, this makes the protocol has strong scalability. 
* Energy-efficiency: Since most of the time the nodes communicate inside the cluster, it can reduce the power of transmitting. The cluster heads are in charge of mantaining the clusters and transmit data from cluster members to gateway, so the cluster members sleeps most of the time. Also, switching cluster head helps balancing energy comsumption between nodes, that could make lifetime of whole network prolonged.
* Robustness: With only considering the local network, clustering protocol is able to handle topology change inside network. All the topology changes can be viewed as nodes leaving and joining cluster. As long as the cluster can detect and deal with topology change, the whole network remains its functionality 
* Additional Functions: An emergency alarm is added to call for help when needed. Also, there is an improvement on calculating the time of a node taking role of being a cluster head, it helps with energy efficientcy


# 3. Gateway and GUI

## Gateway
The gateway node in the network is a central one which is
in charge of collecting data from every sensor node and
contacting with GUI on PC. Its job can be divided into two
parts:
* Recognize and process incoming packets. According to
the network protocol, the gateway node is able to scan
every input packet and determine the source of each.
* Based on the reviewed source, process and transmit data
into GUI for displaying. The processed data streams
should obey a distinct protocol between the node and
GUI.

Meanwhile, the gateway node is connected to the GUI by
a wire-based link, normally a cable inserted to a USB port on
PC. UART is implemented for serial communication between
the node and GUI, adding the error correction to ensure a
successful transmission. In the programming domain, the
node program only needs to simply use printf function to send
processed data steams towards GUI.
 ## GUI

In the GUI designed by QT. To get the software for Ubuntu/Debian computer, simply run these command:

```
sudo apt-get install build-essential
sudo apt-get install qtcreator
sudo apt-get install qt5-default
sudo apt-get install mesa-common-dev
```
An available USB port can be
chosen at first as the input port. All the incoming data will be
displayed in “Mote Output” block. Then, QT starts to analyse
the node information recorded in each packet and updates the
topology simultaneously. In the topology graph, the central
blue node is the gateway node, and the two red circles
represent temperature/humidity sensor and windspeed sensor
respectively. Six remaining white circles indicate six mobile
heart rate sensor and the colors of them show the connection
state as well as health condition periodically. When any
emergency button is pressed, the corresponding user in the topology graph will be noted as “!!!” to call supervisors for
help

![](20230217144407.png)  

Additionally, three display boxes upwards correspond to
three types of environmental factors. When relative packets
are received, the latest data will be uploaded to the boxes.
Furthermore, the colors of these boxes can be changed. Data
will be compared with predefined thresholds to determine
whether it is appropriate for outdoor activities. If the element
value is proper, the box will be painted in green. When an
inappropriate value is indicated, the box will turn red to warn
users of going outside.