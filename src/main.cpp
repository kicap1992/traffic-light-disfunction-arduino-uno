#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial toEsp(2, 3); // RX, TX

//  green light on relay
const int relayPin_green = 5;
const int checkPLN_green = 11;
const int analogIn_green = A2;

//  yellow light on relay
const int relayPin_yellow = 6;
const int checkPLN_yellow = 12;
const int analogIn_yellow = A1;

//  red light on relay
const int relayPin_red = 7;
const int checkPLN_red = 13;
const int analogIn_red = A0;

const int mVperAmp = 66;    // 66 mV per Amp for ACS712-30A
const int ACSoffset = 2500; // Offset for zero current (mV)

float amplitude_current = 0.0; // Amplitude current
float effective_value = 0.0;   // Effective current (RMS)

int getMaxValue(int duration, int analogPin)
{
  int sensorValue; // Value read from the sensor
  int sensorMax = 0;
  uint32_t start_time = millis();

  while ((millis() - start_time) < duration)
  {
    sensorValue = analogRead(analogPin);
    if (sensorValue > sensorMax)
    {
      sensorMax = sensorValue; // Record the maximum sensor value
    }
  }
  return sensorMax;
}

void controlRelay(int relayPin, int checkPLN, int analogIn , int duration)
{
  digitalWrite(relayPin, LOW);
  String relayStat;
  if (relayPin == relayPin_green)
  {
    relayStat = "Green";
    Serial.println("Green ON");
    digitalWrite(relayPin_yellow, HIGH);
    digitalWrite(relayPin_red, HIGH);
  }
  else if (relayPin == relayPin_yellow)
  {
    relayStat = "Yellow";
    Serial.println("Yellow ON");
    digitalWrite(relayPin_green, HIGH);
    digitalWrite(relayPin_red, HIGH);
  }
  else if (relayPin == relayPin_red)
  {
    relayStat = "Red";
    Serial.println("Red ON");
    digitalWrite(relayPin_green, HIGH);
    digitalWrite(relayPin_yellow, HIGH);
  }
  for (int i = 0; i < duration; i++)
  {
    int ii = duration - i;
    String pln;
    byte x = digitalRead(checkPLN);
    Serial.println(relayStat);
    if (x == 0)
    {
      Serial.println("PLN ON");
      pln = "PLN ON";
    }
    else
    {
      Serial.println("PLN OFF");
      pln = "PLN OFF";
    }

    int sensorMax = getMaxValue(1000, analogIn); // Get max value over 1 second

    float voltage = (sensorMax / 1024.0) * 5000.0; // Convert to millivolts
    amplitude_current = (voltage - ACSoffset) / mVperAmp;
    effective_value = amplitude_current / 1.414; // RMS calculation

    Serial.print("Current Max (A) : ");
    Serial.println(amplitude_current, 3);
    Serial.print("Current RMS (A) : ");
    Serial.println(effective_value, 3);
    Serial.println();

    toEsp.print(relayStat + "," + pln + "," + String(amplitude_current, 3) + "," + String(effective_value, 3) + "," + String(voltage, 3) +"," +ii+"\n");
    // delay(1000); // Delay between readings
  }
}

void setup()
{
  pinMode(relayPin_green, OUTPUT);
  pinMode(analogIn_green, INPUT);
  pinMode(checkPLN_green, INPUT);

  pinMode(relayPin_yellow, OUTPUT);
  pinMode(analogIn_yellow, INPUT);
  pinMode(checkPLN_yellow, INPUT);

  pinMode(relayPin_red, OUTPUT);
  pinMode(analogIn_red, INPUT);
  pinMode(checkPLN_red, INPUT);
  Serial.begin(115200);
  toEsp.begin(9600);
  digitalWrite(relayPin_green, HIGH);  // Ensure relay starts OFF
  digitalWrite(relayPin_yellow, HIGH); // Ensure relay starts OFF
  digitalWrite(relayPin_red, HIGH);    // Ensure relay starts OFF
}

void loop()
{
  controlRelay(relayPin_green, checkPLN_green, analogIn_green , 30); //60 - 90 detik
  controlRelay(relayPin_yellow, checkPLN_yellow, analogIn_yellow ,  5); // 3- 5 detik
  controlRelay(relayPin_red, checkPLN_red, analogIn_red, 30);  //30 - 60 detik
}
