#ifndef CHATIDSETUP_H
#define CHATIDSETUP_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Blynk/BlynkHandlers.h"

#define EEPROM_SIZE 512 // Define EEPROM size
#define MAX_CHAT_IDS 5  // Max number of strings
#define STRING_SIZE 20  // Max length of each string

String chat_ids[MAX_CHAT_IDS];  // Array to store chat IDs
#define BLYNK_TERMINAL_PIN V127 // Blynk Terminal widget on Virtual Pin V1

bool isFirstRun = false; // Flag to check if EEPROM is empty
int chatIdCount = 0;     // Tracks number of stored chat IDs

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
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("Erasing all chat IDs from EEPROM...");

  for (int i = 0; i < MAX_CHAT_IDS * STRING_SIZE; i++)
  {
    EEPROM.write(i, 0xFF); // Write 0xFF to clear data
  }

  EEPROM.commit(); // Save changes
  Serial.println("EEPROM erased. Restarting ESP32...");
  EEPROM.end();
  delay(500);
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

// This function is called in the setup() function to load chat IDs from EEPROM
void loadChatIds()
{
  WidgetTerminal chatTerminal(V127); // Blynk Terminal widget on Virtual Pin
  chatTerminal.clear(); // Clear the terminal widget
  chatTerminal.flush(); // Flush the terminal widget
  EEPROM.begin(EEPROM_SIZE);
  checkAndLoadChatIds();

  while (isFirstRun)
  {
    Blynk.run();
    delay(10);
  }
  EEPROM.end();
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

// add another chat ID to the list
void addChatId(String chatId)
{
  EEPROM.begin(EEPROM_SIZE);
  if (chatIdCount < MAX_CHAT_IDS)
  {
    chat_ids[chatIdCount] = chatId;
    chatIdCount++;
    int addr = (chatIdCount - 1) * STRING_SIZE;
    writeStringToEEPROM(addr, chatId); // Write new chat ID to EEPROM
    Serial.printf("Added Chat ID[%d]: %s\n", chatIdCount - 1, chatId.c_str());
    Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "Added Chat ID: " + chatId + "\n");
  }
  else
  {
    Serial.println("Max chat IDs reached. Cannot add more.");
    Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "Max chat IDs reached. Cannot add more.\n");
  }
  EEPROM.commit(); // Save changes to EEPROM
  EEPROM.end();
}

// Blynk Terminal input handler
BLYNK_WRITE(BLYNK_TERMINAL_PIN)
{
  String receivedChatID = param.asStr();
  receivedChatID.trim(); // Remove any leading/trailing spaces

  // if recerivedChatID starts with addChatId, add it to the list
  if (receivedChatID.startsWith("add-"))
  {
    String newChatId = receivedChatID.substring(4); // Extract the chat ID after "addChatId-"
    newChatId.trim();                               // Remove any leading/trailing spaces
    addChatId(newChatId);
  }

  else if (receivedChatID.equalsIgnoreCase("erase"))
  {
    eraseChatIds(); // Erase all chat IDs from EEPROM
    Blynk.virtualWrite(BLYNK_TERMINAL_PIN, "All chat IDs erased. Restarting...\n");
    return;
  }

  if (!isFirstRun)
    return; // Ignore input if chat IDs are already present

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

#endif // CHATIDSETUP_H