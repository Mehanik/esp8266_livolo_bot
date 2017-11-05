/*
   Based https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/tree/master/examples/ESP8266/FlashLED
*/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <vector>
#include "livolo.h"
#include "config.h"


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
Livolo livolo(TRANSMITTER_PIN);

const int BOT_MTBS = 500; //mean time between scan messages
long bot_lasttime;   //last time messages' scan has been done
bool start = false;

const int ledPin = 2;

std::vector<String> allowed_ids;

bool CheckChatID(const String& chat_id) {
  for (const String &t : allowed_ids) {
    if (chat_id == t) {
      return true;
    }
  }
  return false;
}

void AddChatID(const String &id) {
  allowed_ids.push_back(id);
  Serial.println(allowed_ids[allowed_ids.size() - 1].length());

  File f = SPIFFS.open("/auth.txt", "a");

  if (!f) {
    // Serial.println("File open failed");
    return;
  }

  f.println(id);
}

void ReadChatIDs() {
  File f = SPIFFS.open("/auth.txt", "r");

  if (!f) {
    Serial.println("file open failed");
    return;
  }

  while (f.available()) {
    String s = f.readStringUntil('\n');
    s.trim();
    allowed_ids.push_back(s);
    // Serial.println(allowed_ids[allowed_ids.size() - 1]);
    // Serial.println(allowed_ids[allowed_ids.size() - 1].length());
  }
}

void handleNewMessages(int numNewMessages) {
  // Serial.println("handleNewMessages");
  // Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (CheckChatID(chat_id)) {
      if (text == "/room") {
        bot.sendMessage(chat_id, "Switching light in the room...", "");
        digitalWrite(ledPin, LOW); // Turn off status LED
        livolo.sendButton(LIVOLO_REMOTE_ID, 0);
        digitalWrite(ledPin, LOW); // Turn on status LED
        continue;
      }

      if (text == "/kitchen") {
        bot.sendMessage(chat_id, "Switching light in the kitchen...", "");
        digitalWrite(ledPin, LOW); // Turn off status LED
        livolo.sendButton(LIVOLO_REMOTE_ID, 96);
        digitalWrite(ledPin, LOW); // Turn on status LED
        continue;
      }

      if (text == "/hall") {
        bot.sendMessage(chat_id, "Switching light in the hall...", "");
        digitalWrite(ledPin, LOW); // Turn off status LED
        livolo.sendButton(LIVOLO_REMOTE_ID, 120);
        digitalWrite(ledPin, LOW); // Turn on status LED
        continue;
      }

      if (text == "/off") {
        bot.sendMessage(chat_id, "Switching OFF all lights...", "");
        digitalWrite(ledPin, LOW); // Turn off status LED
        livolo.sendButton(LIVOLO_REMOTE_ID, 106);
        digitalWrite(ledPin, LOW); // Turn on status LED
        continue;
      }
      if (text == "/help") {
        String welcome = "Welcome to ESP8266 Livolo control bot.\n";
        welcome += "Your Chat ID is " + chat_id + ".\n\n";
        welcome += "/room : to switch light in the room\n";
        welcome += "/kitchen : to switch light in the kitchen\n";
        welcome += "/hall : to switch light in the hall\n";
        welcome += "/off : to switch OFF all lights\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
        continue;
      }
    }

    if (text == "/start") {
      String welcome = "Please enter passphrase.";
      bot.sendMessage(chat_id, welcome, "");
      continue;
    }

    if (text == BOT_PASSPHRASE) {
      AddChatID(chat_id);
      bot.sendMessage(chat_id, "Welcome!", "");
      continue;
    }

    bot.sendMessage(chat_id, "Sorry.", "");
  }
}

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWIRD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  SPIFFS.begin();
  ReadChatIDs();

  pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
  delay(10);
  digitalWrite(ledPin, LOW); // initialize pin as off
}

void loop() {
  if (millis() > bot_lasttime + BOT_MTBS)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
