# TinyPaws - TinyML Dog Bark Detection Project

## Overview
TinyPaws is a TinyML (Tiny Machine Learning) project developed for the "Advanced Topics In Signal & Image Processing" course at the University of Washington's graduate computing department. The project focuses on real-time dog bark detection using an Arduino microcontroller. Once a bark is detected, TinyPaws emits a sound, shines a light, and sends a push notification to the user's device, providing an alert for potential intruders or disturbances.

## Demo
https://github.com/ZacharyDavidSaunders/TinyPaws/assets/29029748/17b99695-ccc8-43f9-a272-94d18c31973f

## Features
- Real-time dog bark detection using TinyML on an Arduino.
- Audio feedback: emits a sound upon detecting a bark.
- Visual feedback: shines a light upon detecting a bark.
- Notification system: sends a push notification to the user's device.

## Technologies Used
- Arduino: Used as the microcontroller platform for running the TinyML model and controlling the hardware components. The Arduino Nano 33 BLE Sense Lite was used for this project. 
- TensorFlow Lite for Microcontrollers: Used for deploying the machine learning model on the Arduino.
- Arduino IDE: Integrated Development Environment for writing and uploading code to the Arduino board.
- Python: Used for preprocessing audio data and training the machine learning model.
- Push Notification Service: Utilized for sending notifications to the user's device.

## Installation
1. Clone the TinyPaws repository to your local machine.

`git clone https://github.com/ZacharyDavidSaunders/TinyPaws`

2. Install the necessary Arduino libraries.
3. Upload the Arduino sketch to your Arduino board. Specifically, go to TinyPaws/BarkNet_inferencing/examples/nano_ble33_sense/nano_ble33_sense_microphone_continuous/nano_ble33_sense_microphone_continuous.ino and upload to the Arduino. 
4. Set up the necessary dependencies for the push notification service.
5. Run the Python scripts for preprocessing audio data and training the model.
6. Create a pushover account, download the mobile app, and add your information to ComputerBluetooth.py 

## Usage
1. Power on the Arduino board.
2. Issue the 'detect' command on the serial monitor to start bark detection.
3. Choose one of the available detection modes: p, l, or b. 
4. Play a dog bark sound or simulate a bark. Once a dog bark is detected, TinyPaws will emit a sound, shine a light, and send a push notification to the user's device.
5. Monitor the push notification on your device for alerts about potential disturbances.

## Contributors
- Bassam Halabiya
- Daniel Mohaghegh
- Pinxuan Lu
- Zachary Saunders

## Acknowledgements
Special thanks to Professor Dinuka Sahabandu for supervising this project and providing valuable insights and guidance throughout its development.

