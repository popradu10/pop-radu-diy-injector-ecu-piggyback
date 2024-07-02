## Pop Radu - DIY Injector ECU Piggyback 

I start created my own piggyback module that will delay closing the injector for my EFI moped.
DIY piggyback solution on one injector for an EFI engine running with an existing ECU.

#### It does this by:

<img align="right" src="/docs/barton-volcano-50cc.jpg" width="330" />

1. Taking the injector input from the **ECU** using a EV1 male and female connector and a **PC817 Optocoupler**
2. Monitoring the opening and closing of the injector using **Arduino ESP32 CH340** board
3. Delay closing the injector, the delay is computed proportional using **Arduino ESP32 CH340** and the [arduino code from here.](pop-radu-diy-injector-ecu-piggyback/pop-radu-diy-injector-ecu-piggyback.ino) Using the esp32 espressif library v2.x.x .
4. The injector is then closed and open using **F5305S Power Mosfet Module** that can react very quickly to changes.
5. The user can select to apply a delay of 25%, 35% or 45% 

#### Preconditions:

1. Increase engine capacity. For e.g. increase 139fmb engine from 50cc to 72cc.
2. Upgrate your exhaust. For e.g. remove the catalytic converter from the exhaust and obstruct the lambda sensor entry.
3. Unlocked your ECU engine controller. For e.g. unrestricted ECU engine controller for Barton Volcano Euro 5.

#### Examples of the piggyback module in action:

<img src="/docs/Injector-duty-cycle-45-piggyback-setup-june-2024.png" height="400">

Based on the [data collected while running for 35 minutes.](/docs/serial-data-june-2024.csv)

### Youtube video
[![My video](https://i.ytimg.com/vi/03AhqZIt5jw/0.jpg)](https://youtu.be/03AhqZIt5jw "My Video")

### EV1 male and female connector to piggyback the existing Injector signal comming from ECU

<img src="/docs/EV1%20male%20female%20connector.jpg" width="400">

### Capturing on/off signal
Using a PC817 2-Channel Optocoupler Isolation Board you are able to capture the 12v that comes to the injector

<img src="/docs/pc817-optocoupler.jpg" width="400">

Details:

* Driving Port Signal Voltage: 3.6-24V
* Output Port Voltage Port: 3.6-30V

### NODEMCU Arduino ESP32 CH340

The NODEMCU Arduino ESP32 CH340 is a development board based on the ESP32 microcontroller.

<img src="/docs/NodeMCU-ESP32.png" width="400">

Specifications:

* Core: Dual-core Tensilica LX6 microprocessor
* Clock Speed: Up to 240 MHz
* RAM: 520 KB SRAM
* Flash Memory: 4 MB (varies by model)
* Wireless: Integrated 802.11 b/g/n Wi-Fi and Bluetooth 4.2 (including BLE)

### F5305S Power Mosfet Module

Features:

* In the FET module, the input and output are completely isolated from each other.
* Signal trigger terminal: digital high and low level. Can be connected to the microcontroller IO port, PLC interface, DC power supply, etc.
* The output can control high-power equipment, motors, bulbs, LED strips, DC motors, micro pumps, solenoid valves, etc.
* Output data can control powerful devices: motor, light bulb, light and strips, DC motor, small pumps, solenoid valve.

<img src="/docs/mosfet.jpg" width="400">

Specifications:

* Input Voltage (V): 3 ~ 20
* Input Current: 5mA
* Output Voltage(V): 5 ~ 35

### Increase cylinder capacity kit 139fmb 72cc

Increasing the cylinder capacity of a 139FMB engine from its stock displacement (typically 50cc) to 72cc involves using a big bore kit.

<img src="/docs/139fmb-72cc-kit.png" width="400">

Components of a Big Bore Kit:

* Cylinder: A larger diameter cylinder to accommodate the increased bore.
* Piston: A piston that matches the new cylinder bore size.
* Piston Rings: Corresponding rings for the new piston.
* Gaskets: New head and base gaskets to ensure a proper seal.

### Exhaust without catalytic converter

Upgrading your exhaust system to one without a catalytic converter can have several implications, in terms of performance. 

<img src="/docs/JUNAK-BARTON-exhaust.jpg" width="400">

Performance Benefits:
* Increased Power. Removing the catalytic converter reduces back pressure, allowing exhaust gases to flow more freely. This can lead to a noticeable increase in horsepower and torque.
* Improved Throttle Response. The engine can expel exhaust gases more efficiently, improving throttle response and acceleration.
* Enhanced Sound. The exhaust note is typically louder and more aggressive without a catalytic converter.

### Unlock ECU Junak/Barton 50cc

Unlocking an ECU (Engine Control Unit) typically refers to modifying or reprogramming it to remove restrictions placed by the manufacturer.

<img src="/docs/unlock-ecu-junak-barton.jpg" width="400">

Ensure your vehicle’s ECU can be unlocked and that there are compatible tuning solutions available.
Regularly monitor engine parameters to prevent overheating, detonation, or other issues.

### Arduiono IDE with esp32 espressif library v2.x.x

Set up the Arduino IDE with the ESP32 Espressif library version 2.x.x .

<img src="/docs/arduino-idee-esp32 v2.x.jpg" width="400">

You should have the Arduino IDE set up with the ESP32 Espressif library version 2.x.x successfully.
Writing the code to your ESP32 Board parameters:
* Port: Select the COM port to which your ESP32 is connected.
* Flash Frequency: 80 MHz
* Upload Speed: 921600 (you can adjust this based on your setup)
