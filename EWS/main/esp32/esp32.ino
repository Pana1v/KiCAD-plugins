#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT11.h>

DHT11 dht11(22);
#define d_tiltSensorPIN 4
#define d_sim808_relay_pin 5  // for no circuits in relay this should have 1 at all time. So it is recommented to set this at 1 at initialisation.
#define d_sim808_startup_PIN 7
#define d_led_PIN 2

#define soilMositureSensorPIN 18
// #define TdsSensorPin A1
// #define Turbidity_Sensor_Pin A2
// #define pH_Sensor_Pin A3
#define capacitiveSoilMositureSensor 19
void (*resetFunc)(void) = 0;

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
 
bool readDHT11Data(float& temperature, float& humidity) {
  temperature = dht11.readTemperature();


    // If using ESP32 or ESP8266 (xtensa architecture), uncomment the delay below.
    // This ensures stable readings when calling methods consecutively.
    // delay(50);

     humidity = dht11.readHumidity();


  Serial.print("temperature: ");
  Serial.println(temperature);

  Serial.print("humidity: ");
  Serial.println(humidity);

  return true;
}

///////////////////////////////////////
///////// SENSOR  FUNCTIONS ///////////
///////////////////////////////////////

bool readTiltSensorData(float& value) {
  int read = digitalRead(d_tiltSensorPIN);
  if (read == HIGH) {
    value = 1.0;
  }

  if (read == LOW) {
    value = 0.0;
  }
  Serial.print("Tilt: ");
  Serial.println(!value);
  return true;
}

// bool readCapacitiveSoilMoistureSensorData(float& value) {
//   int val = analogRead(capacitiveSoilMositureSensor);
//   value = (float)val;
//   Serial.print("CapacitiveSoilMoistureSensorData: ");
//   Serial.println(value);
//   return true;
// }

bool readTraditionalSoilMoistureSensorData(float& value) {
  int val = analogRead(soilMositureSensorPIN);
  value = (float)val;
  Serial.print("TraditionalSoilMoistureSensorData: ");
  Serial.println(val);
  return true;
}

// bool readpHData(float& ph_act) {
//   float calibration_value = 21.34;
//   int phval = 0;
//   unsigned long int avgval;
//   int buffer_arr[10], temp;

//   for (int i = 0; i < 10; i++) {
//     buffer_arr[i] = analogRead(pH_Sensor_Pin);
//     delay(30);
//   }
//   for (int i = 0; i < 9; i++) {
//     for (int j = i + 1; j < 10; j++) {
//       if (buffer_arr[i] > buffer_arr[j]) {
//         temp = buffer_arr[i];
//         buffer_arr[i] = buffer_arr[j];
//         buffer_arr[j] = temp;
//       }
//     }
//   }
//   avgval = 0;
//   for (int i = 2; i < 8; i++)
//     avgval += buffer_arr[i];
//   float volt = (float)avgval * 5.0 / 1024 / 6;
//   ph_act = -5.70 * volt + calibration_value;

//   Serial.print("pH: ");
//   Serial.println(ph_act);

//   return true;
// }

// bool readTurbidityData(float& ntu) {
//   float Turbidity_Sensor_Voltage = 0;
//   int samples = 600;
//   for (int i = 0; i < samples; i++) {
//     Turbidity_Sensor_Voltage += ((float)analogRead(Turbidity_Sensor_Pin) / 1023) * 5;
//   }

//   Turbidity_Sensor_Voltage = Turbidity_Sensor_Voltage / samples;
//   Turbidity_Sensor_Voltage = round_to_dp(Turbidity_Sensor_Voltage, 2);
//   if (Turbidity_Sensor_Voltage < 2.5) {
//     ntu = 3000;
//   } else {
//     ntu = -1120.4 * square(Turbidity_Sensor_Voltage) + 5742.3 * Turbidity_Sensor_Voltage - 4352.9;
//   }

//   Serial.print("turbidity: ");
//   Serial.println(ntu);

//   return true;
// }

// float round_to_dp(float in_value, int decimal_place) {
//   float multiplier = powf(10.0f, decimal_place);
//   in_value = roundf(in_value * multiplier) / multiplier;
//   return in_value;
// }

// bool readTDSData(float& tdsValue) {
//   gravityTds.update();
//   tdsValue = gravityTds.getTdsValue();

//   Serial.print("tds: ");
//   Serial.println(tdsValue);

//   return true;
// }

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
// void publishMessage()
// {
//   StaticJsonDocument<200> doc;
//   doc["a"] = b;
//   doc["temperature"] = t;
//   char jsonBuffer[512];
//   serializeJson(doc, jsonBuffer); // print to client
 
//   client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
// }
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}
 
void setup()
{
  Serial.begin(115200);
  connectAWS();
  // dht.begin();
}
 
void loop()
{
 
  StaticJsonDocument<200> doc;
 float temperature, humidity, tdsValue, ntu, ph_act, traditional_moisture_sensor_data, capacitive_soil_moisture_sensor_data, tilt;
// float traditional_moisture_sensor_data, capacitive_soil_moisture_sensor_data, tilt;

if (readDHT11Data(temperature, humidity) && readTraditionalSoilMoistureSensorData(traditional_moisture_sensor_data) && readTiltSensorData(tilt)) {
  if (readTraditionalSoilMoistureSensorData(traditional_moisture_sensor_data) && readTiltSensorData(tilt)) {
    // Update doc directly
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    // doc["capacitive_soil_moisture_sensor_data"] = ;
    doc["traditional_moisture_sensor_data"] = traditional_moisture_sensor_data;
    doc["tilt"] = tilt;
    char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  client.loop();
  }
}

  

  delay(5000);
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C "));
 
 
  // delay(1000);
}