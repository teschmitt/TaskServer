#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "config.h"

void loadWifiConfig(String fileName, WifiConfig* cfg)
{
  File configfile = SPIFFS.open(fileName, "r");
  if (!configfile)
  {
    cfg->error = "Failed to open config file";
    return;
  }

  size_t size = configfile.size();
  if (size > 1024)
  {
    cfg->error = "Config file size is too large";
    return;
  }

  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, configfile);
  if (error)
  {
    cfg->error = "Failed to deserialize data, using default configuration";
    return;
  }

  // Copy values from the JsonObject to the Config
  cfg->ssid = doc["ssid"].as<String>();
  cfg->password = doc["password"].as<String>();

  // We don't need the file anymore
  configfile.close();
}
