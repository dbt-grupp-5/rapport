// Setup the motor controller
#include "DualVNH5019MotorShield.h"
DualVNH5019MotorShield md;


// And prepare for reading the ppm signal from the RC-controller
#define PPM_Pin 2 //this must be 2 or 3
#define multiplier (F_CPU/8000000)  //leave this alone
int ppm[16]; // Store the signal values

// Debug mode for RC-signal
//#define DEBUG

// Define speed limits
#define STOP_LIMIT 1240
#define FULL_SPEED_LIMIT 1900


// Function to do nothing if anyting fails with the motorcntroller
void stopIfFault()
{
  if (md.getM1Fault())
  {
    Serial.println("M1 fault");
    while(1);
  }
  if (md.getM2Fault())
  {
    Serial.println("M2 fault");
    while(1);
  }
}

void setup()
{
  // Initailize serial consle
  Serial.begin(115200);
  Serial.println("DBT Spinchem(R) RBR controller");

  // Initailize motor driver
  md.init();

  // Setuo signal pin from RC and attatch the initial interupt
  pinMode(PPM_Pin, INPUT);
  attachInterrupt(PPM_Pin - 2, read_ppm, CHANGE);

  TCCR3A = 0;  //reset timer3
  TCCR3B = 0;
  TCCR3B |= (1 << CS11);  //set timer3 to increment every 0,5 us
  
}

// The main loop of the program
void loop()
{
  //You can delete everithing inside loop() and put your own code here
  int count=0;
  while(ppm[count] != 0){  //print out the servo values
    #if defined(DEBUG)
    Serial.println(ppm[count]);
    #endif
    count++;
  }
  if(count > 0){
    count--;
    int rcValue = ppm[count];
    int mspeed = 0;
    if(rcValue <= STOP_LIMIT){
      // Should we stop the motors?
      mspeed = 0;
      #if defined(DEBUG)
      Serial.println("Stop");
      #endif
    }
    else if(rcValue >=FULL_SPEED_LIMIT ){
      // Skould we run at full speed?
      mspeed = 400;
      #if defined(DEBUG)
      Serial.println("Full speed");
      #endif
    }
    else{
      // Something inbetween!
      double fspeed = (1.0*rcValue - STOP_LIMIT)/(FULL_SPEED_LIMIT - STOP_LIMIT);
      mspeed = ((int) 400*fspeed);
      #if defined(DEBUG)
      Serial.println(fspeed);
      #endif
    }
    #if defined(DEBUG)
    Serial.print("Setting speed to: ");
    Serial.println(mspeed);
    #endif
    md.setM1Speed(mspeed);
    md.setM2Speed(-mspeed);
  }
  delay(10);
}

// Reads the ppm signal
void read_ppm(){  //leave this alone
  static unsigned int pulse;
  static unsigned long counter;
  static byte channel;
  static unsigned long last_micros;
  counter = TCNT3;
  TCNT3 = 0;

  if(counter < 710*multiplier){  //must be a pulse if less than 710us
    pulse = counter;
    #if defined(servoOut)
    if(sizeof(servo) > channel) digitalWrite(servo[channel], HIGH);
    if(sizeof(servo) >= channel && channel != 0) digitalWrite(servo[channel-1], LOW);
    #endif
  }
  else if(counter > 1910*multiplier){  //sync pulses over 1910us
    channel = 0;
    #if defined(DEBUG)
    Serial.print("PPM Frame Len: ");
    Serial.println(micros() - last_micros);
    last_micros = micros();
    #endif
  }
  else{  //servo values between 710us and 2420us will end up here
    ppm[channel] = (counter + pulse)/multiplier;
    #if defined(DEBUG)
    Serial.print(ppm[channel]);
    Serial.print("  ");
    #endif
    
    channel++;
  }
}

