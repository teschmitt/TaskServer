#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>


struct WifiConfig {
  String ssid;
  String password;
  String error;
};


void loadWifiConfig(String fileName, WifiConfig* cfg);


#endif /* CONFIG_H_ */
