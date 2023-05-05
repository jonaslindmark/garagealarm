#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <rtc.h>

#define GARAGE_REED GPIO_NUM_32
#define SSID "GAnkan"
#define WIFI_PASSWORD "<password>"

#define mqtt_server "192.168.86.180"

#define garage_topic "sensor/garagedoor"

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(WiFi.status());
    delay(1000);
  }
  Serial.println("Connected successfully");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.RSSI());
}

void printWakeupReason(){
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: 
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1: 
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER: 
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: 
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP: 
      Serial.println("Wakeup caused by ULP program");
      break;
    default: 
      Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up!");
  printWakeupReason();

  pinMode(GARAGE_REED, INPUT_PULLDOWN);
  connectToWifi();

  WiFiClient espClient;
  PubSubClient client(espClient);
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    if (!client.connect("esp32garage")) {
      Serial.print("Failed to connect to mqtt");
    }
    delay(1000);
  }

  int was_open = 0;
  while (digitalRead(GARAGE_REED)) {
    was_open = 1;
    Serial.println("Garage open");
    if (!client.publish(garage_topic, "open", true)) {
      Serial.println("Failed to publish!");
      ESP.restart();
    }
    delay(2000);
  }

  if (was_open) {
    Serial.println("Garage closed");
  }

  if (!client.publish(garage_topic, "closed", true)) {
    Serial.println("Failed to publish!");
    ESP.restart();
  }

  // Allow buffers to flush, missing good functionality for htis
  delay(5000);

  esp_sleep_enable_ext0_wakeup(GARAGE_REED, 1);
  esp_deep_sleep_start();
}

void loop() {
  Serial.println("I shouldn't be called, rebooting");
  ESP.restart();
}
