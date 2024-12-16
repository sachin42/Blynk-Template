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

// class TeleBot
// {
// private:
//   String _token;      // Bot token
//   long _lastUpdateId; // Tracks the last processed update_id
// public:
//   TeleBot(String token) : _token(token), _lastUpdateId(0) {}
//   bool sendMessage(String chat_id, String message)
//   {
//     if (WiFi.status() != WL_CONNECTED)
//     {
//       Serial.println("[ERROR] WiFi not connected!");
//       return false;
//     }
//     HTTPClient client;
//     String url = "https://api.telegram.org/bot" + _token + "/sendMessage?chat_id=" + chat_id + "&text=" + message;
//     client.begin(url);
//     int httpCode = client.GET();
//     if (httpCode > 0)
//     {
//       if (httpCode == HTTP_CODE_OK)
//       {
// #ifdef TELEBOT_DEBUG
//         String payload = client.getString();
//         Serial.println("[INFO] Message Sent Successfully!");
//         Serial.println(payload);
// #endif
//       }
//       else
//       {
//         Serial.printf("[ERROR] HTTP Response Code: %d\n", httpCode);
//         client.end();
//         return false;
//       }
//     }
//     else
//     {
//       Serial.printf("[ERROR] HTTP GET Failed: %s\n", client.errorToString(httpCode).c_str());
//       client.end();
//       return false;
//     }
//     client.end();
//     return true;
//   }
//   bool getUpdates(String &message, String &chat_id)
//   {
//     if (WiFi.status() != WL_CONNECTED)
//     {
//       Serial.println("[ERROR] WiFi not connected!");
//       return false;
//     }
//     HTTPClient client;
//     String url = "https://api.telegram.org/bot" + _token + "/getUpdates?offset=" + String(_lastUpdateId + 1);
//     client.begin(url);
//     int httpCode = client.GET();
//     if (httpCode > 0)
//     {
//       if (httpCode == HTTP_CODE_OK)
//       {
//         String payload = client.getString();
//         client.end();
//         return processTelegramResponse(payload, message, chat_id);
//       }
//       else
//       {
//         Serial.printf("[ERROR] HTTP Response Code: %d\n", httpCode);
//         client.end();
//         return false;
//       }
//     }
//     else
//     {
//       Serial.printf("[ERROR] HTTP GET Failed: %s\n", client.errorToString(httpCode).c_str());
//     }
//     client.end();
//     return false;
//   }
// private:
//   bool processTelegramResponse(String payload, String &message, String &chat_id)
//   {
//     // Parse JSON response
//     JsonDocument doc; // Adjust size based on payload
//     DeserializationError error = deserializeJson(doc, payload);
//     if (error)
//     {
//       Serial.print("[ERROR] JSON Deserialization failed: ");
//       Serial.println(error.c_str());
//       return false;
//     }
//     JsonArray result = doc["result"].as<JsonArray>();
//     for (JsonObject update : result)
//     {
//       long updateId = update["update_id"];
//       _lastUpdateId = updateId; // Update the last processed ID
//       if (update.containsKey("message"))
//       {
//         message = update["message"]["text"].as<String>();
//         chat_id = String(update["message"]["chat"]["id"].as<long>());
// #ifdef TELEBOT_DEBUG
//         Serial.println("[INFO] New Message Received:");
//         Serial.print("Chat ID: ");
//         Serial.println(chat_id);
//         Serial.print("Message: ");
//         Serial.println(message);
// #endif
//         return true; // New message successfully fetched
//       }
//     }
//     return false; // No new messages
//   }
// };

#endif
