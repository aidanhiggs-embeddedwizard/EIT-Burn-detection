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

<img width="2160" height="1770" alt="3D_PCB1_2026-05-03" src="https://github.com/user-attachments/assets/1d9c5d26-dadf-4791-adb0-7a676de41071" />

  
### Program dependencies
To use the Teensy, I installed [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) that was created by the legendary [Paul Stoffregen](https://github.com/paulstoffregen).

I give credit to [Rob Tillaart](https://github.com/RobTillaart) for using some of his libraries in my code
- Waveform generator [AD9833](https://github.com/RobTillaart/AD9833)
- I modified the [ADG732](https://github.com/RobTillaart/ADG732) library to work for my EIT setup that uses 16 channels
I also used the [Neo pixel](https://github.com/adafruit/Adafruit_NeoPixel) by Adafruit


# Matlab code
### Program dependencies
- MATLAB
- [EIDORS](https://eidors3d.sourceforge.net/)
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

### Test setup

<img width="522" height="1160" alt="image" src="https://github.com/user-attachments/assets/c682f584-99ef-4e7c-affa-1aa886fb1a1b" />

### Reconstructed result 

<img width="1477" height="828" alt="T12-chan8-Tank-CC-F=25kHz-V=250ml-d=155mm" src="https://github.com/user-attachments/assets/59b8c825-ce2b-43e7-b053-ec15142d6174" />

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

### Test setup

<img width="4000" height="3000" alt="electrode_ring_altview_2026-04-16" src="https://github.com/user-attachments/assets/c2219919-050a-4493-93c3-b476e50f9d0f" />

### Reconstructed result 

<img width="1289" height="836" alt="Prior vs Hyperparameter Grid" src="https://github.com/user-attachments/assets/990d9ffa-858b-4eb2-ad65-30f31493b225" />

# Python code
The experiments were repeated using Python and pyEIT
## Program dependencies
An open-source Python framework for Electrical Impedance Tomography(EIT) [pyEIT](https://github.com/eitcom/pyEIT)

## Python + pyEIT Reconstructed result 
**Test setup:**
- Tank with a diameter of 155mm
- Saline 250ml 
- 8 electrodes
- A conductive object (20mm) is placed at the centre of the tank
  
## pyEIT reconstructed results 

### Jacobian solver

<img width="1200" height="900" alt="image" src="https://github.com/user-attachments/assets/75c04a85-fafe-465d-905c-da5f1e17a99c" />

### GREIT solver

<img width="1200" height="900" alt="image" src="https://github.com/user-attachments/assets/8e148608-dde5-469e-aa6c-c1bbb72a5653" />


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

<img width="3000" height="4000" alt="IMG_5541" src="https://github.com/user-attachments/assets/866c426c-a875-4345-9e7b-0757a033780d" />

