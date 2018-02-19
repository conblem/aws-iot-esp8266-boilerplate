#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>

namespace {
    const char* awsEndpoint = "a1tmnj7iegjw8k.iot.us-west-2.amazonaws.com";
}

const void msgReceived(const char* topic, const byte* payload, const unsigned int len);
const void waitForMQTTConnection();
const void setupArduinoOTA();

WiFiClientSecure wifiClient;
PubSubClient client(awsEndpoint, 8883, msgReceived, wifiClient);


void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    pinMode(BUILTIN_LED, OUTPUT);

    WiFiManager wiFiManager;
    wiFiManager.autoConnect();

    SPIFFS.begin();
    auto key = SPIFFS.open("/key.der", "r");
    wifiClient.loadPrivateKey(key);

    auto crt = SPIFFS.open("/crt.der", "r");
    wifiClient.loadCertificate(crt);
    SPIFFS.end();

    setupArduinoOTA();
    waitForMQTTConnection();
    client.publish("/esp8266", "Hallo");
    client.subscribe("/esp8266");
}

void loop() {
    client.loop();
    ArduinoOTA.handle();
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    digitalWrite(BUILTIN_LED, LOW);
}

const void msgReceived(const char* topic, const byte* payload, const unsigned int length) {
    Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

const void waitForMQTTConnection() {
    while(!client.connect("esp-8266-1")) {
        delay(500);
    }
    Serial.println("Client connected");
};

const void setupArduinoOTA() {
    const auto psk = WiFi.psk();
    char pskChar[psk.length() + 1];
    psk.toCharArray(pskChar, psk.length() + 1);

    ArduinoOTA.setPassword(pskChar);
    ArduinoOTA.onStart([]() {
        Serial.println("\nStart");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.begin();
}
