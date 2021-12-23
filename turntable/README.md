# File List
**2610_Subsonic.ino** = the code uploaded on the ESP32 (via Arduino IDE with the ESP add on) <br>
***Wing.vi***           = the LabView ***2017*** VI used in AE2610 to interface with the turntable and the load cell <br>
***FT5339.cal***        = the ATI Gamma (ID: FT5339) load cell calibration file loaded by Wing.VI to resolve logged voltages into forces/torques <br>
***subVIs***      = a folder containing the Sub VIs used by Wing.vi <br>

# Concept of operations
The ESP32 is responsible for taking a desired motion command from the user, over a serial interface, and executing that motion via the transmission of pulses to the stepper motor driver. To calculate this pulse train, the ESP32 uses the FastAccelStepper library. Meanwhile, the ESP32 outputs its current position, a motion flag, and its perception of time, over the serial interface. The ESP32 can be thought of as a self contained unit: it does not need LabView or any other fancy software for moving the stepper, it just needs to be sent a serial command constructed in the appropriate manner using a program such as PuTTY, RealTerm, or similar. The ESP32 also implements limit switches to ensure the turntable can't turn too far and damage something, as well as input command filtering to ensure that accelerations/velocities/positions beyond a certain limit can't be executed.<br>
 <br>
The Wing.vi is a graphical user interface that unifies communications with the ESP32 and with the National Instruments DAQ used to capture the 6 voltages outputted by the 6DOF ATI Gamma load cell. Since this is designed for AE2610 it is hard-coded to send the relevant command to the ESP32 which in turn executes the special traverse function. Modifying the block diagram behind the VI can change the command being sent to the ESP32 to execute custom commands. Logging of the load cell data occurs when the special ESP32 motion flag "LOGGING" is being sent, which is also logic that be changed to suit your needs.
 <br>
 # ESP32 Serial Command Structure and Motion Types
 TBD
