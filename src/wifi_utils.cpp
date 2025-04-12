#include <logger.h>
#include <WiFi.h>
#include "configuration.h"
#include "web_utils.h"
#include "display.h"


extern      Configuration       Config;
extern      logging::Logger     logger;

uint32_t    noClientsTime        = 0;

#ifndef AP_ADDRESS_IP
    #define AP_ADDRESS_IP   "192.168.4.1"
#endif

namespace WIFI_Utils {

    void startAutoAP() {
        IPAddress ap_ip((uint32_t)0);
        ap_ip.fromString(AP_ADDRESS_IP);
        IPAddress ap_gateway((uint32_t)ap_ip);
        //NOTE: for other than 192.168.x.y addresses this needs additional logic
        IPAddress ap_subnet((uint32_t)0x00FFFFFF); //255.255.255.0
        WiFi.mode(WIFI_MODE_NULL);
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(ap_ip, ap_gateway, ap_subnet);
        WiFi.softAP("LoRaTracker-AP", Config.wifiAP.password);
    }

    void checkIfWiFiAP() {
        if (Config.wifiAP.active || Config.beacons[0].callsign == "NOCALL-7"){
            displayShow(" LoRa APRS", "    ** WEB-CONF **","", "WiFiAP:LoRaTracker-AP", "IP   :  " AP_ADDRESS_IP ,"");
            logger.log(logging::LoggerLevel::LOGGER_LEVEL_WARN, "Main", "WebConfiguration Started!");
            startAutoAP();
            WEB_Utils::setup();
            while (true) {
                if (WiFi.softAPgetStationNum() > 0) {
                    noClientsTime = 0;
                } else {
                    if (noClientsTime == 0) {
                        noClientsTime = millis();
                    } else if ((millis() - noClientsTime) > 2 * 60 * 1000) {
                        logger.log(logging::LoggerLevel::LOGGER_LEVEL_WARN, "Main", "WebConfiguration Stopped!");
                        displayShow("", "", "  STOPPING WiFi AP", 2000);
                        Config.wifiAP.active = false;
                        Config.writeFile();
                        WiFi.softAPdisconnect(true);
                        ESP.restart();
                    }
                }
            }
        } else {
            WiFi.mode(WIFI_OFF);
            logger.log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, "Main", "WiFi controller stopped");
        }
    }
}