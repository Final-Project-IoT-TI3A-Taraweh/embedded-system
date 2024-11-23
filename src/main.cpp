#include "WiFi.h"
#include "PubSubClient.h"
#include "Wire.h"
#include "DHT.h"
#include "Arduino.h"
#include "ArduinoJson.h"
#define ON 0x0
#define OFF 0x1

int DHT_PIN = 17;
int SOIL_PIN = 34;
int lamps_pin[3] = { 13, 12, 14};

JsonDocument jsonDocument;
char buffer[250];


char ssid[] = "";
char password[] = "";

const char* mqtt_server = "";  // e.g., "192.168.1.10" or domain like "broker.example.com"
const int mqtt_port = 1883;                        // Default MQTT port is 1883
const char* mqtt_user = "";      // Leave empty if not using authentication
const char* mqtt_password = "";

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHT_PIN, DHT22);

void setup_wifi() {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("connected");
            client.subscribe("iot/sensor/lamp");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    dht.begin();
    setup_wifi();
    client.setServer(mqtt_server, 1883);

    for (int i : lamps_pin) {
        pinMode(i, OUTPUT);
    }
}

void loop(){
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    int soil = map(analogRead(SOIL_PIN), 0, 4095, 0, 1000);

    jsonDocument["timestamp"] = millis();
    jsonDocument["temperature"] = dht.readTemperature();
    jsonDocument["humidity"] = dht.readHumidity();
    jsonDocument["soil_moisture"] = soil;
    serializeJson(jsonDocument, buffer);

    client.publish("iot/sensor", buffer);

    delay(2000);
}