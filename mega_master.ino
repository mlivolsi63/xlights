//========================================================================================
// Program : Mega Master
// Author  : Mike LiVolsi
//
// Purpose:  This is the guy who will be attached to the computer (be it a pi or windows)
//           if there are more than 52 channels being passed, then we hand those off to the next
//           mega.
//
// Save on:  /nas/Mike/dev/Arduino
// 
// Original written: ??
// Modifications    : Jan 2022
// Documented on    : March 2023
//
//========================================================================================
#include <string.h>
#include <stdlib.h>

// On the mega, There are about 52 workable channels (54 - 2 for pin0 and pin1)
#define noop
#define DEBUG_PIN 53

bool tested=false;
//-----------------------------------------------------------------------------------------------------------balena
// This is what we are going to send
//-----------------------------------------------------------------------------------------------------------

const byte thisChannels     = 48;             // See mappings
const byte maxArraySize     = 48;             // Leave room for string + "<" + ">" + EOL + 1 (see below when declaring the array, we add 4 bytes)
                                              // ... it should also be the number of channels on Vixen
//========================================================================================
// THis is THE script.
// Read in from vixen the string for each time event.. ie. let'say channel 1 is 0, channel 2 is 1.. Vixen will send "< 1 0 >"
// Then, any channel less than 52 is processed here and sent to it's respective controller
// Channels 52-103 can be sent to Arduino 2
// .. and, if needed, 104 to 155 could be sent to Arduino 3 (and so on)
//
// For each cell in Vixen, it sends 1 byte (0-255 or hex 0-FF) in the stream
// 0 means nothing sent (or off), and 255 means on...
// However, values between 0 and 255 are used for dimming in LED land..
//
// NOTE: See function that processes the array for details on pins and PWM
//
//========================================================================================

//---------------------------------------------------------------------
// Strings state from Vixen
//---------------------------------------------------------------------
const byte _wait    = 0;            // we are in the state of waiting for a '<'
const byte _read    = 1;            // we are in the state of reading until we  encounter a footer
const byte _work    = 2;            // we're in the middle of processing and spitting out to the relays


byte     arr1[thisChannels+4];        // number of channels we are working with + 3 for head/footer and EOL
byte     readStatus = _wait;          // 0 not reading | 1 - encountered '<' so set to reading


//------------------------------------------
// setup the pins/ inputs & outputs
//------------------------------------------
void setup() {
  // Serial.println("Begin 57600");
  Serial.begin(57600);                      // set up Serial at 57600 bps
    
  unsigned int i = 0;
  for (i = 0; i < thisChannels; i++) {        // Though we could start with pin 2, let's really clarify things here
    switch(i) {
      case 0:
      case 1:   
      case 18: 
      case 19:  noop;                            // 0/1 are Serial. 18/19 are Serial1
                break;
      case 2: 
      case 46:  noop;
                break;                          // Broken pins
      default:  pinMode(i,  OUTPUT);        
              
    }
    pinMode(48,  OUTPUT);        
    pinMode(49,  OUTPUT);            
    pinMode(50,  OUTPUT);        

  }  

  for (i = 40; i <= 43; i++) {        // Though we could start with pin 2, let's really clarify things here
      digitalWrite(i, HIGH); 
  }
  for (i = 47; i <= 50; i++) {        // Though we could start with pin 2, let's really clarify things here   
      digitalWrite(i, HIGH);                    
  }
}

void blinkIt(byte inPin, byte inCount) {
    for(int i=0; i < inCount; i++) {  
        digitalWrite(inPin, HIGH);
        delay(250);
        digitalWrite(inPin, LOW);
        delay(250);
    }
}

//--------------------------------------------------------------------------------------------
// Routine: writeOutBuffer
//
// Purpose: The first 52 positions (minus whatever pins are dead or we are avoiding)
//          go out to their respective box (mosfets are 0-15, with the last three physically
//          mapped to the last pins on the board.
//          Everything in the range of 52-103   are written to Arduino #2 (if available )
//          "stuff" after 54 (array 53) are sent to target (and relayed to Nano)
//--------------------------------------------------------------------------------------------

void writeOutBuffer() {

    readStatus = _wait;
    //if(!tested) {
    //  Serial.println("writing out this buffer");
    //  blinkIt(14, 5);
    //  tested=true;
    // }
    //-----------------------------------------------------------------------------------------------------------------------
    // send out to the pins
    // we are avoiding pins 0 and 1 on the Mega
    // Pin 2 and 46 are  foobared
    //
    //
    // For  reference Mega PWM Pins: 2-13
    //                               44-46
    //
    //     array       Vixen Channel     | Comments             MEGA       PWM             port     
    //     0           1             (1)   DEBUG                53               (1)        -     ONLY ENABLE    
    //     1-8         2-9           (8)   LED                  3-10        X    (8)        1
    
    //     9-11        10-12         (3)   LED                  11-13       X    (3)        2
    //     12-13       13-14         (2)   LED                  44-45       X    (2)        2
    //
    //     NOTE: PIN 2 AND 46 IS DEAD
    //-----------------------------------------------------------------------------------------------------------------------
    //   we are going to use pins 18/19 for tx/rx so mapping accordingly
    //-----------------------------------------------------------------------------------------------------------------------
    //     array       Vixen Channel     | Comments             MEGA       PWM             port     
    //-----------------------------------------------------------------------------------------------------------------------
    //     14-17       15-18         (4)                        14-17            (4)        3      skip pins 18/19
    //     18-21       19-22         (4)                        20-23            (4)        3     
    //
    //     22-29       23-30         (8)                        24-31            (8)        4
    //     30-37       31-38         (8)                        32-39            (8)        5
    //
    //     38-41       39-42         (4)   MECHANICAL           40-43            (4)        6      44,45 used above, skip 46
    //     42-45       43-46         (4)   MECHANICAL           47-50            (4)        6      
    //
    //     46-47       47-48         (2)   TBD                  51-52            (2)        -
    //
    //  TEST NOTES - Change comparison from '0' to '48' below.
    //  The following strings for turning on mechanical relays : <000000000000000000000000000000000000001111111100>
    //-----------------------------------------------------------------------------------------------------------------------
    // Serial.print(millis()); Serial.println(" Processing THIS array ");

    for (int i = 0; i < thisChannels; i++) {
        //  Serial.print(i);
        // ----------------- ANALOG (PWM) PINS ----------------------
        if (i <= 13) {
            // Serial.print("A "); Serial.println(millis());
            if (i <= 11) {      
                if (i==0 ) {
                    if ( arr1[i] == 255) {
                        //Serial.print(" - DEBUG Pin: "); Serial.print(DEBUG_PIN); 
                        // blinkIt(DEBUG_PIN, 5);
                    }
                    // else Serial.println(arr1[i]);    
                }
                else {
                     analogWrite(i+2, arr1[i]);
                     // Serial.print(" - Pin: "); Serial.println(i+2); 
                }    
            }    
            else {    
                analogWrite(i+32, arr1[i]);
                // Serial.print(" - Pin: "); Serial.print(i+32); 
            }
        }
        //------------------- DIGITAL PINS ---------------------------- It's greater than 13
        else {
           // Serial.print("D "); Serial.println(millis());
            if ( i <= 17 ) {                                            // between 13 and including 17
                //if((i==14) && (arr1[i] > 0 ) && !tested)  {
                //   blinkIt(14, 1);
                //   tested=true;
                //}   
          
                if (arr1[i] > 0 ) digitalWrite(i, HIGH);
                else              digitalWrite(i, LOW);                
             
            }
            //--------------- DIGITAL PINS ---- Greater than 17 ------- 
            else {              
                if ( i <= 37) {                                             // between 18 and 37
                    //Serial.print(" - Pin: "); Serial.print(i+2); 
                    if (arr1[i] > 0 ) digitalWrite(i+2, HIGH);
                    else              digitalWrite(i+2, LOW);
                }
                else {
                    if ( i <= 45) {                                         // between 38 and 45
                        if( i <= 41 ) {                                     // between 38 and 41 -> mechanical pins. So current is reversed
                            // Serial.print(" - Mechanical Pin(1): "); Serial.print(i+2);  Serial.print(" value :"); Serial.println(arr1[i]);
                            if (arr1[i] > 0 ) digitalWrite(i+2, LOW); 
                            else              digitalWrite(i+2, HIGH); 
                        }
                        else {                                              // between 
                            // Serial.print(" - Mechanical Pin(2): "); Serial.print(i+5);  Serial.print(" value :"); Serial.println(arr1[i]);
                            if (arr1[i] > 0 ) digitalWrite(i+5, LOW); 
                            else              digitalWrite(i+5, HIGH); 
                        }
                    }
                    else {
                        if( i <= 47 ) {
                          //Serial.print(" - TBD ");
                          noop;
                        }
                    }
                }
            }
        }
    }    
}        
      
//---------------------------------------------------------------------------------------------------
// Routine: Loop
//
// Purpose: Read in the string of bytes coming in from Vixen. Then once we get our EOL indicator '>'
//          take the buffer and write that out to whatever target is appropriate for that range
//          in this case, first 15 go to LED string, the next up to 51 go to SSD relays
//          52-102   go to Arduino 2 (51 pins are usable)
//          104-155  go to Arduino 3
//
//---------------------------------------------------------------------------------------------------
unsigned int  byteCount = 0;
unsigned int  inByte;                                                          // can't make this a byte, since there might be other characters

void loop() {


  unsigned int i = 0; // loop counter
  unsigned int k = 0; // debugger
 

  while ( Serial.available() > 0 ) {                                        // bytes in the buffer
      inByte = Serial.read();                                               // read in the value

      switch (readStatus) {
 
          case _wait:                                                     // looking for start of header
                        if ( inByte == '<' ) { 
                            readStatus = _read;          // set flag to start reading data
                            // Serial.println("Received start bracket");
                        }    
                        break;

          case _read:
                        if (inByte == '>' ) {                             // footer - which means we are done
                            // Serial.println("Received end bracket");
                            if ( byteCount < maxArraySize ) {             // we missed some bytes
                                // Serial.println("Byte count too low");
                                readStatus = _wait;                       // start re-reading because the argument is foobared
                                byteCount=0;
                            }
                            else {
                                // Serial.println("Everything checks out. Sending out to switches");
                                readStatus = _work;                       // set the flag that we're ready to use our buffer. So both arduinos can do work !!
                            }
                        }
                        else {                                            // we are still reading in
                            if (byteCount < thisChannels) {               // but we're still in the part of the array that's meant for THIS arduino
                                arr1[byteCount] = inByte;                 // set the byte in the array
                            }
                            else {                                        // ok, we're done with array that deals with THIS  arduino
                                // Serial.println("Something is wrong");
                                readStatus = _work;                                
                            }    
                            byteCount++;                                   // increment our byte count
                        }    
                        break;
      }                
  } 

  if (readStatus == _work) {                                        // our buffer is full and ready to process
      
      writeOutBuffer();
      for (i = 0; i < sizeof(arr1); i++) {    // wipe out the array
          arr1[i] = 0;
      }
      byteCount=0;          
  } 
  
  // end serial.available();
}                                                                    // end loop


