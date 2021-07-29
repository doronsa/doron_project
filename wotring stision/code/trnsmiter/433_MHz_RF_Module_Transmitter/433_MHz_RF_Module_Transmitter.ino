/*
  433 MHz RF Module Transmitter Demonstration 1
  RF-Xmit-Demo-1.ino
  Demonstrates 433 MHz RF Transmitter Module
  Use with Receiver Demonstration 1

  DroneBot Workshop 2018
  https://dronebotworkshop.com
*/
// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>
// Include dependant SPI Library 
#include <SPI.h> 
//const char *msg;
char msg[15];
// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

int sensorTEMP = A0;    // select the input pin for the potentiometer
int sensorHOMEDTY = A1;  
int sensorV = A2;  
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

//for test conversion
//float sensorTMP1=10.0;
//float sensorTMP2=10.0;
//int sensorTMP3=20;
//float sensorTMP4=0.0;
//float sensorTMP5=0.0;

int sensorTMP1=10;
int sensorTMP2=20;
int sensorTMP3=30;
int sensorTMP4=0;
int sensorTMP5=0;




void TestSansor(void)
{
  //convert the sensor to float and convert the float char
  //int reading = analogRead(sensorTEMP);

  //int reading = 253;//analogRead(sensorTEMP);
  //float voltage = reading * 1.0;
  //voltage /= 1024.0;  
  //float  temperatureC = (sensorTMP1) / 10;
//  char buffer[15];
//  String tem1 = dtostrf(sensorTMP1, 5, 1, buffer);
//  //memcpy (msg[0],buffer,10);
//  msg[0] = sensorTMP1;//buffer;
//  //Serial.println(msg); 
//  
//  String tem2 = dtostrf(sensorTMP2, 5, 1, buffer);
//  //memcpy (msg[3],buffer,10);
//  msg[3] =buffer;
//  //Serial.println(msg); 
//  
//  String tem3 = dtostrf(sensorTMP3, 5, 0, buffer);
//  //memcpy (msg[6],buffer,10);
//  msg[6] =buffer;
//  //Serial.println(msg); 
//  
//  String tem4 = dtostrf(sensorTMP4, 5, 1, buffer);
//  //memcpy (msg[8],buffer,10);
// // msg[3] =buffer;
//  msg[8] =buffer;
//  //Serial.println(msg); 
//  
//  //memcpy (msg[12],buffer,10);
//  
//  String tem5 = dtostrf(sensorTMP5, 5, 1, buffer);
//  msg[11] =buffer;
//  //Serial.println(msg); 
  
  //String temper = (tem1+tem2+tem3+tem4+tem5);
  sprintf(msg,"%d%d%d%d%d",sensorTMP1*10,sensorTMP2*10,sensorTMP3,sensorTMP4*10,sensorTMP5*10);
  //Serial.print("test Value: ");
  //Serial.println(temper); 
  Serial.print("Mssg Value: ");
  Serial.println(msg); 
}

void BuildMssg(void) 
{
//  // read the value from the sensor:
//  sensorTEMP++ ;//analogRead(sensorPin);
//  sensorHOMEDTY=sensorHOMEDTY+4;
//  sensorV=sensorV+2 ;
//  // turn the ledPin on
//  digitalWrite(ledPin, HIGH);
//  // stop the program for <sensorValue> milliseconds:
//  delay(sensorTEMP);
//  // turn the ledPin off:
//  digitalWrite(ledPin, LOW);
//  // stop the program for for <sensorValue> milliseconds:
//  delay(sensorTEMP);
  
  //sprintf(msg,"Temp %d Homdty %d snsVOlt %d",sensorTEMP,sensorHOMEDTY,sensorV);
  sprintf(msg,"%d%d%d",sensorTEMP,sensorHOMEDTY,sensorV);
  Serial.print("sensor Value: ");
  Serial.println(sensorTEMP); 
  Serial.println();
  Serial.print("Message Received:-");
  Serial.print(msg);
  Serial.println();
  //delay(1000);
}


void setup()
{
    // Initialize ASK Object
    rf_driver.init();
    //for test
    pinMode(ledPin, OUTPUT);
    Serial.begin(9600);
}




void loop()
{
  //for test
  //BuildMssg();
  TestSansor();
    //const char *msg = sensorValue;//"Welcome to the Workshop!";
    //sprintf(msg,"Temprtor&d",sensorValue);
    rf_driver.send((uint8_t *)msg, strlen(msg));
    rf_driver.waitPacketSent();
    delay(1000);
}
