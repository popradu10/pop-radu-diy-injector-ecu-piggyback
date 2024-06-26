## Pop Radu - DIY Injector ECU Piggyback 

I start created my own piggyback module that will delay closing the injector for my EFI moped.
DIY piggyback solution on one injector for an EFI engine running with an existing ECU.

It does this by:

1. Taking the injector input from the **ECU** using a EV1 male and female connector and a **PC817 Optocoupler**
2. Monitoring the opening and closing of the injector using **Arduino ESP32 CH340** board
3. Delay closing the injector, the delay is computed proportional using **Arduino ESP32 CH340** and the [arduino code from here.](/src/pop-radu-diy-injector-ecu-piggyback.ino)
4. The injector is then closed and open using **F5305S Power Mosfet Module** that can react very quickly to changes.
5. The user can select to apply a delay of 25%, 35% or 45% 

Preconditions:

1. Increase engine capacity. For e.g. increase 139fmb engine from 50cc to 72cc.
2. Upgrate your exhaust. For e.g. remove the catalytic converter from the exhaust.
3. Unlocked your ECU engine controller. For e.g. unrestricted ECU engine controller for Barton Volcano Euro 5.

### Piggyback the existing Injector signal comming from ECU

<img src="/docs/EV1%20male%20female%20connector.jpg" width="400">

### Capturing on/off signal
Using a PC817 2-Channel Optocoupler Isolation Board you are able to capture the 12v that comes to the injector

<img src="/docs/pc817-optocoupler.jpg" width="400">

Details:

* Driving Port Signal Voltage: 3.6-24V
* Output Port Voltage Port: 3.6-30V

### NODEMCU Arduino ESP32 CH340

<img src="/docs/NodeMCU-ESP32.png" width="400">

### F5305S Power Mosfet Module

Features:

* In the FET module, the input and output are completely isolated from each other.
* Signal trigger terminal: digital high and low level. Can be connected to the microcontroller IO port, PLC interface, DC power supply, etc.
* The output can control high-power equipment, motors, bulbs, LED strips, DC motors, micro pumps, solenoid valves, etc.
* Using the original F5305S power FET, you can input PWM, control motor speed, the brightness of the lamp, etc.
* Trigger signal: digital high/weak signal, can be connected to IO microcontroller, plc interface, DC power supply, and so on.
* Output data can control powerful devices: motor, light bulb, light and strips, DC motor, small pumps, solenoid valve.

Specifications:

* Input Voltage (V): 3 ~ 20
* Input Current: 5mA
* Output Voltage(V): 5 ~ 35

<img src="/docs/mosfet.jpg" width="400">

### Increase cylinder capacity kit 139fmb 72cc

<img src="/docs/139fmb-72cc-kit.png" width="400">

### Exhaust without catalytic converter

<img src="/docs/JUNAK-BARTON-exhaust.jpg" width="400">

### Unlock ECU Junak/Barton 50cc

<img src="/docs/unlock-ecu-junak-barton.jpg" width="400">

