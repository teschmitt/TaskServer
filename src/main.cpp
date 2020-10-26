#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "config.h"
#include "uuid.h"

#define APP_PORT 45678
#define WIFI_CONF_PATH "/wifi.json"


WiFiServer server(APP_PORT);
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void connectToWifi(WifiConfig*);
void log(String);

void setup ()
{
  Serial.begin(115200);
  log("Getting wifi configuration from file...");
  if (!SPIFFS.begin())
  {
    log("Failed to mount file system, exiting.");
    return;
  }

  WifiConfig cfg;
  loadWifiConfig(WIFI_CONF_PATH, &cfg);

  if (cfg.error != NULL)
  {
    log("Error while loading wifi configuration, exiting.");
  }

  connectToWifi(&cfg);
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    log("New client connected");
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();

      if (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n') {
            StaticJsonDocument<200> res;
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();

            res["success"] = "great";
            res["id"] = uuid::generate_uuid_v4();
            serializeJson(res, client);

            client.println();
            break;
        }
      }
    }
    client.stop();
    log("Client disconnected.\n");
  }
}


void connectToWifi(WifiConfig* c) {
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  log(c->ssid);
  WiFi.begin(c->ssid.c_str(), c->password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  log("\nWiFi connected.");
  log("IP address: ");
  log(WiFi.localIP());
  server.begin();
}


void log(String msg)
{
  Serial.println(msg);
}
