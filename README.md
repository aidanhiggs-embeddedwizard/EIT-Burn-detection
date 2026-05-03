# EIT-Burn-detection
Evaluating the automatic detection of diathermy pad site burns by utilising electrical impedance tomography

# The device (iEITBacon)
iEITBacon (pronounced "i eat bacon"), is a custom EIT device that uses the following:
- Teensy 4.1 as the main controller
- AD5933 Waveform generator
- Howland current source (VCCS)
  -  That injects a stimulation current into a test sample
-  2 x Current injection Multiplexers
  - ADG732
-  2 x Voltage Measurement Multiplexer 
  - ADG732
- SMA connectors for Current injection and Voltage measurement
- A series of operational amplifiers
  - Amplify the signal
  - Filter the signal
  - Apply a DC offset to the signal

# Matlab code
## Saline tank tests

## Pork tests

# The Burn wound device (Baconator)
The PID-controlled Baconator has the following components
### Electronics
- Arduino Nano
- MAX6675 Thermocouple
- Mosfet (AOD4184A)
- 16x2i2c lcd screen
- 4 x 12VDC Heater cartridges
- 12DC power supply at 350W
- 2 x cooling fans
- Power switch
- RGB led

In order to determine the PID values of the controller, the open-loop response of the heater block had to be measured. The baconator device was set to a 70% duty cycle and the temperature measured for 1 hour.
Using MATLAB and SIMULINK, the open-loop response was plotted, and the transfer function was derived.
G(s) = (K*e^-st) / (\tau s+1)
was used to calculate the correct transfer function
  
### Hardware
- Drill press
- Aluminium heater block
