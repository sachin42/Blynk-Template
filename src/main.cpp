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
#include <EEPROM.h>

#define BOT_TOKEN "6553250052:AAF8xEp0zIXihw3qNOgcI87x0TJIXkWrsOI"
#define DEGREEC " °C"
#define DEGREEF " °F"
#define LED_BUILTIN 2

#define EEPROM_SIZE 512 // Define EEPROM size
#define MAX_CHAT_IDS 5  // Max number of strings
#define STRING_SIZE 20  // Max length of each string

String chat_ids[MAX_CHAT_IDS];  // Array to store chat IDs
#define BLYNK_TERMINAL_PIN V127 // Blynk Terminal widget on Virtual Pin V1

bool isFirstRun = false; // Flag to check if EEPROM is empty
int chatIdCount = 0;     // Tracks number of stored chat IDs

// WiFi credentials
char ssid[] = "project";
char pass[] = "12345678";

WidgetTerminal terminal(V0);
TeleBot bot(BOT_TOKEN);

BlynkTimer timer;

// Function to read a string from EEPROM
String readStringFromEEPROM(int startAddr)
{
  char data[STRING_SIZE + 1];
  int i;
  for (i = 0; i < STRING_SIZE; i++)
  {
    data[i] = EEPROM.read(startAddr + i);
    if (data[i] == '\0' || data[i] == 0xFF)
      break; // Stop if null terminator or empty EEPROM
  }
  data[i] = '\0'; // Ensure null termination
  return (i == 0) ? "" : String(data);
}

// Function to write a string to EEPROM
void writeStringToEEPROM(int startAddr, String data)
{
  int len = data.length();
  for (int i = 0; i < STRING_SIZE; i++)
  {
    EEPROM.write(startAddr + i, (i < len) ? data[i] : '\0'); // Write with null termination
  }
}

void eraseChatIds()
{
  Serial.println("Erasing all chat IDs from EEPROM...");

  for (int i = 0; i < MAX_CHAT_IDS * STRING_SIZE; i++)
  {
    EEPROM.write(i, 0xFF); // Write 0xFF to clear data
  }

  EEPROM.commit(); // Save changes
  Serial.println("EEPROM erased. Restarting ESP32...");

  ESP.restart(); // Restart ESP32 to reset logic
}

// Function to check if EEPROM is empty
bool isEEPROMEmpty()
{
  for (int i = 0; i < STRING_SIZE; i++)
  {
    if (EEPROM.read(i) != 0xFF)
      return false; // If any byte is not 0xFF, EEPROM has data
  }
  return true; // EEPROM is uninitialized
}

// Function to check and load chat IDs from EEPROM
void checkAndLoadChatIds()
{
  Serial.println("Checking EEPROM for stored chat IDs...");
  chatIdCount = 0;

  if (isEEPROMEmpty())
  {
    Serial.println("EEPROM is empty. Asking user for chat IDs via Blynk...");
    isFirstRun = true;
    Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "No chat ID found. Send up to 5 chat IDs or type 'exit' to save & restart:\n");
    return;
  }

  for (int i = 0; i < MAX_CHAT_IDS; i++)
  {
    int addr = i * STRING_SIZE; // EEPROM address for each string
    chat_ids[i] = readStringFromEEPROM(addr);

    if (chat_ids[i].length() > 0)
    {
      Serial.printf("Chat ID[%d]: %s\n", i, chat_ids[i].c_str());
      chatIdCount++; // Increment count of valid chat IDs
    }
  }

  if (chatIdCount > 0)
  {
    Serial.println("Chat IDs found. Proceeding to main logic.");
  }
  else
  {
    Serial.println("No chat ID found. Waiting for user input from Blynk...");
    isFirstRun = true;
    Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "No chat ID found. Send up to 5 chat IDs or type 'exit' to save & restart:\n");
  }
}

// Function to save all chat IDs to EEPROM and restart ESP32
void saveAndRestart()
{
  Serial.println("Saving chat IDs to EEPROM...");
  for (int i = 0; i < chatIdCount; i++)
  {
    int addr = i * STRING_SIZE;
    writeStringToEEPROM(addr, chat_ids[i]);
  }

  EEPROM.commit(); // Save to memory
  Serial.println("EEPROM updated. Restarting ESP32...");
  ESP.restart();
}

// Blynk Terminal input handler
BLYNK_WRITE(BLYNK_TERMINAL_PIN)
{
  if (!isFirstRun)
    return; // Ignore input if chat IDs are already present

  String receivedChatID = param.asStr();
  receivedChatID.trim(); // Remove any leading/trailing spaces

  if (receivedChatID.equalsIgnoreCase("exit"))
  { // Exit and save if user types "exit"
    if (chatIdCount > 0)
    {
      saveAndRestart();
    }
    else
    {
      Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "No chat IDs entered. Please add at least one before exiting.\n");
    }
    return;
  }

  if (chatIdCount < MAX_CHAT_IDS)
  {
    chat_ids[chatIdCount] = receivedChatID;
    Serial.printf("Stored Chat ID[%d]: %s\n", chatIdCount, receivedChatID.c_str());
    chatIdCount++;

    if (chatIdCount == MAX_CHAT_IDS)
    {
      Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "Max Chat IDs reached. Saving & restarting...\n");
      saveAndRestart();
    }
    else
    {
      Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "Chat ID stored. Enter more or type 'exit' to save & restart.\n");
    }
  }
  else
  {
    Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "Chat ID storage is full. Type 'exit' to save & restart.\n");
  }
}

BLYNK_WRITE(V0)
{
  String receivedCommand = param.asStr();

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
    terminal.println("- erase: Erase all chat IDs from EEPROM");
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
  else if (receivedCommand == "erase")
  {
    eraseChatIds();
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

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "protocol.electrocus.com", 8080);

  terminal.clear();
  terminal.println("Blynk Terminal Widget Connected!");
  terminal.println("------------------------------");
  terminal.flush();

  EEPROM.begin(EEPROM_SIZE);
  Serial.println("Reading chat IDs from EEPROM:");
  checkAndLoadChatIds();

  while (isFirstRun)
  {
    Blynk.run();
    delay(10);
  }

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
