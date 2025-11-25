# ESPHID Ducky 8266

## Overview
**ESPHID Ducky 8266** is a Python-based automation tool inspired by the USB Rubber Ducky concept, designed to work with the ESP8266 platform.  
It allows remote execution of automated keyboard and mouse actions on a connected computer via a Flask web interface.

Created by **Dinesh Patra**, this project provides an easy way to trigger HID (Human Interface Device) scripts without directly connecting a traditional USB payload device.

---

## Features
- **Remote Control** – Send commands via HTTP to control keyboard/mouse actions.
- **Live Window Tracking** – Detect and log newly opened application windows.
- **Cross-Platform Automation** – Works with Windows, macOS, and Linux (PyAutoGUI).
- **CORS-Enabled API** – Interact with the server from web-based clients.
- **ESP8266 Integration** – Can be triggered over Wi-Fi for wireless payload delivery.

---

## Requirements
- **Python 3.7+**
- Required Python packages:
  - `flask`
  - `pyautogui`
  - `psutil`
  - `pygetwindow`

---
## Installation
1. Clone this repository:
   ```bash
   git clone https://github.com/navyaXdev/ESP8266_DUCKY.git
   cd ESP8266_DUCKY
---
## Target connection 
1. Run this to convert .exe file but use python version 3.10.10
  ```bash
  pyinstaller --noconsole --onefile main.py
