/*
 * Deauthentication Header File
 * Author - WireBits
 */

#ifndef DEAUTH_H
#define DEAUTH_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

void startDeauth(int index);
void stopDeauth();
void sendDeauth();
void initDeauth();

extern bool deauthRunning;
extern bool deauthExecuted;

#endif
