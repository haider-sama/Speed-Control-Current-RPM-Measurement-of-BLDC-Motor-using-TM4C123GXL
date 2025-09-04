# 🌀 BLDC Motor Speed Control & Monitoring using TM4C123GXL

## 📖 Introduction
This project implements **speed control, current monitoring, and RPM measurement of a BLDC motor** using the **TM4C123GXL (Tiva C LaunchPad)** microcontroller.

The system is capable of:  
- ⚡ Speed control using PWM  
- 📈 Current measurement via ACS712/INA219 sensors  
- 🔄 RPM calculation using Hall-effect sensors  
- 💻 Displaying motor parameters through UART or serial monitor  

This project demonstrates **microcontroller-based motor control**, closed-loop feedback, and real-time monitoring for industrial or robotics applications.

---

## 🎯 Objectives
- Implement **speed control** for a BLDC motor.  
- Measure and monitor **current drawn** by the motor.  
- Calculate **RPM** using Hall sensor feedback.  
- Display motor parameters in real-time via **UART**.  

---

## 🛠️ Components Used
- TM4C123GXL (Tiva C LaunchPad – TM4C123GH6PM)  
- BLDC Motor (A2212)  
- ESC (30A) for motor speed & direction control  
- Hall-effect sensors for rotor position & RPM  
- Current Sensor (ACS712 / INA219)  
- 12V / 24V DC Power Supply  
- 5V DC Supply (optional if USB powered)  
- Resistors, capacitors, diodes for signal conditioning & protection  
- Pull-up resistors for Hall sensors  
- Potentiometer for manual speed control  
- Oscilloscope and PC/Laptop for debugging & programming  

---

## 📂 Project Structure

📁 uVision_Keil_Project_11/  
<br />
├── Objects/ # Compiled object files  
<br />
├── RTE/ # Run-time environment files  
<br />
├── tempCodeRunnerFile.py # Temporary file inside Keil listings  
<br />
├── MOTOR_CTRL_COMBINED_CODE.uvprojx  
<br />
├── MOTOR_CTRL_COMBINED_CODE.uvoptx  
<br />
└── MOTOR_CTRL_COMBINED_CODE.uvguix.ApriZon  
<br />
└── main.c  # Main code for PWM, speed, RPM, current measurement  

📁 Project Root/  
<br />
├── graph.py # Graphing / plotting motor parameters  
<br />
├── DEMO_1.mp4 # Demo video of motor operation  
<br />
└── Smester_Project_Report_11.pdf # Detailed project report  

---

## 🚀 How to Run

### Hardware Setup
- Connect the **BLDC motor, ESC, sensors, and power supply** as per schematic.  
- Ensure all pull-up resistors and protection components are installed.  

### Software Setup
- Open `MOTOR_CTRL_COMBINED_CODE.uvprojx` in **Keil uVision IDE**.  
- Build and flash the program to **TM4C123GXL LaunchPad**.  

### Monitoring
- Open **UART serial monitor** to view RPM and current readings.  
- Optionally, run `graph.py` to plot motor performance.  

---

## 📌 Reference
- TM4C123GXL Datasheet and Peripheral Guide  
- BLDC Motor Control Theory and PID Control  
- ACS712 / INA219 Sensor Datasheets  
- Detailed implementation provided in **Smester_Project_Report_11.pdf**  
