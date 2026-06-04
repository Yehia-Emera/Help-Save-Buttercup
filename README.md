# HELP. SAVE BUTTERCUP! 🌸🫧🟢
**Automated Ball Sorting & Shooting Recycling System**

This repository contains the mechanical, electrical, and software files for our MCT231 Basics of Mechatronics final project at the Faculty of Engineering, Ain Shams University (UG2023).

## 📌 Project Overview
"Help. Save Buttercup!" is a closed-loop mechatronic system that autonomously receives, color-sorts, shoots, and recycles balls through a continuous cycle. The system integrates servo actuation, stepper motor control, IR sensing, color detection, and Bluetooth communication, all coordinated across dual Arduino Uno controllers powered by a single shared PSU.

## 🎮 The Game Theme
To make the system interactive, we integrated a Powerpuff Girls game loop. Mojo Jojo has kidnapped Buttercup, and players must help Blossom and Bubbles save her before a 2-minute timer expires. 
* Players shoot sorted balls into designated targets.
* Questions and choices appear via a Bluetooth terminal.
* **Blossom (Red Ball):** Multiplication questions. Awards 2 HP damage to Mojo Jojo on a correct answer.
* **Bubbles (Blue Ball):** Addition questions. Awards 1 HP damage on a correct answer.
* **Goal:** Deplete Mojo Jojo's 10 HP bar to win!

## ⚙️ System Architecture
The repository is organized by engineering discipline rather than subsystem, as components are shared across the dual microcontrollers.

### Hardware Specifications
* **Controllers:** 2x Arduino Uno
* **Color Detection:** TCS3200 Color Sensor
* **Sorting Actuator:** SG90 Servo (with 15g PLA+ custom claw)
* **Turnstile:** PLA+ (40mm radius, 120° sweep)
* **Shooting Base:** Stepper Motor (45° rotation)
* **Lifting Mechanism:** Servo arm (90° vertical lift over 40cm)
* **Sensors:** 5x IR Sensors (sorter, lifter x2, box x2)
* **Comms & UI:** HC-06 Bluetooth Module, 16x2 I2C LCD

## 📂 Repository Structure
* `/CAD_Assembly` - Autodesk Inventor files and 3D printable STLs (turnstile, claw, base).
* `/Code` - Arduino `.ino` files for both the master and slave controllers.
* `/Electronics` - Circuit wiring schematics and pinout diagrams.
* `/Documentation` - System flowchart, block diagram, and the final A0 presentation poster.

## 👥 Team Members
* Yehia Mohammed Fawzy (24P0322)
* Mowahed Mohamed Mohamed (24P0302)
* Ahmed Mohammed Youssef (24P0079)
* Abdulrahman Khaled Ahmed (24P0465)
* Gana Khalid Abdelwahab (24P0251)
