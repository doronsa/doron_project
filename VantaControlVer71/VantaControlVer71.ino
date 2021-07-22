/******************************************************************
 Created with PROGRAMINO IDE for Arduino - 24/05/2020 17:48:03
 Project     :
 Libraries   :
 Author      :
 Description :
******************************************************************/
/*   Venta Control 
 * Doron Sandroy
 * version 7.2
 * BTN - From the Wolt 
 *       From The BOX Start Venta
 *                    Start HIT
 *                    Stop ALL
 *                    Sabat MODE 
 *  The Wolt BTN is from transistor 
 *             //test the time if the time is 12 nite than clase all relay
           //than on the mornig set up agin on 8 a clock
           /*
            hour();            // the hour now  (0-23)
            minute();          // the minute now (0-59)          
            second();          // the second now (0-59) 
            day();             // the day now (1-31)
            weekday();         // day of the week, Sunday is day 0 
            month();           // the month now (1-12)
            year();            // the full four digit year: (2009, 2010 etc) 
            **************************************************************    
            *    add new protocol from the wifi card to the arduino system cpu
            *    ADD NEW UART --
            *    the command :
            *    --------------
            *    FROM MAIN CPU
            *    ---------------
            *    GET_TIME = 110
            *    SET_TIME = 120
            *    GET_STATUS = 130
            *    ------------
            *    FROM WIFI CPU
            *    -------------
            *    SET_VENTA_RELAY = 210
            *    STOP_VENTA_RELAY = 220
            *    SET_HIT_RELAY = 230
            *    STOP_HIT_RELAY = 240
            *    STOP_ALL = 250  
            DOTO ------
            1. ADD NEW UART
            2. ADD NEW switch  protocol
            3. add new btn for the SABAT sw
            -----------------------------
            on the wifi cpu
            1. add all need for the protocol     
            2. add all command from up (from the arduino cpu)   
            3. add all need for WIFI connceted and get from the alaxa command
               for on / off the relays.
            
            
*/           
/*
 * Test Venta System
 * the test is
 * 1. sw test
 * 2. led test
 * 3. lcd test
 * 4. com test
 */
//include 

#include <TimeLib.h>
#include <TimeAlarms.h> 
//#include <ESP8266WiFi.h>
#include <time.h>
#include <DS3232RTC.h>      // https://github.com/JChristensen/DS3232RTC

//
#define STOP_ALL 1
#define VENTA 2
#define HIT 3
#define SABAT 4
#define SABAT_CLOSE 5
#define WOLT_POWER_VENTA 6
#define OFF 0
#define ON 1
//
#include "U8glib.h"
U8GLIB_ST7920_128X64_4X u8g(13,11,10,U8G_PIN_NONE);
// PIN asimnt
//button setup
const int  buttonStop = 6;    
const int  buttonStartVanta = 7;  
const int  buttonSABAT = 9;  
const int  buttonStartHit = 8;
//-------------------------------
const int  WoltStartVanta = A2;  
 
//-------------------------------
//led setup
const int ledPinHit = 3;
const int ledPinVanta = 4;
const int ledPinSabat = 2;
const int ledPinPower = 5;//ok
//-------------------------------
//relay setup
const int  realyVanta = A0;
const int  realyHit = A1;   
//-------------------------------
//   var
char buttonStateStop = 0;
char buttonStateVenta = 0;
char buttonStateVentaWolt = 0;
char buttonStateHit = 0;
char buttonStateSABAT = 0;
char lastButtonStateStop = 0; 
char lastButtonStateWolt = 0; 
//------------------------------
int TimeCount =0;
char TimeCountStatus =0;
char status_led = 0;
char VentaWolt =0;
char StatusWorgink = 0;
//------------------------------
int HITtime = 60*15;//20 sec
int VENTAtime = 60*30;//30 sec 
int SabatOntime = 60*20;// sec   
int SabatClosetime = 60*15;// sec
//*********for debug*********
//int HITtime = 60*1;//20 sec
//int VENTAtime = 60*1;//30 sec
//int SabatOntime = 60*1;//30 sec   
//int SabatClosetime = 60*1;//30 sec
//*********for debug********* 
int BTNStatus =0; 
int SabatSatus = 0;
int BTNStatusOLD = 0;
//Display Data
char strdata1[16]="Vanta Control";//line 1
char strdata2[16];//line 2
char strdata3[16];//line 3
char strdata4[16];//line 4
//const


//fnq test
void printDateTime(time_t t);

void draw(void) 
{
  // graphic commands to redraw the complete screen should be placed her 
  u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_osb21);
  u8g.drawStr( 0, 12, strdata1);
  u8g.drawStr( 0, 26, strdata2);
  u8g.drawStr( 0, 36, "-------------");
  u8g.drawStr( 0, 46, strdata3);
  u8g.drawStr( 0, 60, strdata4);
}

//****************************

//int timezone = 3;

AlarmId id;

int ReadBTN(void)
{
  int ReturnVal = 0;
//  buttonStateStop = 0;
//  buttonStateVenta = 0;
//  buttonStateVentaWolt = 0;
//  buttonStateHit = 0 ;
//  buttonStateSABAT = 0;
  
 // read the pushbutton input pin:
  buttonStateStop = digitalRead(buttonStop);
  buttonStateVenta = digitalRead(buttonStartVanta);
  buttonStateVentaWolt = digitalRead(WoltStartVanta);
  buttonStateHit = digitalRead(buttonStartHit);
  buttonStateSABAT = digitalRead(buttonSABAT);
                 
  Alarm.delay(500);

  if (buttonStateStop != lastButtonStateStop)
  {
      //Serial.println("Stop Button");      
      sprintf(strdata2, "Mode Stop");   
      lastButtonStateStop = buttonStateStop;
      ReturnVal = STOP_ALL;
  }else{
          if( (buttonStateVentaWolt != lastButtonStateWolt))// &&( SabatSatus == 0))
          {
                VentaWolt = 1;               
                sprintf(strdata2, "W Mode Venta");
                TimeCountStatus = 1;                   
//                Serial.print("Venta Wolt Start :");   
//                Serial.print(lastButtonStateWolt); 
//                Serial.println(buttonStateVentaWolt); 
                lastButtonStateWolt = buttonStateVentaWolt;  
                ReturnVal = WOLT_POWER_VENTA;
          
          }else
          {                            
                if ((buttonStateVenta == 1)&&( SabatSatus == 0))
                { 
//                        Serial.println("Venta Button");
                        ReturnVal = VENTA;
                        sprintf(strdata2, "Mode Venta");
                        TimeCountStatus = 1;
                        
                }else{
                        if (buttonStateHit == 1)// 
                        {
//                            Serial.println("HIT Button");
                            ReturnVal = HIT;
                            sprintf(strdata2, "Mode Hit");
                            TimeCountStatus = 1;
                        }else{
                                if (buttonStateSABAT == 1)
                                {
//                                    Serial.println("SABAT Button");
                                    ReturnVal = SABAT; 
                                    SabatSatus = 1;
                                    sprintf(strdata2, "Mode SABAT");
                                    TimeCountStatus = 1;
                                }else{
                                        ReturnVal = 0;
                                     }
                              }
                     
                  } 
                } 
              }
return ReturnVal;
}
//****************************



void setup() 
{
// rtc init
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");
 // RTC.set(compileTime());
  //pin setup
  pinMode(buttonStop, INPUT);
  pinMode(buttonStartVanta, INPUT);
  pinMode(ledPinSabat, INPUT);
  pinMode(buttonStartHit, INPUT); 
  pinMode(WoltStartVanta, INPUT);
//--------------------------------
  pinMode(ledPinHit, OUTPUT);
  pinMode(ledPinVanta, OUTPUT);
  pinMode(ledPinSabat, OUTPUT);
  pinMode(ledPinPower, OUTPUT);
//--------------------------------
  pinMode(realyVanta, OUTPUT);
  pinMode(realyHit, OUTPUT); 
//--------------------------------
//all pin set up on init 
  digitalWrite(ledPinPower,LOW);
  digitalWrite(ledPinVanta,LOW);
  digitalWrite(ledPinHit,LOW);
  digitalWrite(ledPinSabat,LOW);
//--------------------------------  
// initialize serial communication:
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor  
   //setTime(17, 20, 0, 8, 4, 19); // set time to Saturday 22:00:00am Jan 1 2018
  Alarm.timerRepeat(1, Repeats2);// timer for every 1 seconds for led blink
}


void loop() 
{
int tempbtn=0;
//get time
   digitalClockDisplay();
   tempbtn = ReadBTN();
   if(tempbtn > 0)
   {
     BTNStatus = tempbtn;
     if (BTNStatus != BTNStatusOLD )
     {
        managSwRelay(); 
        BTNStatusOLD = BTNStatus;  
//        Serial.print("**************   Loop :"); 
//        Serial.println(BTNStatus); 
     }
   }
   
//******** update disply *******
   u8g.firstPage();  
  do {
       draw();
     } while( u8g.nextPage() );
        
//******** end update disply *******
 

  Alarm.delay(500); // wait one second between clock display
}

void managSwRelay(void)
{
  switch (BTNStatus) 
  {      
      case SABAT: 
//            Serial.println("SABAT MODE VENTA"); 
            digitalWrite(ledPinSabat,HIGH);      
            digitalWrite(realyVanta, HIGH);  
            digitalWrite(ledPinVanta,HIGH); 
            sprintf(strdata2, "Mode SABAT Open");                     
            Alarm.timerOnce(SabatOntime, OnceOnly);      
           break;
      case SABAT_CLOSE: 
//            Serial.println("SABAT MODE VENTA close");
            digitalWrite(ledPinVanta,LOW); 
            digitalWrite(realyVanta, LOW);    
            sprintf(strdata2, "Mode SABAT Close"); 
            Alarm.timerOnce(SabatClosetime, OnceOnly); 
            break;
      case WOLT_POWER_VENTA:
//            Serial.println("from mneg WOLT MODE VENTA");   
            digitalWrite(realyVanta, HIGH);  
            digitalWrite(ledPinVanta,HIGH);     
            Alarm.timerOnce(VENTAtime, OnceOnly);   
//            Serial.print(VentaWolt);
//            Serial.println();             
            break;
      case VENTA: 
//            Serial.println("MODE VENTA");   
            digitalWrite(realyVanta, HIGH);  
            digitalWrite(ledPinVanta,HIGH);     
            Alarm.timerOnce(VENTAtime, OnceOnly);   
//            Serial.print("Start Time:");
            //Serial.println();
            //VentaWolt = 1;
           break;
      case HIT:
//            Serial.println("MODE HIT");   
            digitalWrite(realyHit, HIGH);  
            digitalWrite(ledPinHit,HIGH);     
            Alarm.timerOnce(VENTAtime, OnceOnly);   
//            Serial.print("Start Time:");
            //Serial.println();
            //Alarm.free(id);
            break;
      case  STOP_ALL:
            //display time
            TimeCount = 0;
            TimeCountStatus = 0;
            sprintf(strdata3, "Stop");
            //venta
            digitalWrite(realyVanta, LOW);  
            digitalWrite(ledPinVanta,LOW);
            //hit
            digitalWrite(realyHit, LOW);  
            digitalWrite(ledPinHit,LOW);
            //sabat
            SabatSatus = 0;
            digitalWrite(ledPinSabat,LOW);  
            //***
//            Serial.print("Stop:");
//            Serial.println(); 
            sprintf(strdata2, "Stop All");  
            //Alarm.free(id);        
            break;
      default: 
      // if nothing else matches, do the default
      // default is optional
          break;
  }
}



// functions to be called when an alarm triggers:
void MorningAlarm() {
  Serial.println("Alarm: - turn lights off");
}

void EveningAlarm() {
  Serial.println("Alarm: - turn lights on");
}

void WeeklyAlarm() {
  Serial.println("Alarm: - its Monday Morning");
}

void ExplicitAlarm() {
  Serial.println("Alarm: - this triggers only at the given date and time");
}

void Repeats() 
{
  
}
int sec,minit =0;
void Repeats2() 
{

    if ( status_led == 0)
    {
        status_led = 1;
    }else {
            status_led = 0;
          }
    digitalWrite(ledPinPower,status_led);
//    Serial.print("Time = ");
//    Serial.print(TimeCount);
//    Serial.println("");
    if(TimeCountStatus == 1)
    {
       sec++;
       TimeCount++;
       if(sec == 60)
       {
           sec = 0;
           minit++;
       }
       if(TimeCount == 1)
       {
          sec = 0;
          minit=0;        
       }
       sprintf(strdata3, "Start: %d:%d",minit,sec);
    }
}

void OnceOnly() 
{
  
//    Serial.print("BTNStatus :");
//    Serial.println(BTNStatus);
    if (SabatSatus == 1) 
    {
        if(BTNStatus == SABAT_CLOSE)
        {
//           Serial.println("SABAT_CLOSE");          
           BTNStatus = SABAT;
           TimeCount = 0;
        }
        else{
             if(BTNStatus == SABAT)
             {
//                Serial.println("SABAT OPEN");
                BTNStatus = SABAT_CLOSE;
                TimeCount = 0;
             }
            }
        
        
    }else{
            BTNStatus = STOP_ALL; 
//            Serial.println("STOP ALL Time = "); 
//            Serial.println();  
           }
    managSwRelay();           
}

void digitalClockDisplay()
{
    // digital clock display of the time
    time_t t = RTC.get();
    sprintf(strdata4, "Time: %d:%d:%d",hour(),minute(),second());
//    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
//    Serial.print(' ');
//    Serial.print(day());
//    Serial.print(' ');
//    Serial.print(month());
//    Serial.print(' ');
//    Serial.print(year());
//    Serial.println();
}

void printDigits(int digits)
{
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(':');
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

time_t compileTime()
{
    const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char compMon[4], *m;

    strncpy(compMon, compDate, 3);
    compMon[3] = '\0';
    m = strstr(months, compMon);

    tmElements_t tm;
    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);
    sprintf(strdata4, "Time: %d:%d:%d",tm.Hour,tm.Minute,tm.Second);
    time_t t = makeTime(tm);
    setTime(t);
    return t + FUDGE;        //add fudge factor to allow for compile time
}
