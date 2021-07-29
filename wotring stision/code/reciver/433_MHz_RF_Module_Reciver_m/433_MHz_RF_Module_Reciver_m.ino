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

// Create Amplitude Shift Keying Object
RH_ASK rf_driver;

char* TempSensor;
char* Humidity;
char* Pressure;
char* Volt;
char* Spaer;



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
    uint8_t buf[25];
    uint8_t buflen = sizeof(buf);
    // Check if received packet is correct size
    if (rf_driver.recv(buf, &buflen))
    {
      
      // Message received with valid checksum
      Serial.print("Message Received: ");
      Serial.println((char*)buf);  
      //strncpy(TempSensor,buf,2);
      //Serial.println(TempSensor); 
      TempSensor = strtok(buf, ",");
      //char* eastSegment = strtok(NULL, ",");
      char* Humidity = strtok(NULL, ",");
      char* Pressure = strtok(NULL, ",");
      char* Volt = strtok(NULL, ",");
      char* Spaer = strtok(NULL, ",");
        Serial.println("Message Received: ");
        Serial.print("Temp: ");
        Serial.println(TempSensor);
        Serial.print("Humidity: ");
        Serial.println(Humidity);
        Serial.print("Pressure: ");
        Serial.println(Pressure);
        Serial.print("Volt: ");
        Serial.println(Volt);
        Serial.print("Spaer: ");
        Serial.println(Spaer);

        
      
    }
}
