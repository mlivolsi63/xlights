int inByte=0;             // The byte coming in
int counter=0;            // Count the bytes
bool loopFlag=true;       // boolean flag
bool readFlag=false;      // Flag to let us know when to read



// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);                      // set up Serial at 9600 bps, must be same on xlights
}

void blinkIt(int looper) 
{
  for (int i=0; i< looper; i++)  {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(250);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(250);                       // wait for a second
  }

}

// the loop function runs over and over again forever
// You can't set the above vars in this function, since loop always gets called and resets the values
// So, we're using global variables
void loop() {

  while ( (Serial.available() > 0) && loopFlag ) {                          // bytes in the buffer and only do this once
      inByte = Serial.read();                                               // read in the value
      if (inByte == '<')  {
        readFlag=true;                                                      // start reading
      }
      else {
        if (inByte == '>') loopFlag = false;                                // stop whatever we're doing 
        else {
          if (readFlag) counter++;                                          // else only start counting if we are first bounded by a '<'
        }
      }
      
      if (inByte == '>' && counter >= 50) {
          blinkIt(counter);
     }
  }
}
