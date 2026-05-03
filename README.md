# EIT-Burn-detection
Evaluating the automatic detection of diathermy pad site burns by utilising electrical impedance tomography

# The device (iEITBacon)

iEITBacon (pronounced "i eat bacon") is a custom EIT device that uses the following:
- Teensy 4.1 as the main controller
- AD9833 Waveform generator
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

  The device injects a 650uA current at 25kHz into a test sample.
  The Teensy samples the voltage at a rate of 500kHz; to achieve this, DMA was used
  The sampled voltages are then demodulated using IQ demodulation and stored for EIT experiments
  
### Program dependencies
To use the Teensy, I installed [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) that was created by the legendary [Paul Stoffregen](https://github.com/paulstoffregen).

I give credit to [Rob Tillaart](https://github.com/RobTillaart) for using some of his libraries in my code
- Waveform generator [AD9833](https://github.com/RobTillaart/AD9833)
- I modified the [ADG732](https://github.com/RobTillaart/ADG732) library to work for my EIT setup that uses 16 channels
I also used the [Neo pixel](https://github.com/adafruit/Adafruit_NeoPixel) by Adafruit


# Matlab code
### Program dependencies

## Saline tank tests
A saline tank phantom was used to test whether the EIT device works
### Details about experimental setup
- Number electrodes = 8
- Stimulation pattern = adjacent 
- Measurement pattern => adjacent
- Stimmulation frequency = 25kHz
- tank diameter = 155mm
- Solution volume = 100ml or 250ml
- Saline solution = 36.7g/L
  
## Pork tests
To prove that EIT can be used as an alternative method to monitor electro-surgical machine dispersion pads, precise burns were inflicted on the pork skin. 
EIT images were then reconstructed of the burn site
### Details about experimental setup
- Number electrodes = 8
- Stimulation pattern = adjacent 
- Measurement pattern => adjacent
- Stimmulation frequency = 25kHz
- Electrode ring diameter = 80mm
- Burn wound diameter = 20mm
- Sample size = 110mm x 110mm x 10mm square
- Burn time = 30s
- Burn temperature = 150 &deg;C

# Python code
The experiments were repeated using Python and pyEIT
### Program dependencies
An open-source Python framework for Electrical Impedance Tomography(EIT) [pyEIT](https://github.com/eitcom/pyEIT)


# The Burn wound device (Baconator)
The PID-controlled Baconator has the following components
### Electronics
- Arduino Nano
- MAX6675 Thermocouple
- MOSFET (AOD4184A)
- 16x2i2c LCD screen
- 4 x 12VDC Heater cartridges
- 12DC power supply at 350W
- 2 x cooling fans
- Power switch
- RGB LED

In order to determine the PID values of the controller, the open-loop response of the heater block had to be measured. The baconator device was set to a 70% duty cycle and the temperature measured for 1 hour.
Using MATLAB and SIMULINK, the open-loop response was plotted, and the transfer function was derived. 
The equation, $G(s) = (K*e^{-st}) / (\tau s+1)$ was used to calculate the correct transfer function
Using the SIMULINK PID Tuner, the PID values were chosen.
Brian Douglas made a [video](https://www.youtube.com/watch?v=Mbx5IMICS_Y&t=1337s) that helped me design the PID controller. So, check him out
### Hardware
- Drill press
- Aluminium heater block
