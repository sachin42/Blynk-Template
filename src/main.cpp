#define BLYNK_PRINT Serial
#define BLYNK_AUTH_TOKEN "4huy-J3UAzCmrBkz-z_Mprb5UR1a6t1z"

#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#endif

#include "TeleBot.h"
#include "chatidsetup.h"

#define BOT_TOKEN "6553250052:AAF8xEp0zIXihw3qNOgcI87x0TJIXkWrsOI"
#define DEGREEC " °C"
#define DEGREEF " °F"
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// WiFi credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

WidgetTerminal terminal(V0);
TeleBot bot(BOT_TOKEN);
BlynkTimer timer;

BLYNK_WRITE(V0)
{
  String receivedCommand = param.asStr();
  receivedCommand.trim();        // Remove leading and trailing whitespace
  receivedCommand.toLowerCase(); // Convert to lowercase for case-insensitive comparison

  terminal.print("Received Command: ");
  terminal.println(receivedCommand);

  if (receivedCommand == "help")
  {
    terminal.println("Available Commands:");
#ifdef ESP32
    terminal.println("- cpu: Get current cpu temperature");
#endif
    terminal.println("- status: Check device status");
    terminal.println("- restart: Restart the device");
    terminal.println("- clear: Clear the terminal");
  }
#ifdef ESP32
  else if (receivedCommand == "cpu")
  {
    terminal.print("Current Temperature: ");
    terminal.println(String(temperatureRead(), 2) + DEGREEC);
  }
#endif
  else if (receivedCommand == "status")
  {
    terminal.println("Device Online");
    terminal.print("WiFi Signal Strength: ");
    terminal.print(WiFi.RSSI());
    terminal.println(" dBm");
  }
  else if (receivedCommand == "restart")
  {
    terminal.println("System Restarting");
    delay(1000);
    ESP.restart();
  }
  else if (receivedCommand == "clear")
  {
    terminal.clear();
  }
  else
  {
    terminal.println("Unknown Command. Type 'help' for assistance.");
  }

  terminal.flush();
}

// void polling()
// {
//   String message;
//   String chat_id;
//   String name;
//   if (bot.getUpdates(message, chat_id, name))
//   {
//     // Uncoment code block below if you want to print incomming telegram massages.
//     // Serial.print("User name :");
//     // Serial.println(name);
//     // Serial.print("Chat ID :");
//     // Serial.println(chat_id);
//     // Serial.print("Received Message :");
//     // Serial.println(message);
//     if (message.equals("/start"))
//       bot.sendMessage(chat_id, "Welcome");
// #ifdef ESP32
//     else if (message.equals("/temp"))
//       bot.sendMessage(chat_id, String(temperatureRead(), 2) + DEGREEC);
// #endif
//     else if (message.equals("/restart")){
//       bot.sendMessage(chat_id, "System Restarting");
//       delay(1000);
//       ESP.restart();
//     }
//   }
// }

void checkBlynkStatus()
{
  bool isconnected = Blynk.connected();
  if (isconnected == false)
  {
#ifdef ESP32
    digitalWrite(LED_BUILTIN, LOW); // Turn off WiFi LED
#elif ESP8266
    digitalWrite(LED_BUILTIN, HIGH); // Turn off WiFi LED
#endif
  }
  if (isconnected == true)
  {
#ifdef ESP32
    digitalWrite(LED_BUILTIN, HIGH); // Turn on WiFi LED
#elif ESP8266
    digitalWrite(LED_BUILTIN, LOW); // Turn on WiFi LED
#endif
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef ESP32
  digitalWrite(LED_BUILTIN, LOW); // Turn off WiFi LED
#elif ESP8266
  digitalWrite(LED_BUILTIN, HIGH); // Turn off WiFi LED
#endif
  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "protocol.electrocus.com", 8080);

  terminal.clear();
  terminal.println("Blynk Terminal Widget Connected!");
  terminal.println("------------------------------");
  terminal.flush();

  loadChatIds(); // Load chat IDs from EEPROM

  int success = bot.sendMessage(chat_ids, chatIdCount, "Device Started");
  Serial.print("Sccess Count :");
  Serial.println(success);

  // timer.setInterval(10000UL, polling);
  timer.setInterval(5000UL, checkBlynkStatus);
}

void loop()
{
  Blynk.run();
  timer.run();
}
