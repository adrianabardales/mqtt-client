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

void setup()
{
  Serial.begin(115200);
  delay(1000);

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

void loop()
{
  if (!client.connected()) {
    connectToThinger();
  }
  client.loop();

  delay(2000);
}