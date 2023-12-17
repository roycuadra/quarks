/*
 Created by: Roy Cuadra
 Date: 12/02/2023
*/

#define BLYNK_TEMPLATE_ID "TMPL6W6xPxCgO"
#define BLYNK_TEMPLATE_NAME "Sample"
#define BLYNK_AUTH_TOKEN "EyCwbe3g5W0Urap7UzxxEUtR3HL0xYS3"

#include <ESP8266WiFi.h>
#include <Wire.h>
#include "MQ135.h"
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Arduino.h>

#define LENG 31
unsigned char buf[LENG];

float air_quality_ppm = 0;
int air_quality_aqi = 0;

const char *ssid = "freewifi"; // YOUR WIFI NAME HERE PLEASE DONT PUT ANY SPACE
const char *password = "qwerty123"; // YOUR WIFI PASSWORD HERE PLEASE DONT PUT ANY SPACE


#define BUZZER_PIN 4  // Pin where the Buzzer is connected

#define DHTPIN 5       // Pin where the DHT11 is connected
#define DHTTYPE DHT11   // Type of DHT sensor

DHT dht(DHTPIN, DHTTYPE);
MQ135 gasSensor(A0);  // Move the initialization outside the loop

void setup()
{
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
   
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
   
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(80);
  digitalWrite(BUZZER_PIN, LOW);
  delay(80);
  
  
  // Initialize Blynk with your Auth Token
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Serial.println("BLYNK CONNECTED");

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());

  // Start the DHT sensor
  dht.begin();

  delay(1000); // Allow some time for the MQ135 sensor to stabilize
}


void loop()
{

   // Turn the LED on
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);  // Wait for 1 second

  // Turn the LED off
  digitalWrite(LED_BUILTIN, LOW);
  delay(300);  // Wait for 1 second

  if (Serial.find(0x42))
  {
    Serial.readBytes(buf, LENG);

    if (buf[0] == 0x4d);
    {
      if (checkValue(buf, LENG))
      {
       
      }
    }
  }

  air_quality_ppm = gasSensor.getPPM();


 // Map PPM to estimated AQI (example linear mapping)
  air_quality_aqi = map(air_quality_ppm, 0, 30000, 0, 500);
  Serial.println("Air Quality Index:");
  Serial.println(air_quality_aqi);
   
  
 
  




  if (air_quality_aqi <= 150)
  {
    Serial.println("Good Air Quality");

    // Turn off the buzzer for good air quality
    digitalWrite(BUZZER_PIN, LOW);
  } 
  else if (air_quality_aqi <= 200)
  {
    Serial.println("Bad Air Quality");
    Blynk.logEvent("bad_air");

    // Make the buzzer pulse for bad air quality
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      delay(100);
    }
  }
  else
  {
    Serial.println("Extremely Bad Air Quality");
    Blynk.logEvent("bad_airs");

    // Make the buzzer pulse more intensely for extremely bad air quality
    for (int i = 0; i < 10; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50);
      digitalWrite(BUZZER_PIN, LOW);
      delay(50);
    }
  }
  


  static unsigned long OledTimer = millis();
  if (millis() - OledTimer >= 1000)
  {
    OledTimer = millis();

    

    

    // Read temperature and humidity from DHT sensor
    float temperature = dht.readTemperature();
     Serial.println("Temperature:");
     Serial.println(temperature);
    float humidity = dht.readHumidity();
     Serial.println("Humidity:");
     Serial.println(humidity);

    // Send DHT data to Blynk
    Blynk.virtualWrite(V4, air_quality_aqi);
    Blynk.virtualWrite(V5, temperature); // Temperature value
    Blynk.virtualWrite(V6, humidity);    // Humidity value
  }

  memset(buf, 0, LENG);  // Clear the buffer

  Blynk.run();
}

char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;

  for (int i = 0; i < (leng - 2); i++)
  {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1]))
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  return ((thebuf[3] << 8) + thebuf[4]);
}

int transmitPM2_5(unsigned char *thebuf)
{
  return ((thebuf[5] << 8) + thebuf[6]);
}

int transmitPM10(unsigned char *thebuf)
{
  return ((thebuf[7] << 8) + thebuf[8]);
}
