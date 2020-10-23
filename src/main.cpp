#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>

#define WIFI_CONF_PATH "/wifi.json"

struct WifiConfig {
  String ssid;
  String pass;
  String error;
};

void log(String msg);
void loadWifiConfig(String fileName, WifiConfig &cfg);

void setup() {
  Serial.begin(115200);
  log("Getting wifi configuration from file...");
  if (!SPIFFS.begin()) {
    log("Failed to mount file system, exiting.");
    return;
  }

  WifiConfig cfg;
  loadWifiConfig(WIFI_CONF_PATH, cfg);

  log(cfg.ssid);
  log(cfg.pass);
}

void loop() {
  // put your main code here, to run repeatedly:
}


void loadWifiConfig(String fileName, WifiConfig &cfg) {
  File configfile = SPIFFS.open(fileName, "r");
  if (!configfile) {
    cfg.error = "Failed to open config file";
    return;
  }

  size_t size = configfile.size();
  if (size > 1024) {
    cfg.error = "Config file size is too large";
    return;
  }

  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, configfile);
  if (error) {
    cfg.error = "Failed to deserialize data, using default configuration";
    return;
  }

  // Copy values from the JsonObject to the Config
  cfg.ssid = doc["ssid"].as<String>();
  cfg.pass = doc["pass"].as<String>();

  // We don't need the file anymore
  configfile.close();
}

void log(String msg) {
  Serial.println(msg);
}
