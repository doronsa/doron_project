#include<Servo.h>
Servo Myservo;
int pos;
int angle = 0; 


void setup()
{
Myservo.attach(3);
Serial.begin(9600);
}

void loop()
{

  
if (Serial.read() == '\n')
{
    Serial.print("angle : ");
    angle = Serial.read(); 
    Myservo.write(angle);
    Serial.println(angle);
    delay(800);  
}
  
//for(pos=70;pos<=100;pos++)
//{
//  Myservo.write(pos);
//  delay(15);
//}
//  delay(1000);
  
//  for(pos=100;pos>=70;pos--)
//  {
//    Myservo.write(pos);
//    delay(15);
//  }

 

 
  //Myservo.write(110);
  
}
