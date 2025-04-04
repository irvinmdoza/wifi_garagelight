# Wifi Garage Stoplight - Arduino Engineer Support

This documentation is intended for the support engineer to assist enhancing and troubleshooting the Wifi Garage Stoplight devices.
The system is a wifi stoplight built to help find the optimal parking distance within your garage.  It utilizes a distance sensor to light up an indicator LED based on how close you are to the wall of your garage.  It operates through a series of sent from multiple devices.

## Getting Started ##

Reach out to the previous engineer to receive access rights to the github repository https://github.com/irvinmdoza/wifi_garagelight

Once you have access, you can download the repository and make modifications where/if necessary.  Currently all editing is done through the Arduino IDE, with the Wemos D1 mini libraries added.

## System Operation ##

The system will automatically start as long as the devices are powered.  See ![End User Documentation](./end_user_doc.md) for details on system operation.
In order to perform in-depth troubleshooting you must be able to physically connect the devices to a computer running the Arduino IDE.  The device will send status messages to the Serial Monitor for monitoring and troubleshooting.

## Directory Structure ##

The following describes the directory structure with notes on important locations

![Directory](../img/directory.PNG)

1. /code: Contains the code for the application, specifically the wifi_light.ino, wifi_distancesensor.ino and the wifi_reed.ino files.  These files host the stoplight functionality, the distance sensor functionality and the garage door functionality respectively.

2. /docs: Contains the documentation for the application, including this guide, end user and operations center guides as well as the primary design document.

3. /img: Contains images referenced in the documentation.

4. README.md: File that contains a general directory for the project.


## Assembly and Configuration ##
This device has the following technical specifications
1. The devices hosting the applications are three Wemos D1 Mini
2. The stoplight device, referred to as the `LED device`, receives messages via MQTT from the `Sensor device` based on the distance sensed by it.  The specific distance must be configured by an engineer, at the time of writing.
3. The distance sensor device, referred to simply as the `Sensor device`, receives signals from its attached sensor and sends mqtt messages to the indicator device based on the distance read.
4. The reed switch device, referred to as the `Door device`, verifies whether the garage door of the user is closed or open. If the `Door device` is sensing a closed door, both the `Sensor device` and the `LED device` will cease operation until the garage door is sensed as open again.

The device assembly consists of
- 3 Wemos D1 Mini
- 1 HC-SR04 Ultrasonic Sensor
- 1 GDKG PS-3150 Reed Switch
- 15 Jumper Wires
- 3 LEDs, one of each of the following colors: Red, Green, Yellow
- 4 75 Ohm resistors

They are connected as shown in the following schematics:

![Circuit Breadboard for Wifi Stoplight](../img/wd1m_stoplight_diagram_v2.png)

![Circuit Breadboard for Wifi Distance Sensor](../img/wd1m_distancesensor_diagram.png)

![Circuit Breadboard for Wifi Reed Switch](../img/wd1m_reed_switch.png)

## State Machine

The following state machines describe the behavior of the devices.
The indicator changes state based on the HTTP request it receives.
The sensor sends HTTP requests to the indicator based on the distance it reads.

![Indicator State Machine](../img/indicator_statemachine.PNG)
![Sensor State Machine](../img/sensor_statemachine.PNG)
![Door State Machine](../img/door_statemachine.PNG)

### Troubleshooting Issues ###

The following are troubleshooting steps for issues that the Operations Support center might reach out about.

#### The Host is Unavailable ####

If the host is unavailable, verify that the devices are powered.  If they are powered, verify that the blue LED on the devices are on and not blinking.  If they are not, this indicates that the devices are not connected to the Wifi network.  Verify that the network defined in the code is accurate as well as the credentials.  

If the credentials need to be updated, make the changes to either the wifi_light.ino file or the wifi_distancesensor.ino file depending on which of them was failing and then upload the updates to the devices.