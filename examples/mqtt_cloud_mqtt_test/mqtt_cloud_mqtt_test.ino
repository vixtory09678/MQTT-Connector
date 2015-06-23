#define DEBUG_MODE

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <MqttWrapper.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "dht_helper.h"

// const char* ssid     = "OpenWrt_NAT_500GP.101";
// const char* pass = "activegateway";

// const char* ssid     = "MAKERCLUB-CM";
// const char* pass = "welcomegogogo";

const char* ssid     = "Opendream Play";
const char* pass = "5k,skrijv',7'sik";

// const char* ssid     = "Opendream";
// const char* pass = "gfkgvkgv'2015!!!!";


MqttWrapper *mqtt;
DHT *dht;

void connect_wifi()
{
    WiFi.begin(ssid, pass);

    int retries = 0;
    while ((WiFi.status() != WL_CONNECTED))
    {
        Serial.print(".");
        retries++;
        delay(500);
    }

    Serial.println("WIFI CONNECTED ");
}

void reconnect_wifi_if_link_down() {
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WIFI DISCONNECTED");
        connect_wifi();
    }
}


void callback(const MQTT::Publish& pub) {
    if (pub.payload_string() == "0") {
        Serial.print(" => ");
        Serial.println(pub.payload_string());
    }
    else if(pub.payload_string() == "1") {
        Serial.print(" => ");
        Serial.println(pub.payload_string());
    }
    else {
        Serial.print(pub.topic());
        Serial.print(" => ");
        Serial.println(pub.payload_string());
    }
}

void hook_prepare_data(JsonObject** root) {
  JsonObject& data = (*(*root))["d"];

  static float t_dht;
  static float h_dht;

  read_dht(dht, &t_dht, &h_dht);

  data["myName"] = "NAT";
  data["temp"] = t_dht;
  data["humid"] = h_dht;

}

void hook_configuration(MqttWrapper::Config config) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String result;
    for (int i = 0; i < 6; ++i)
    {
        result += String(mac[i], 16);
    }

    *(config.clientId) = String("d:quickstart:arduino:") + result;
    *(config.username) = String("test");
    *(config.password) = String("test");
    *(config.channelId) = String("esp8266/");
    *(config.topicPub) = "iot-2/evt/status/fmt/json";
}

void hook_publish_data(char* data) {
    Serial.print("PUBLISH: ->");
    Serial.println(data);
}

void setup() {
    Serial.begin(115200);
    pinMode(0, INPUT_PULLUP);
    delay(10);
    Serial.println();
    Serial.println();

    connect_wifi();

    mqtt = new MqttWrapper("m20.cloudmqtt.com", 19642, hook_configuration);
    mqtt->connect(callback);
    mqtt->set_prepare_data_hook(hook_prepare_data, 5000);
    mqtt->set_publish_data_hook(hook_publish_data);
}

void loop() {
    reconnect_wifi_if_link_down();
    mqtt->loop();

    // ตรวจจับการกด Switch
    if (digitalRead(0) == LOW) {
        // วนลูปจนกว่าจะเอาปล่อย Switch
        while(digitalRead(0) == LOW) { 
            mqtt->loop();
            yield(); 
        }
        String status = "0";
        mqtt->sync_pub(status);
    }

}