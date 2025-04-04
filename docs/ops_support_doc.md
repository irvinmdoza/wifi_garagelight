# Wifi Garage Stoplight - Arduino User Support

This documentation is intended for the end user of the Wifi Garage Stoplight devices.
The system is a wifi stoplight built to help find the optimal parking distance within your garage.  It utilizes a distance sensor to light up an indicator LED based on how close you are to the wall of your garage.  It operates through a series of sent from multiple devices.

### Operating to the Device ###

Refer them to the ![End User Documentation](./end_user_doc.md) for further instructions regarding the system's operation.

### Troubleshooting Issues ###

Below is a list of common issues and instructions on how to respond.

### ISSUE: Cannot connect devices to the Internet ###

Description: The user reports that the devices are not accessible over the internet

Troubleshooting:
1. Verify that the devices are powered: Ask the user to verify that the devices are connected to a power source.  This can be done by inspecting the devices and verifying that the built in LED indicator light is either flashing or on.  If the LED light is off, this means that the devices are not being powered. Ask the user to power the devices by either plugging them into a power source or using a different power source if it already connected to one.

2. Verify that the devices are connected to the wifi network: Ask the user whether or not the built in LED light on the devices is blinking or constant.  A blinking LED on either device indicates that the device is attempting to connect to a wifi network.  Normally this process takes only a few seconds, if the device continues to blink, verify whether the user went through the WiFi connection instructions located on the End User guide under the "Setting up the Device" section.  walk them through the process if necessary.

4. If all devices are connected but are still not functioning as expected, notify an engineer.  The engineer will have to go manually inspect the components and manually troubleshoot the sensor, door and led devices.