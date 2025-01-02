# ESP32 Robots

## Humanoid Robot Design

This system uses parallel task management to independently control the upper and lower body movements, ensuring synchronized and dynamic motion. Servo movements are smoothed with step delays, incorporating acceleration and deceleration adjustments for more natural motion.


<p>
<img src="https://github.com/user-attachments/assets/33eeecd6-4677-4fbd-a872-94bbbf2eefce" alt="Humanoid" width="43%"/>
<img src="https://github.com/user-attachments/assets/a473b7e6-6665-4e33-9bf3-f1a85def5217" alt="Humanoid" width="43%"/>
</p>

Version 2.1 (left) Version 1.0 (right)

**Components:** 
- ESP32-WROOM-32D
- x12 MG90S Metal Gear Micro Servos
- Adafruit 16-Channel 12-bit PWM/Servo Driver (I2C)
- MPU-6050 Accelerometer and Gyroscope (I2C)
- 6V Power Supply for Servo Driver
- 3.3V Power Supply for Servo Driver logic, MPU-6050 logic, and ESP32

**Key Features:**
- **Parallel Control:** Independently manages upper and lower body movements.
- **Smooth Motion:** Servo movements are gradually slowed and smoothed with adjustable delays for acceleration and deceleration.
- **Modular Design:** The object-oriented programming (OOP) structure abstracts parallel servo steps and organizes servos into logical body part groups (e.g., leg, legs, arm, arms), simplifying control and improving code maintainability.


## RC RoboCar Design


**Key Features:**
- **Wireless Communication:** Utilizes ESP-NOW for low-latency, reliable communication between the remote control and the robot.
- **Skid Steering Control:** Implements skid-steering logic using data from dual joysticks on the remote control.


<img src="https://github.com/user-attachments/assets/c6a26b6b-00a3-4c9e-8731-a4ce020789a2" alt="Robo Car gif"/>
<img src="https://github.com/user-attachments/assets/a2301321-068f-44e5-b97a-933604c310ef" alt="Remote Control" width="260px"/>

RoboCar (left) and Remote Control (right)

**Components:** 
- x2 ESP32-WROOM-32D
- L298N Dual H-Bridge Motor Driver
- x2 DC Gearbox Motor "TT Motor"
- x2 Dual-Axis Joystick Module
- 3.3V, 5V, and 9V Power Supplies


<p>
<img src="https://github.com/user-attachments/assets/fd1674b6-fb37-43bd-a88e-9d3303b20929" alt="Robo Car Schematic" width="350px"/> 
&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp
<img src="https://github.com/user-attachments/assets/8af3e560-f5a4-484a-88ce-18e1c9abfe71" alt="Controller Schematic" width="370px"/>

</p>

RoboCar Schematic (left) 
Remote Control Schematic (right)

<!---
## Quadruped
-->


## IoT Design
This IoT system features a centralized architecture, where a single server orchestrates communication 
and control across multiple ESP32 clients. It leverages Wi-Fi for seamless connectivity and WebSocket 
for real-time, bidirectional communication, ensuring low latency and efficient command execution. 
Additionally, it supports device-to-device communication via Bluetooth Low Energy (BLE), providing flexible connectivity options.


- **Web Interface Clients** initiate commands via a user-friendly interface.
- **ESP32 Server** acts as a central controller, hosting the web interface and relaying commands (JSON) to the appropriate ESP32 clients.
- **ESP32 Clients** execute hardware-specific actions based on commands received from the server.

<img src="https://github.com/user-attachments/assets/73430e0a-76c6-429e-8c54-d63966fa9557" alt="IoT System" width="65%"/>

<!---
**Web Interface:**
<img src="" alt="Web Interface" width="29%"/>
-->

