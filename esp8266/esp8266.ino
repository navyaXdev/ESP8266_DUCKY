#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>

const char* AP_SSID = "HIDESP8266";
const char* AP_PASSWORD = "ESPATTCK";

ESP8266WebServer setupServer(80);
ESP8266WebServer runtimeServer(80);

String wifiSSID = "";
String wifiPassword = "";
String targetIP = "192.168.1.100";
bool isInRuntimeMode = false;

// Function declarations
void connectToWiFi();
void saveTargetIP();
String loadTargetIP();
void handleRoot();
void handleSave();
void handleSaveIP();

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
    runtimeServer.on("/", handleRoot);
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

void handleRoot() {
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
    // Runtime Mode with SOFTWARE button
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

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  setupServer.on("/", handleRoot);
  setupServer.on("/save", HTTP_POST, handleSave);
  setupServer.begin();
  
  Serial.println("Access Point Started");
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  if (isInRuntimeMode) {
    runtimeServer.handleClient();
  } else {
    setupServer.handleClient();
  }
}