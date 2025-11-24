// Source : http://www.iotsharing.com/2017/05/tcp-udp-ip-with-esp32.html
// Source : https://www.dfrobot.com/blog-948.html

#include <Arduino.h>
#include "secrets.h"
#include "WiFi.h"
#include <PubSubClient.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

const char *host = "maisonneuve.aws.thinger.io";
const int port = 1883;

//credentials for thinger.io
const char *thinger_username = THINGER_USERNAME;
const char *thinger_device_id = THINGER_DEVICE_ID;
const char *thinger_device_credential = THINGER_DEVICE_CREDENTIAL;

// ---- HC-SR04 configuration ----
const int TRIG_PIN = 2; // trigger -> GPIO2
const int ECHO_PIN = 3; // echo    -> GPIO3

WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

PubSubClient client(wifiClient);

void connectToThinger() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection to thinger.io...");
    
    // Create MQTT client ID
    String clientId = thinger_device_id;
    
    // Connect with username and password
    if (client.connect(clientId.c_str(), thinger_username, thinger_device_credential)) {
      Serial.println("connected");
      
      // Subscribe to device commands topic
      String subscribe_topic = String(thinger_username) + "/devices/" + thinger_device_id + "/cmd";
      client.subscribe(subscribe_topic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String translateEncryptionType(wifi_auth_mode_t encryptionType)
{

  switch (encryptionType)
  {
  case (WIFI_AUTH_OPEN):
    return "Open";
  case (WIFI_AUTH_WEP):
    return "WEP";
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
  default:  
    return "UNKNOWN";
  }
  return "UNKNOWN";
}

void scanNetworks()
{

  int numberOfNetworks = WiFi.scanNetworks();

  // Add delay so the terminal can catch up
  delay(3000);

  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);

  for (int i = 0; i < numberOfNetworks; i++)
  {

    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));

    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));

    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));

    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("-----------------------");
  }
}

void connectToNetwork()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to network");
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

float measureDistanceCM() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse duration (timeout 30ms -> ~5 meters)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1.0f;

  // Convert to centimeters: speed of sound ~343 m/s -> 0.0343 cm/us
  float distanceCm = (duration * 0.0343f) / 2.0f;
  return distanceCm;
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // configure HC-SR04 pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // Print MAC address
  Serial.println("MCU MAC address: " + WiFi.macAddress());

  scanNetworks();
  connectToNetwork();

  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());

  // configure MQTT client
  client.setServer(host, port);
  client.setCallback(callback);
}

unsigned long last = 0;

void loop()
{
  if (!client.connected()) {
    connectToThinger();
  }
  client.loop();

  unsigned long now = millis();
  float dist = measureDistanceCM();
  if (dist >= 0.0f && now - last > 2000) {
    Serial.print("Distance: ");
    Serial.print(dist);
    Serial.println(" cm");
    last = now;

    String topic = "Proximity";
    String payload = String(dist);
    client.publish(topic.c_str(), payload.c_str());
  } 

}