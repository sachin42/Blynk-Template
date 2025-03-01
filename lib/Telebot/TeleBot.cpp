#include "TeleBot.h"

TeleBot::TeleBot(String token) : _token(token), _lastUpdateId(0) {}

String urlEncode(String str)
{
  String encodedString = "";
  char c;
  char buf[4];
  for (size_t i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == ' ')
    {
      encodedString += "%20"; // Replace spaces
    }
    else if (c == '\n')
    {
      encodedString += "%0A"; // Replace newlines
    }
    else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      encodedString += c; // Keep letters and numbers
    }
    else
    {
      sprintf(buf, "%%%02X", (unsigned char)c); // Encode special characters
      encodedString += buf;
    }
  }
  return encodedString;
}

bool TeleBot::sendMessage(String chat_id, String message)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[ERROR] WiFi not connected!");
    return false;
  }

  String newmessage = urlEncode(message);
  HTTPClient client;
  String url = "https://api.telegram.org/bot" + _token + "/sendMessage?chat_id=" + chat_id + "&text=" + newmessage;
#ifdef ESP32
  client.begin(url);
#elif ESP8266
  std::unique_ptr<BearSSL::WiFiClientSecure> wclient(new BearSSL::WiFiClientSecure);
  wclient->setInsecure();
  client.begin(*wclient, url);
#endif
  int httpCode = client.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
#ifdef TELEBOT_DEBUG
      String payload = client.getString();
      Serial.println("[INFO] Message Sent Successfully!");
      Serial.println(payload);
#endif
    }
    else
    {
      Serial.printf("[ERROR] HTTP Response Code: %d\n", httpCode);
      client.end();
      return false;
    }
  }
  else
  {
    Serial.printf("[ERROR] HTTP GET Failed: %s\n", client.errorToString(httpCode).c_str());
    client.end();
    return false;
  }

  client.end();
  return true;
}

int TeleBot::sendMessage(String chat_ids[], size_t length, String message)
{
  int success = 0;
  for (size_t i = 0; i < length; i++)
  {
    if (!sendMessage(chat_ids[i], message))
    {
      Serial.print("[ERROR] Failed to send message to chat ID: ");
      Serial.println(chat_ids[i]);
    }
    else
      success++;
  }
  return success;
}

bool TeleBot::getUpdates(String &message, String &chat_id)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[ERROR] WiFi not connected!");
    return false;
  }

  HTTPClient client;
  String url = "https://api.telegram.org/bot" + _token + "/getUpdates?offset=" + String(_lastUpdateId + 1);
#ifdef ESP32
  client.begin(url);
#elif ESP8266
  std::unique_ptr<BearSSL::WiFiClientSecure> wclient(new BearSSL::WiFiClientSecure);
  wclient->setInsecure();
  client.begin(*wclient, url);
#endif

  int httpCode = client.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = client.getString();
#ifdef TELEBOT_DEBUG
      Serial.println("[INFO] Got Updates!");
      Serial.println(payload);
#endif
      client.end();
      return processTelegramResponse(payload, message, chat_id);
    }
    else
    {
      Serial.printf("[ERROR] HTTP Response Code: %d\n", httpCode);
      client.end();
      return false;
    }
  }
  else
  {
    Serial.printf("[ERROR] HTTP GET Failed: %s\n", client.errorToString(httpCode).c_str());
  }

  client.end();
  return false;
}

bool TeleBot::getUpdates(String &message, String &chat_id, String &name)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[ERROR] WiFi not connected!");
    return false;
  }

  HTTPClient client;
  String url = "https://api.telegram.org/bot" + _token + "/getUpdates?offset=" + String(_lastUpdateId + 1);
#ifdef ESP32
  client.begin(url);
#elif ESP8266
  std::unique_ptr<BearSSL::WiFiClientSecure> wclient(new BearSSL::WiFiClientSecure);
  wclient->setInsecure();
  client.begin(*wclient, url);
#endif

  int httpCode = client.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = client.getString();
#ifdef TELEBOT_DEBUG
      Serial.println("[INFO] Got Updates!");
      Serial.println(payload);
#endif
      client.end();
      return processTelegramResponse(payload, message, chat_id, name);
    }
    else
    {
      Serial.printf("[ERROR] HTTP Response Code: %d\n", httpCode);
      client.end();
      return false;
    }
  }
  else
  {
    Serial.printf("[ERROR] HTTP GET Failed: %s\n", client.errorToString(httpCode).c_str());
  }

  client.end();
  return false;
}

bool TeleBot::processTelegramResponse(String payload, String &message, String &chat_id)
{
  // Parse JSON response
  JsonDocument doc; // Adjust size based on payload
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("[ERROR] JSON Deserialization failed: ");
    Serial.println(error.c_str());
    return false;
  }

  JsonArray result = doc["result"].as<JsonArray>();
  for (JsonObject update : result)
  {
    long updateId = update["update_id"];
    _lastUpdateId = updateId; // Update the last processed ID

    if (update["message"].is<JsonObject>())
    {
      message = update["message"]["text"].as<String>();
      chat_id = String(update["message"]["chat"]["id"].as<long>());
#ifdef TELEBOT_DEBUG
      Serial.println("[INFO] New Message Received:");
      Serial.print("Chat ID: ");
      Serial.println(chat_id);
      Serial.print("Message: ");
      Serial.println(message);
#endif
      return true; // New message successfully fetched
    }
  }
  // update.containsKey("message") &&
  return false; // No new messages
}

bool TeleBot::processTelegramResponse(String payload, String &message, String &chat_id, String &name)
{
  // Parse JSON response
  JsonDocument doc; // Adjust size based on payload
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("[ERROR] JSON Deserialization failed: ");
    Serial.println(error.c_str());
    return false;
  }

  JsonArray result = doc["result"].as<JsonArray>();
  for (JsonObject update : result)
  {
    long updateId = update["update_id"];
    _lastUpdateId = updateId; // Update the last processed ID

    if (update["message"].is<JsonObject>())
    {
      message = update["message"]["text"].as<String>();
      chat_id = update["message"]["from"]["id"].as<String>();
      name = update["message"]["from"]["first_name"].as<String>() + " " + update["message"]["from"]["last_name"].as<String>();
#ifdef TELEBOT_DEBUG
      Serial.println("[INFO] New Message Received:");
      Serial.print("Chat ID: ");
      Serial.println(chat_id);
      Serial.print("Message: ");
      Serial.println(message);
#endif
      return true; // New message successfully fetched
    }
  }
  return false; // No new messages
}
