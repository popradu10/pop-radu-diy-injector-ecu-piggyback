## Pop Radu - DIY Injector ECU Piggyback 

I start created [my own piggyback module](#module) that will delay opening the injector for my EFI moped as I install a larger injector.
DIY piggyback solution on one injector for an EFI engine running with an existing ECU.

#### It does this by:

<img align="right" src="/docs/barton-volcano-50cc.jpg" width="330" />

1. Taking the injector input from the **ECU** using a EV1 male and female connector and a **PC817 Optocoupler**
2. Monitoring the opening and closing of the injector using **Arduino Micro Pro** board
3. Delay opening the injector, the delay is computed proportional using **Arduino Micro Pro** and the [arduino code from here.](diy-injector-ecu-piggyback/diy-injector-ecu-piggyback.ino) using the TimerOne library v1.1 for counting microseconds. 
4. The injector is then closed and open using **IRF520 Mosfet Driver Module** that can react very quickly to changes.
5. We can set the delay percentage in the arduino code. 

#### Preconditions:

1. Increase engine capacity. For e.g. increase 139fmb engine from 50cc to 72cc.
2. Upgrade the injector, bigger injector. For e.g. from a stock 2 hole injector to a 3 hole injector. 
3. Upgrade your exhaust. For e.g. remove the catalytic converter from the exhaust.
4. Unlocked your ECU engine controller. For e.g. get a ECU without rev limitation for Barton Volcano Euro 5.
5. Upgrade Front Pin tooth count.

#### DIY injector piggyback module:

<p name=module>
<img src="/docs/diy_injector_piggyback_a.jpg" width="800">
</p>
<p>
<img align="left" src="/docs/diy_injector_piggyback_b.jpg" height="200"> <img src="/docs/diy_injector_piggyback_c.jpg" height="200">
</p>

#### Examples of the piggyback module in action:

<img src="/docs/Injector-duty-cycle-45-piggyback-setup-june-2024.png" height="400">

Based on the [data collected while running for 35 minutes.](/docs/serial-data-june-2024.csv)

### Youtube video

[![My video](https://i.ytimg.com/vi/03AhqZIt5jw/0.jpg)](https://youtu.be/03AhqZIt5jw "My Video")

https://youtu.be/03AhqZIt5jw

### EV1 male and female connector to piggyback the existing Injector signal comming from ECU

<img src="/docs/EV1%20male%20female%20connector.jpg" width="400">

### Capturing on/off signal
Using a PC817 2-Channel Optocoupler Isolation Board you are able to capture the 12v that comes to the injector

<img src="/docs/pc817-optocoupler.jpg" width="400">

Details:

* Driving Port Signal Voltage: 3.6-24V
* Output Port Voltage Port: 3.6-30V

### Arduino Micro Pro

The Arduino Micro is a microcontroller board based on the ATmega32U4 microcontroller. It is compact and designed for embedded projects where small size is critical.

<img src="/docs/arduino_micro_board.png" width="400">

Specifications:

* Clock Speed: 16 MHz
* RAM: 2.5 KB
* Flash Memory: 32 KB
* Operating Voltage: 5V

### IRF520 Mosfet Driver Module

The IRF520 MOSFET Driver Module is a simple and widely used electronic component for switching high-power loads using a microcontroller or other low-voltage control systems. This module utilizes the IRF520 N-channel MOSFET, which can be controlled via a low-voltage signal to switch higher voltage and current loads.

<img src="/docs/IRF520_mosfet_module.jpg" width="400">

Specifications:

* Input Voltage (V): the module is designed to accept control signals from 3.3V or 5V logic, making it compatible with a wide range of microcontrollers like Arduino, Raspberry Pi, and other development boards.
* Output Voltage(V): typically up to 24V DC

The IRF520 MOSFET Driver Module is a useful tool for controlling high-power DC loads using a simple, low-power control signal. However, for more efficient or advanced applications, especially with logic-level microcontrollers, consider using MOSFETs better suited for low-voltage gate control.

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

### Upgrade front pinion from 12T to 14T or 16T

Upgrading the front sprocket (also known as the front pin or front chainring) from a 12-tooth (12T) to a 14-tooth (14T) on a motorcycle will affect several aspects of the bike's performance:

* Top Speed Increase: A larger front sprocket will result in a higher top speed. This is because each rotation of the front sprocket will move the chain further, resulting in more wheel rotations per engine rotation.
* Acceleration Decrease: Conversely, with a larger front sprocket, the motorcycle will have a slower acceleration. The engine will need to work harder to turn the wheel, reducing the bike's ability to accelerate quickly.
* Engine RPM: At any given speed, the engine RPM will be lower with a larger front sprocket. This can reduce engine wear and improve fuel efficiency.
* Torque: The torque at the rear wheel will decrease, as the mechanical advantage is reduced with a larger front sprocket.

<img src="/docs/pinion-428-14T.jpg" width="400">

### Arduiono IDE with TimeOut library version 1.1.1

Set up the Arduino IDE with the TimeOut library version 1.1.1:

<img src="/docs/arduino-idee-timeone.jpg" width="400">

You should have the Arduino IDE set up with the TimeOut library version 1.1.1 successfully.
Writing the code to your Arduino Micro Pro Board parameters:
* Port: Select the COM port to which your Arduino Micro Pro Board is connected.
