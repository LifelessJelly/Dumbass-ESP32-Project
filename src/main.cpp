#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char ssid[] = "SSID_HERE";
const char password[] = "PASSWORD_HERE";
const char mqtt_server[] = "MQTT_SERVER_IP_HERE";
WiFiClient wifiClient;
PubSubClient client {wifiClient};

void wifi_begin() {
    WiFi.begin(ssid, password);

    Serial.println("Connecting...");
    int stopwatch {};
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(1000);
        ++stopwatch;
        Serial.print("\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r");
        Serial.print("Connecting pending for ");
        Serial.print(stopwatch);
        Serial.print('s');
    }
    Serial.print('\n');
    Serial.println("WiFi Connected!");
    Serial.println("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
}

void callback_function(const char* topic, unsigned char* payload, unsigned int length) {
    Serial.print("Sending topic: ");
    Serial.println(*topic);
    //other callback control stuff if needed
}

void reconnect() {
    Serial.println("Connection lost, attempting reconnection...");
    while (!client.connected()) {
        if (client.connect("Esben's secret dilapidated shack")) {
            client.subscribe("mong/output");
            Serial.println("Success");
        }
        else {
            delay(5000);
            /*
             * ---------------------------------------------------------------------------------------------------------
             *                      IF THE ESP32 EVER CRASHES, IT MIGHT BE THIS LINE
             * ---------------------------------------------------------------------------------------------------------
             */
            //slowly crash the ESP32 with stack_overflow if it does not connect
            reconnect();
        }
    }
}

void setup() {
    Serial.begin(115200);
    wifi_begin();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback_function);
}

void loop() {
    reconnect();
    client.loop();
    delay(1000);
    client.publish("mong/camera", "image bytes (JPEG FORMAT) goes here");
    client.publish("mong/temperature_object", "object temperature goes here");
    client.publish("mong/temperature_ambient", "ambient temperature goes here");
    client.publish("mong/imu", "IMU data goes here (add more publishing channels if needed or just shove all the bytes into an array and send it)");
}
