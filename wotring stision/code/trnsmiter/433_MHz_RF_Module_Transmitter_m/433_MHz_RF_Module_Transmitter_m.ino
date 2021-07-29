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

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;
double TempSensor = 10;
double Humidity = 30;
double Pressure = 40;
int Volt = 50;
int Spaer = 60;



void setup()
{
    // Initialize ASK Object
    rf_driver.init();
    Serial.begin(9600);
}

void loop()
{
    char msg[31];
    String  temp;
    int i=0;
    TempSensor = TempSensor+20;
    Humidity=Humidity+20;
    Pressure=Pressure+20;
    Volt=Volt+20;
    Spaer=Spaer+20;

    Serial.print("Message Trensmit: ");
    temp = String(TempSensor)+","+ String(Humidity)
    +","+ String(Pressure)+","+ String(Volt)+ ","+String(Spaer)+","+"/O";
    Serial.println(temp);         
    temp.toCharArray(msg, 31);
    Serial.print("Message Len: ");
    Serial.println(strlen(msg));
    rf_driver.send((uint8_t *)msg, strlen(msg));
    //rf_driver.send((uint8_t *)msg, strlen(msg));
    rf_driver.waitPacketSent();
    delay(1000);
}
