/*
  433 MHz RF Module Receiver Demonstration 1
  RF-Rcv-Demo-1.ino
  Demonstrates 433 MHz RF Receiver Module
  Use with Transmitter Demonstration 1

  DroneBot Workshop 2018
  https://dronebotworkshop.com
*/

// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>
// Include dependant SPI Library 
#include <SPI.h> 

int sensorTMP1;
int sensorTMP2;
int sensorTMP3;
int sensorTMP4;
int sensorTMP5;
// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

void setup()
{
    // Initialize ASK Object
    rf_driver.init();
    // Setup Serial Monitor
    Serial.begin(9600);
}

void loop()
{
    // Set buffer to size of expected message
    char msg[30];
    uint8_t buf[20];
    uint8_t buflen = sizeof(buf);
    // Check if received packet is correct size
    if (rf_driver.recv(buf, &buflen))
    {
       Serial.print("Message Received: ");
       Serial.print((int)buf[0]);
       Serial.print(" ");
       sensorTMP1 = (int)buf[0];
       Serial.print((int)buf[3]);
       Serial.print(" ");
       sensorTMP2 = (int)buf[3];
       Serial.print((int)buf[6]);
       Serial.print(" ");
       sensorTMP3 = (int)buf[6];
       sensorTMP4 = (int)buf[8];
       sensorTMP5 = (int)buf[11];
          
      //sprintf(msg,"Temp:%d% Hmid d%, v, d,%d"(sensorTMP1,sensorTMP2,sensorTMP2,sensorTMP4,sensorTMP5); );
      
      // Message received with valid checksum
//      Serial.print("Message Received: ");
//      Serial.print("TEMP ");
//      Serial.print(sensorTMP1);
//      Serial.print(" hm ");
//      Serial.print(sensorTMP2);
//      Serial.print(" V ");
//      Serial.print(sensorTMP3);
//      Serial.println();        
    }
}
