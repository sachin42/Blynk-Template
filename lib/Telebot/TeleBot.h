#ifndef TELEBOT_H
#define TELEBOT_H

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

#include <ArduinoJson.h>

// Uncomment the following line to enable debug mode
// #define TELEBOT_DEBUG

class TeleBot
{
private:
  String _token;      // Bot token
  long _lastUpdateId; // Tracks the last processed update_id

public:
  TeleBot(String token);

  bool sendMessage(String chat_id, String message);                  // Send String to one user.
  int sendMessage(String chat_ids[], size_t length, String message); // Send String to multiple user by a string array.
  bool getUpdates(String &message, String &chat_id);                 // Get messages from user and get chat_id and message paasing strings by reference.
  bool getUpdates(String &message, String &chat_id, String &name);   // Get messages from user and get chat_id, name and message paasing strings by reference.

private:
  bool processTelegramResponse(String payload, String &message, String &chat_id);
  bool processTelegramResponse(String payload, String &message, String &chat_id, String &name);
};

#endif
