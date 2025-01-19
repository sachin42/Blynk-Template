#define BLYNK_PRINT Serial
#define BLYNK_AUTH_TOKEN "k8xNIvWelw4RiOuOeDhq2e1W64h0NCf3"

#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#endif
#include "TeleBot.h"

#define BOT_TOKEN "6553250052:AAF8xEp0zIXihw3qNOgcI87x0TJIXkWrsOI"
#define DEGREEC " °C"
#define DEGREEF " °F"
#define LED_BUILTIN 2

// Add up to five different chat IDs
String chat_ids[] = {"1919990541"};

// WiFi credentials
char ssid[] = "project";
char pass[] = "12345678";

WidgetTerminal terminal(V0);
TeleBot bot(BOT_TOKEN);

BlynkTimer timer;

#ifdef ESP8266
float temperatureRead()
{
  return random(32.2, 52.6);
}
#endif

BLYNK_WRITE(V0)
{
  String receivedCommand = param.asStr();

  terminal.print("Received Command: ");
  terminal.println(receivedCommand);

  if (receivedCommand == "help")
  {
    terminal.println("Available Commands:");
    terminal.println("- temp: Get current temperature");
    terminal.println("- status: Check device status");
  }
  else if (receivedCommand == "temp")
  {
    terminal.print("Current Temperature: ");
    terminal.println(String(temperatureRead(), 2) + DEGREEC);
  }
  else if (receivedCommand == "status")
  {
    terminal.println("Device Online");
    terminal.print("WiFi Signal Strength: ");
    terminal.print(WiFi.RSSI());
    terminal.println(" dBm");
  }
  else
  {
    terminal.println("Unknown Command. Type 'help' for assistance.");
  }

  terminal.flush();
}

void polling()
{
  String message;
  String chat_id;
  String name;
  if (bot.getUpdates(message, chat_id, name))
  {
    // Uncoment code block below if you want to print incomming telegram massages.

    // Serial.print("User name :");
    // Serial.println(name);
    // Serial.print("Chat ID :");
    // Serial.println(chat_id);
    // Serial.print("Received Message :");
    // Serial.println(message);

    if (message.equals("/start"))
      bot.sendMessage(chat_id, "Welcome");
    else if (message.equals("/temp"))
      bot.sendMessage(chat_id, String(temperatureRead(), 2) + DEGREEC);
  }
}

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

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "protocol.electrocus.com", 8080);

  terminal.clear();
  terminal.println("Blynk Terminal Widget Connected!");
  terminal.println("------------------------------");
  terminal.flush();

  int success = bot.sendMessage(chat_ids, sizeof(chat_ids) / sizeof(chat_ids[0]), "Device Started");

  Serial.print("Sccess Count :");
  Serial.println(success);

  timer.setInterval(10000UL, polling);
  timer.setInterval(5000UL, checkBlynkStatus);
}

void loop()
{
  Blynk.run();
  timer.run();
}
