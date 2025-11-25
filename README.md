
# Multi-Mode ESP8266 Tool - Evil Twin & HID Simulator

**Author:** Dinesh Patra  
**Original Author:** WireBits (Evil Twin), Modified by Assistant (HID Simulator)

## 🔍 Overview

This is a comprehensive ESP8266-based tool that combines two powerful attack modes:
1. **Evil Twin Mode** - Create fake WiFi access points and perform deauthentication attacks
2. **HID Simulator Mode** - Remotely control a target computer through keyboard injection

The tool operates as a dual-mode device that can switch between WiFi attacks and HID (Human Interface Device) simulation, making it versatile for security testing and penetration testing scenarios.

## ⚡ Features

### Evil Twin Mode
- WiFi network scanning with detailed information (SSID, BSSID, Channel, RSSI)
- Fake AP creation mimicking selected networks
- Deauthentication attack capabilities
- Web-based phishing interface
- LED status indicators

### HID Simulator Mode
- Remote keyboard injection
- Web-based control interface
- Command preset system
- Target IP configuration
- Connection status monitoring
- Python backend for advanced command processing

## 🛠 Hardware Requirements

### Required Components
- **ESP8266 Development Board** (compatible with):
  - NodeMCU 1.0 (ESP-12E Module)
  - WEMOS D1 mini
  - Generic ESP8266 Module
  - DSTIKE Deauther
  - Maltronics Deauther
- **Micro USB Cable** for programming and power
- **Computer** for Arduino IDE and Python server

### Optional Components
- **External LED** (connected to GPIO2) for visual feedback
- **Battery Pack** for portable operation

## 💻 Software Requirements

### Arduino IDE Setup
1. **Arduino IDE** (version 1.8.19 or newer)
2. **ESP8266 Board Support**
3. **Spacehuhn Deauther Core** (recommended for enhanced features)

### Python Requirements
- **Python 3.7+**
- Required packages:
  ```
  flask
  pyautogui
  pygetwindow
  psutil
  ```

## 🚀 Installation

### Step 1: Arduino IDE Setup

1. **Install Arduino IDE** from [arduino.cc](https://www.arduino.cc/en/software)

2. **Add ESP8266 Board Support**:
   - Open Arduino IDE
   - Go to File → Preferences
   - Add this URL to Additional Board Manager URLs:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```

3. **Install Spacehuhn Deauther Core** (Recommended):
   - Go to Tools → Board → Boards Manager
   - Search for "deauther"
   - Install "Deauther ESP8266 Boards" by Spacehuhn Technologies
   - Alternatively, add this custom board package URL:
     ```
     https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json
     ```

4. **Select Board**:
   - Tools → Board → "Deauther ESP8266 Boards"
   - Choose your specific board (e.g., NodeMCU 1.0)

### Step 2: Python Environment Setup

1. **Install Python 3.7+** from [python.org](https://www.python.org/downloads/)

2. **Install Required Packages**:
   ```bash
   pip install flask pyautogui pygetwindow psutil
   ```

3. **Download the Python Script**:
   - Save `main.py` to your computer
   - Ensure all dependencies are installed

### Step 3: Upload Arduino Code

1. **Open Arduino IDE**
2. **Load the Sketch**:
   - Open `HID.ino` (main sketch file)
   - Ensure `deauth.cpp` and `deauth.h` are in the same folder
3. **Configure Settings**:
   - Select correct COM port
   - Set board type
   - Upload speed (115200 recommended)
4. **Upload** the sketch to your ESP8266

## 📖 Usage

### Evil Twin Mode (Default)

1. **Power on the device**
2. **Open Serial Monitor** (115200 baud rate)
3. **Scan for networks**:
   ```
   scan
   ```
4. **Select a target network** (note the index number)
5. **Start Fake AP**:
   ```
   ap [index]
   ```
6. **Optional: Start Deauth Attack**:
   ```
   deauth [index]
   ```
7. **Stop Deauth**:
   ```
   deauth off
   ```

### HID Simulator Mode

1. **Switch to HID Mode**:
   ```
   hid
   ```
2. **Connect to the ESP8266 AP**:
   - SSID: `HIDESP8266`
   - Password: `ESPATTCK`
3. **Open browser** and navigate to `192.168.4.1`
4. **Configure WiFi**:
   - Enter target network credentials
   - Click "Save & Connect"
5. **Once connected**, access the dashboard at the ESP's IP
6. **Set Target IP** (computer running Python script)
7. **Send commands** using the web interface

### Python Server Setup

1. **Run the Python script**:
   ```bash
   python main.py
   ```
2. **Server starts** on port 5000
3. **Configure firewall** to allow connections if needed
4. **Test connection** from ESP using:
   ```
   check
   ```

## 📋 Command Reference

### Evil Twin Commands
| Command | Description |
|---------|-------------|
| `scan` | Scan for WiFi networks |
| `ap [index]` | Create fake AP for selected network |
| `deauth [index]` | Start deauth attack on network |
| `deauth off` | Stop deauth attack |
| `hid` | Switch to HID Simulator mode |
| `help` | Show available commands |
| `status` | Show current status |

### HID Simulator Commands
| Command | Description |
|---------|-------------|
| `evil` | Switch to Evil Twin mode |
| `check` | Test Python server connection |
| `help` | Show available commands |
| `status` | Show current status |

## 🎯 HID Simulator Commands

### Basic Commands
- `WINR` - Press Windows+R
- `ENTER` - Press Enter key
- `TAB` - Press Tab key
- `ESC` - Press Escape key
- `SOFTWARE` - Focus on last opened application

### Text Input
- `TYPE:[text]` - Type specified text
- Multi-line text supported

### Delays
- `DELAY:[milliseconds]` - Wait specified time

### Hotkeys
- `HOTKEY:[key1],[key2]` - Press key combination
- Example: `HOTKEY:ctrl,c` for Ctrl+C

### Example Payloads
```
WINR
TYPE:notepad
ENTER
DELAY:1000
TYPE:Hello World!
ENTER
```

## 🔧 Troubleshooting

### Common Issues

1. **Upload Failed**:
   - Check COM port
   - Press BOOT button during upload
   - Reduce upload speed to 115200

2. **Deauth Not Working**:
   - Ensure using Spacehuhn core
   - Check target channel
   - Verify ESP8266 supports packet injection

3. **HID Connection Failed**:
   - Check Python server is running
   - Verify target IP address
   - Check firewall settings

4. **WiFi Issues**:
   - Reset ESP8266
   - Check antenna connection
   - Verify power supply

### Debug Mode

Enable serial output at 115200 baud rate for detailed debugging information.

## ⚠️ Disclaimer

This tool is intended for **educational purposes** and **authorized security testing only**. Users are responsible for ensuring they have proper authorization before testing any networks or systems. The authors are not responsible for any misuse or illegal activities.

### Legal Notice
- Only test on networks you own or have explicit permission to test
- Deauthentication attacks may be illegal in some jurisdictions
- HID injection should only be used on systems you own
- Always comply with local laws and regulations

### Ethical Use
- Use for penetration testing with proper authorization
- Educational demonstrations in controlled environments
- Security research and vulnerability assessment
- Never use for malicious purposes

---

**Created by:** Dinesh Patra  
**Version:** 1.0  
**Last Updated:** 2025

For support and updates, please refer to the project documentation and community forums.
