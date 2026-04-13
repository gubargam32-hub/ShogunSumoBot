#include <xmotionV3.h>

int leftsens = 0;
int frontleftsens = 1;
int frontrightsens = 2;
int rightsens = 4;

int RightLine = A1;
int LeftLine = A2;

int switch1 = 5;
int switch2 = 6;
int switch3 = 7;
int start = A0;
int led1 = 8;
int led2 = 9;
int flag = A5;
int trimpot = A3;
int LastValue = 2;
void setup() {

  xmotion.StopMotors(10);
  xmotion.ToggleLeds(100);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(flag, OUTPUT);
  
  pinMode(RightLine, INPUT);
   pinMode(LeftLine, INPUT);
    pinMode(leftsens, INPUT);
  pinMode(frontleftsens, INPUT);
  pinMode(frontrightsens, INPUT);
  pinMode(rightsens, INPUT);
  pinMode(start, INPUT_PULLUP);
  pinMode(switch1, INPUT);
  pinMode(switch2, INPUT);
  pinMode(switch3, INPUT);
  digitalWrite(switch1, HIGH);
  digitalWrite(switch2, HIGH);
  digitalWrite(switch3, HIGH);
  digitalWrite(flag, LOW);
  Serial.begin(9600);

}
void leds()
{
  if (digitalRead(leftsens) == 1 || digitalRead(frontleftsens) == 1 || analogRead(LeftLine) < 300  )
  {
    digitalWrite(led1, HIGH);
  }
  else
  {
    digitalWrite(led1, LOW);
  }
  if (digitalRead(rightsens) == 1 || digitalRead(frontrightsens) == 1 || analogRead(RightLine) < 300 )
  {
    digitalWrite(led2, HIGH);
  }
  else
  {
    digitalWrite(led2, LOW);
  }
}
void loop() {
  digitalWrite(flag, LOW);
  xmotion.StopMotors(1);
  while (digitalRead(start) == 1)
  {
    //sensor debug
    /*
      Serial.print(digitalRead(leftsens));
      Serial.print(digitalRead(frontleftsens));
      Serial.print(digitalRead(ortaSensor));
      Serial.print(digitalRead(frontrightsens));
      Serial.println(digitalRead(rightsens));
      delay(100);
    */
     xmotion.StopMotors(1);
    leds();
  }
  bool ls = 0;
  delay(50);
 while (digitalRead(start) == 0);
  
  digitalWrite(flag, HIGH);
  delay(50);
 
  while (digitalRead(start) == 1)
  {
    int speed = map(analogRead(trimpot), 0, 1023, 30, 0);
    leds();
    //////////////////////////////////////////////////////////////////////////////
  if (analogRead(LeftLine) < 300 && analogRead(RightLine) < 300 ) //Both Sensor Saw the Line
    {
      delay(3);
      if (analogRead(LeftLine) < 300 && analogRead(RightLine) < 300 ) //Both Sensor Saw the Line
    {
      xmotion.MotorControl(-100,-100); //Backward %100 speed, 200 ms retreat.
      delay(200);
      xmotion.MotorControl(-100,100); //Left Turning %100 speed, 200ms duration.
      delay(220);
      LastValue=2;

    }
    }else if (analogRead(LeftLine) < 300 ) //Left Line Sensor Saw the Line
    {
      delay(3);
       if (analogRead(LeftLine) < 300 ) //Left Line Sensor Saw the Line
    {
      xmotion.MotorControl(-100,-100); //Backward %100 speed, 200 ms retreat.
      delay(200);
      xmotion.MotorControl(-100,100); //Right Turning %100 speed, 200ms duration.
      delay(100);
      LastValue=2;

    } 
    }
    
    else if (analogRead(RightLine) < 300 ) //Right Line Sensor Saw the Line
    {
      delay(3);
      if (analogRead(RightLine) < 300 ) //Right Line Sensor Saw the Line
    {
      xmotion.MotorControl(-100,-100); //Backward %100 speed, 200 ms retreat.
      delay(200);
  xmotion.MotorControl(100,-100); //Left Turning %100 speed, 200ms duration.
  delay(100);
      LastValue=2;

    } 
    }
    
   else if (digitalRead(frontleftsens) == 1 && digitalRead(frontrightsens) == 1)
    {
      xmotion.Forward(100, 1);
      LastValue=2;
    }
    else if (digitalRead(frontleftsens) == 1)
    {
      xmotion.ArcTurn(70, 100, 1);
      LastValue=1;
    }
    else if (digitalRead(frontrightsens) == 1)
    {
      xmotion.ArcTurn(100,70, 1);
     LastValue=3;
    }
    else if (digitalRead(leftsens) == 1)
    {
      xmotion.ArcTurn(0, 100, 10);
      LastValue=4;
    }
    else if (digitalRead(rightsens) == 1)
    {
      xmotion.ArcTurn(100, 0, 10);
      LastValue=5;
    }
    else if (LastValue==2)
    {
     xmotion.Forward(40 - speed, 1);
    }
    else if (LastValue==1)
    {
     xmotion.ArcTurn(30 - speed, 50 - speed, 1);
    }
    else if (LastValue==3)
    {
     xmotion.ArcTurn(50 - speed, 30 - speed, 1);
    }
     else if (LastValue==4)
    {
     xmotion.ArcTurn(0, 50 - speed, 10);
    }
     else if (LastValue==5)
    {
     xmotion.ArcTurn(50 - speed, 0, 10);
    }


  }
  digitalWrite(flag, LOW);
  delay(50);

}
