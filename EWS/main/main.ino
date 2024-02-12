#include <SoftwareSerial.h>
#include <ArduinoJson.h>
// #include "GravityTDS.h"
#include <DHT11.h>

///////////////////////////////////////
//// INITIALISE NECESSARY VARIABLES ///
///////////////////////////////////////

DHT11 dht11(2);
#define d_tiltSensorPIN 3
#define d_sim808_relay_pin 5  // for no circuits in relay this should have 1 at all time. So it is recommented to set this at 1 at initialisation.
#define d_sim808_startup_PIN 7
#define d_led_PIN 13

#define soilMositureSensorPIN A0
// #define TdsSensorPin A1
// #define Turbidity_Sensor_Pin A2
// #define pH_Sensor_Pin A3
#define capacitiveSoilMositureSensor A4
void (*resetFunc)(void) = 0;

SoftwareSerial myserial(10, 11);
// dht11 DHT11;
// GravityTDS gravityTds;

// GPRS settings
const char* GPRS_APN = "airtelgprs.com";
const char* GPRS_USER = "";
const char* GPRS_PASS = "";

int failed = 0;

///////////////////////////////////////
/////////// INIT FUNCTIONS ////////////
///////////////////////////////////////

void sim808StartupSequence() {
  // realy pin only needs to be put to
  // ground then set to high for rest of the time.
  digitalWrite(d_sim808_relay_pin, LOW);
  delay(1000);
  digitalWrite(d_sim808_relay_pin, HIGH);

  delay(5000);

  digitalWrite(d_sim808_startup_PIN, HIGH);
  delay(1000);
  digitalWrite(d_sim808_startup_PIN, LOW);
}

void initSerials() {
  myserial.begin(9600);
  Serial.begin(9600);
  pinMode(d_led_PIN, OUTPUT);
  pinMode(d_tiltSensorPIN, INPUT);
  pinMode(d_sim808_relay_pin, OUTPUT);
  pinMode(d_sim808_startup_PIN, OUTPUT);
}

void initSensors() {
  // gravityTds.setPin(TdsSensorPin);
  // gravityTds.setAref(5.0);
  // gravityTds.setAdcRange(1024);
  // gravityTds.begin();
  Serial.println("Initializing..........");
  delay(1000);
}

void initGPRS() {
  myserial.println("AT");
  delay(1000);

  myserial.println("AT+CGATT=1");  // Attach to GPRS network
  delay(2000);
}

///////////////////////////////////////
/////////////// SETUP /////////////////
///////////////////////////////////////

void setup() {
  initSerials();
  sim808StartupSequence();
  initSensors();
  initGPRS();
}

///////////////////////////////////////
/////////////// LOOP //////////////////
///////////////////////////////////////

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
  Serial.println(value);
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
  Serial.println(value);
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

///////////////////////////////////////
/////////// GPRS FUNCTIONS ////////////
///////////////////////////////////////

bool connectToGPRS() {
  myserial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  delay(2000);
  myserial.println(String("AT+SAPBR=3,1,\"APN\",\"") + GPRS_APN + "\"");
  delay(2000);
  myserial.println("AT+SAPBR=1,1");
  delay(2000);
  return true;  // Return true if connection successful, false otherwise
}

bool sendHTTPPostRequest(const String& data) {
  bool status;
  myserial.println("AT+HTTPINIT");
  delay(2000);
  IgnoreSerialData();
  myserial.println("AT+HTTPPARA=\"CID\",1");
  delay(2000);
  IgnoreSerialData();
  myserial.print("AT+HTTPPARA=\"URL\",\"");
  myserial.print("http://34.93.25.177:7001/api/data");
  myserial.println("\"");
  delay(2000);
  IgnoreSerialData();
  myserial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  delay(2000);
  IgnoreSerialData();
  myserial.print("AT+HTTPDATA=");
  myserial.print(data.length());
  myserial.println(",10000");
  delay(2000);
  IgnoreSerialData();
  myserial.println(data);
  delay(2000);
  IgnoreSerialData();
  myserial.println("AT+HTTPACTION=1");
  delay(2000);
  ReadSerialData(status);  // reading the response here... this wil put status in the status
  myserial.println("AT+HTTPREAD");
  delay(2000);
  IgnoreSerialData();
  myserial.println("AT+HTTPTERM");
  delay(4000);
  IgnoreSerialData();
  return status;
}

void IgnoreSerialData() {
  while (myserial.available() != 0) {
    myserial.read();
  }
}

void SpeakSerialData() {
  while (myserial.available() != 0)
    Serial.write(myserial.read());
  delay(1000);
}

void ReadSerialData(bool& status) {
  String response = "";
  while (myserial.available() != 0) {
    char c = myserial.read();
    response += c;
  }

  Serial.println(response);
  // Check if the response contains "HTTPACTION: 1,"
  if (response.indexOf("HTTPACTION: 1,") != -1) {
    // Find the HTTP status code within the response
    int statusCodeIndex = response.indexOf("HTTPACTION: 1,") + 14;  // Length of "HTTPACTION: 1,"
    String statusCodeStr = response.substring(statusCodeIndex, statusCodeIndex + 3);
    // ort delay before D9 toggle
    // Convert the status code to an integer
    int statusCode = statusCodeStr.toInt();

    // Check if the status code is 201
    if (statusCode == 200) {
      Serial.println("hurray");
      status = true;
    } else {
      Serial.println("noo");
      status = false;
    }
  } else {
    Serial.println("noo");
    status = false;
  }
}

void disconnectFromGPRS() {
  myserial.println("AT+SAPBR=0,1");
  delay(2000);
}

void sendData(const int sensor_id, float value) {
  if (connectToGPRS()) {
    StaticJsonDocument<256> doc;
    doc["ext_id"] = "device_water";
    doc["p"] = "device_water";
    doc["sensor_id"] = sensor_id;
    doc["value"] = value;

    String sendtoserver;
    serializeJson(doc, sendtoserver);

    if (sendHTTPPostRequest(sendtoserver)) {
      Serial.println("Data sent successfully");
      // glow led
      fastBlinkLed();

    } else {
      Serial.println("Failed to send data");
      slowBlinked();
      failed++;
      Serial.print("failed ");
      Serial.println(failed);
    }


    disconnectFromGPRS();
  } else {
    Serial.println("Failed to connect to GPRS");
  }
}

///////////////////////////////////////
/////// LIVE ERROR VISUALISER /////////
///////////////////////////////////////

void fastBlinkLed() {
  digitalWrite(d_led_PIN, HIGH);
  delay(10);
  digitalWrite(d_led_PIN, LOW);
}

void slowBlinked() {
  digitalWrite(d_led_PIN, HIGH);
  delay(1000);
  digitalWrite(d_led_PIN, LOW);
}
void loop() {
  if (myserial.available()) {
    Serial.write(myserial.read());
  }
  float temperature, humidity, tdsValue, ntu, ph_act, traditional_moisture_sensor_data, capacitive_soil_moisture_sensor_data, tilt;
  // float traditional_moisture_sensor_data, capacitive_soil_moisture_sensor_data, tilt;
  if (readDHT11Data(temperature, humidity) &&  readTraditionalSoilMoistureSensorData(traditional_moisture_sensor_data)  && readTiltSensorData(tilt)) {
    // && readCapacitiveSoilMoistureSensorData(capacitive_soil_moisture_sensor_data)  && readTDSData(tdsValue)
  if (readTraditionalSoilMoistureSensorData(traditional_moisture_sensor_data) &&  readTiltSensorData(tilt)) {
    sendData(4, temperature);
    //readCapacitiveSoilMoistureSensorData(capacitive_soil_moisture_sensor_data) &&
    sendData(5, humidity);
    // sendData(1, tdsValue);
    // sendData(3, ph_act);
    // sendData(2, ntu);
    sendData(11, capacitive_soil_moisture_sensor_data);
    sendData(12, traditional_moisture_sensor_data);
    sendData(13, tilt);
    if (failed >= 10) {
      Serial.println(failed);
      sim808StartupSequence();
      failed = 0;
    }
  }
  // delay(1000);
  delay(180000);  // Delay between readings
  }
}