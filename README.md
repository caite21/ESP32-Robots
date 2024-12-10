# ESP32 Robots

## Robot Designs

### Humanoid Robot

Parts: ESP32-WROOM-32D, 14 MG90S Metal Gear Micro Servos, Adafruit 16-Channel 12-bit PWM/Servo Driver (I2C), MPU-6050 (I2C), 3.3V Power Source, and 6V Power Source.

Parallel tasks control the legs, arms, and torso.

<img src="https://github.com/user-attachments/assets/a473b7e6-6665-4e33-9bf3-f1a85def5217" alt="Humanoid" width="50%"/>


<!---

### RoboCar

### Quadruped

-->


## IoT Design

Users can control multiple ESP32 clients by clicking buttons on the web interface.
Web interface clients send commands in JSON to the server. The server forwards 
the message to the correct ESP32 client. The client receives the message and executes 
the command. 

Server.ino: Program for the ESP32 web server.

xClient.ino: Programs for ESP32 clients that automatically connect to the server.
