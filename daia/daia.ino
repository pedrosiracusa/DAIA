/*
  DAIA - Automated Imagery Device Software v1.0
  This project runs on an Arduino Nano board
  Copyright Pedro de Siracusa, 2015

  Version Features:
    - Simple Intervalometer Mode
    - Lighting Photography Mode (beta)
    - Wildlife IR Sensing Mode (beta)
    - Bulb Intervalometer (beta)
*/

/* FUNCTION DECLARATIONS */
void triggerShutter(int bulb, float dly);
void blinkLed();
void tlMode(int slow, int fast);
void bulbMode(int bulbSlow, int bulbFast, int interval);
void lightingMode();
void pirMode(int interval);


/* PINS ASSIGNMENTS */
int cameraCtrlPin=11; /* this pin is the base of the transistor */
int ledPin=12; /* this pin controls the monitor led */
int potPin=A1; /* interval controller pin, reads analogic signal from a potentiometer */
int photoPin=A5; /* light sensing pin, reads analogic signal from the photoresisor */
int pirPin=4; /* PIR sensor logical input */

/* MODE SWITCH */
int modeToEnter; /* stores mode to enter through potentiometer reading, during setup */


/*********************************/
/* MODE 1: Simple intervalometer */

int interval; /* Stores user interval input, read from potentiometer */

void tlMode(int fast, int slow){ 
  /* 
    simple intervalometer main function 
    @param fast: the fastest interval in seconds, min potentiometer
    @param slow: the slowest interval in seconds, max potentiometer
  */
  interval = map(analogRead(potPin), 0, 1023, fast, slow); /* remaps interval input from potentiometer (last two parameters are the lowest and highest intervals in miliseconds, respectively */
  triggerShutter(0,interval); 
  return;
}


/*************************/
/* MODE 2: Lighting mode */

int activate; /* Stores binary: 1 if lighting trigger was activated, 0 otherwise */
int lightThreshold; /* Stores photoresistor sensitivity threshold input from user, through potentiometer */

void lightingMode(){ /* lighting mode main function */
  /* In this mode an image is recorded as soon as a peak in lighting conditions is detected */
  lightThreshold=map(analogRead(potPin), 0, 1023, 1023, 0); /* gets threshold value from user, through potentiometer */
  activate=constrain(map(analogRead(photoPin), lightThreshold, 1024, 0, 255), 0, 1); /* stores binary value: 1 - level exceeded threshold; 0 - otherwise */

  if(activate){
    triggerShutter(0,0);
  }
  return;
}

/*********************/
/* MODE 3: PIR mode  */
void pirMode(int interval){
  /* 
    In this mode the sensor scans for any change in the scene's IR profile 
    @param interval: interval before the camera may be activated again
  */
  
  if(digitalRead(pirPin)==HIGH){
    triggerShutter(0,interval);
  }
}

/*********************/
/* MODE 4: BULB mode */
int bulbDur;
void bulbMode(int bulbFast, int bulbSlow, int interval){
  bulbDur = map(analogRead(potPin), 0, 1023, bulbFast, bulbSlow); /* remaps bulb duration input from potentiometer (last two parameters are the lowest and highest durations in miliseconds, respectively) */
  triggerShutter(bulbDur,interval);
}




/*********************/
/* GENERAL FUNCTIONS */

void triggerShutter(int bulb, float dly){
  /*
    Function to trigger shutter
    @param bulb: If > 0, enters bulb mode, with input interval in seconds. If bulb==0 uses camera settings(user still needs to set camera exposure!)
    @param dly: Delay AFTER photo is taken, in seconds
  */
  digitalWrite(cameraCtrlPin, LOW);
  digitalWrite(ledPin, HIGH);

  if(bulb==0)
    delay(100);
  else{
    float bulbDelay = bulb*1000.0;
    Serial.println(float(bulbDelay));
    delay(bulbDelay);
  }
  
  digitalWrite(cameraCtrlPin, HIGH);
  digitalWrite(ledPin, LOW);

  delay(dly*1000);
  
  return;
}

void blinkLed(){
  /* A simple led blink */  
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
}



/***********************/
/* SETUP AND MAIN LOOP */

void setup(){
  Serial.begin(9600);
  pinMode(cameraCtrlPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(photoPin, INPUT);
  pinMode(pirPin, INPUT);

 
  /* choses mode and sets pins based on potentiometer input */
  if(analogRead(potPin)<=10)
    modeToEnter=1;
  
  else if(analogRead(potPin)<=400)
    modeToEnter=2;
  
  else if(analogRead(potPin)<=800)
    modeToEnter=3;
    
  else
    modeToEnter=4;  
  
  
  digitalWrite(cameraCtrlPin, HIGH); /* prevents camera from start shooting */
}

int firstLoop=1;
void loop(){
  
  switch(modeToEnter){
    case 1: /* enter simple time lapse mode */
      if(firstLoop){
        blinkLed();
        firstLoop=0;
        delay(500);          
      }     
      tlMode(1,10);
      break;
      
    case 2: /* enter bulb mode */
      if(firstLoop){
        for(int i=0; i<2; i++)
          blinkLed();
        firstLoop=0;
        delay(500);          
      }
      bulbMode(30, 180, 1);
      break;

    case 3: /* enter lighting mode */
      if(firstLoop){
        for(int i=0; i<3; i++)
          blinkLed();
        firstLoop=0;
        delay(500);          
      }
      lightingMode();
      break;

    case 4: /* enter pir mode */
      if(firstLoop){
        for(int i=0; i<4; i++)
          blinkLed();
        firstLoop=0;
        delay(500);          
      }   
      pirMode(2);
      break;
      
  }
}
