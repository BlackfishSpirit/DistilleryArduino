//Scale
//pulled from https://www.instructables.com/Arduino-Scale-With-5kg-Load-Cell-and-HX711-Amplifi/

#include "HX711.h"

#define calibration_factor 450//510 //This value is obtained using ScaleCalibration

#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2

#define BUZZER_PIN 11
#define LED_PIN 12
#define BUTTON_PIN 8

HX711 scale;

float midLimit = 1700;
float upperMidLimit = 2000;
float limit = 2200;
float mass = 0;

unsigned long lastTime = millis();

bool buzzing = true;
//float buzzTime = 500;
float buzzTimer = 0;
int buzzCount = 0;

bool snoozed = false;

bool overLimit = false;

void buzz(int buzzTime, unsigned long deltaTime)
{
  buzzTimer += deltaTime;
  if(buzzTimer > buzzTime)
  {
    buzzTimer = 0;
    buzzing = !buzzing;

    if(buzzing)
      tone(BUZZER_PIN, 1000);
    else noTone(BUZZER_PIN);
  }
  
}

void buzzMidway(int timer, int frequency, unsigned long deltaTime)
{
  buzzTimer += deltaTime;
  if(buzzTimer > timer)
  {
    buzzTimer = 0;
    buzzing = !buzzing;
    
    if(buzzing)
    {
      tone(BUZZER_PIN, frequency);
    }
    else 
    {
      noTone(BUZZER_PIN);
      buzzCount += 1;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 scale demo");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  Serial.println("Readings:");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  
  mass = scale.get_units();
  
  Serial.print("Reading: ");
  Serial.print(mass, 1); //scale.get_units() returns a float
  Serial.print(" g"); //You can change this to kg but you'll need to refactor the calibration_factor
  Serial.println();

  unsigned long thisTime = millis();
  unsigned long deltaTime = thisTime - lastTime;
  lastTime = thisTime;

  if(mass > midLimit)
  {
    if(buzzCount < 5)
    {
      buzzMidway(500, 500, deltaTime);
    }
    else if(buzzCount < 10 && mass > upperMidLimit)
    {
      buzzMidway(400, 750, deltaTime);
    }
  }
  else buzzCount = 0;

  if(mass > limit)
  {
    overLimit = true;
    
    if(!snoozed)
    {
      
      int buzzTime = 500 + (2250 - int(mass));
      buzz(buzzTime, deltaTime);
  
      if(digitalRead(BUTTON_PIN) == LOW)
      {
        snoozed = true;
        noTone(BUZZER_PIN);
      }
    }
  }
  else 
  {
    if(overLimit)
    {
      overLimit = false;
      noTone(BUZZER_PIN);
      buzzing = true;
      buzzTimer = 0;
    }

    snoozed = false;
  }
}
