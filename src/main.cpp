#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WiFi.h>

#define WIFI_CONF_PATH "/wifi.json"

struct WifiConfig
{
  String ssid;
  String password;
  String error;
};

// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void log(String msg);
void loadWifiConfig(String fileName, WifiConfig &cfg);

void setup()
{
  Serial.begin(115200);
  log("Getting wifi configuration from file...");
  if (!SPIFFS.begin())
  {
    log("Failed to mount file system, exiting.");
    return;
  }

  WifiConfig cfg;
  loadWifiConfig(WIFI_CONF_PATH, cfg);

  if (cfg.error != NULL)
  {
    log("Error while loading wifi configuration, exiting.");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(cfg.ssid);
  WiFi.begin(cfg.ssid.c_str(), cfg.password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New client connected"); // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            StaticJsonDocument<200> res;
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println("Connection: close");
            client.println();

            res["success"] = "great";
            serializeJson(res, client);

            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void loadWifiConfig(String fileName, WifiConfig &cfg)
{
  File configfile = SPIFFS.open(fileName, "r");
  if (!configfile)
  {
    cfg.error = "Failed to open config file";
    return;
  }

  size_t size = configfile.size();
  if (size > 1024)
  {
    cfg.error = "Config file size is too large";
    return;
  }

  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, configfile);
  if (error)
  {
    cfg.error = "Failed to deserialize data, using default configuration";
    return;
  }

  // Copy values from the JsonObject to the Config
  cfg.ssid = doc["ssid"].as<String>();
  cfg.password = doc["password"].as<String>();

  // We don't need the file anymore
  configfile.close();
}

void log(String msg)
{
  Serial.println(msg);
}
