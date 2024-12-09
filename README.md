# ESP32 Robots

## Robot Designs

### Humanoid Robot


<!---

### RoboCar

### Quadruped

-->


## IoT Design

Users can control multiple ESP32 clients by clicking buttons on the web interface.
Web interface clients send JSON messages of commands to the server. The server forwards 
the message to the correct ESP32 client. The client receives the message and executes 
the command. 

Server.ino: Program for the ESP32 web server.

xClient.ino: Programs for ESP32 clients that automatically connect to the server.
