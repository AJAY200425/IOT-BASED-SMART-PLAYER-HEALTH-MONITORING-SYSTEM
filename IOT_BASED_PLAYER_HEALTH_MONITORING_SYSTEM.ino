#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
float A ;
float temperature;
float x;// always include thingspeak header file after other header files and custom macros
char* ssid = "NOTHING";   // your network SSID (name)
char* pass = "12345678";   // your network password
int keyIndex = 0;
int i = 0;// your network key Index number (needed only for WEP)
WiFiClient  client;
MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
float tem;
int beatAvg;
unsigned long myChannelNumber =1725144;
const char * myWriteAPIKey = "FN19XCWU7Z5SQTS4";
String myStatus = "";
void setup() {
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
 
  WiFi.mode(WIFI_STA);  
  ThingSpeak.begin(client);
  Serial.println("Initializing...");
 
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
 
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED// Initialize ThingSpeak
}

void loop() {
 

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);    
    }
    Serial.println("\nConnected.");
  }
  for (i=0;i<700;i++){
    long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
     
  float temperature = particleSensor.readTemperature();
 
    }
  }
  tem = random(34,39);
 
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  x = map(beatsPerMinute,0,150,70,95);
  Serial.print(",Exact heart beat =");
  Serial.print(x);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();}

  // set the fields with the values
  ThingSpeak.setField(6, x );
  ThingSpeak.setField(7, tem );


 
  // set the status
  ThingSpeak.setStatus(myStatus);
 
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
 
 
  delay(15000); // Wait 20 seconds to update the channel again
}
