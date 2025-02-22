
///////////////
// Libraries //
///////////////

#include <WiFi.h>
#include <iostream>
#include <stdio.h>
#include "time.h"
#include <cmath>
#include <LiquidCrystal.h>



////////////
// To Do: //
////////////
/*
 * modularise code
 * 
 * have go to connecting screen when loses connetion with wifi for more than a set period of time (30 seconds?)
 * 
 * implement all of the physical functions for control loops
 * 
 * implement all of the physical functions for the calibration loop
 */


/////////////////
// Definitions //
/////////////////

#define COMMAND_LENGTH 255


////////////////////////////////////////////////////
// Logic to control the coordinates of the digits //
////////////////////////////////////////////////////

/*
           aaa     
          f   b
          f   b
          f   b
           ggg
          e   c
          e   c
          e   c
           ddd
*/

//coordinates assume bottom left is 0, once homed
//segments starting y, top to bottom, in mm
const int aY = 350;
const int bY = 335;
const int cY = 200;
const int dY = 50;
const int eY = 200;
const int fY = 335;
const int gY = 200;

//segements starting x. left to right, in mm
const int aX = 20;
const int bX = 120;
const int cX = 120;
const int dX = 20;
const int eX = 0;
const int fX = 0;
const int gX = 20;

//segment lengths, short are a, d, g and long are b, c, e, f, in mm
const int shortLength = 100;
const int longLength = 120; 

//offsets for the different digits, 1 2 : 3 4, in mm
const int digitOneOffset = 50;
const int digitTwoOffset = 200;
const int digitThreeOffset = 400;
const int digitFourOffset = 550;
const int colonOffset = 350;

//offset for eraser to cover same ground as marker, in mm
const int eraserX = 10;
const int eraserY = 57;


/////////////////////////
// Function Prototypes //
/////////////////////////

void drawTime(int hrs, int mins, bool erase);
void drawDigit(int number, int digit, int erase);

void dividingColon(int digit, bool erase);

void digitZero(int digit, bool erase);
void digitOne(int digit, bool erase);
void digitTwo(int digit, bool erase);
void digitThree(int digit, bool erase);
void digitFour(int digit, bool erase);
void digitFive(int digit, bool erase);
void digitSix(int digit, bool erase);
void digitSeven(int digit, bool erase);
void digitEight(int digit, bool erase);
void digitNine(int digit, bool erase);

void segmentA(int digit, bool erase);
void segmentB(int digit, bool erase);
void segmentC(int digit, bool erase);
void segmentD(int digit, bool erase);
void segmentE(int digit, bool erase);
void segmentF(int digit, bool erase);
void segmentG(int digit, bool erase);

void markerHome();
void markerDown();
void eraseDown();
void capMarker();
void reHome();
void markerHome();
void goToXY(double x, double y);
void reHome(bool x, bool y);
void test();
void controlledSerial();
void clockLoop();
void eraseAll();
void drawNow();
void rest();
void wifiLoop();
void offLoop();
void onLoop();
void pauseLoop();
void offController();
void timeUpdateController();
void modeUpdateController();
void updateTimeInterval();
void calibrationLoop();

//////////////////////////////
// Constants and Structures //
//////////////////////////////

// Network Configuration
const char* ssid     = ""; //replace with sierra college ssid;
const char* password = ""; //psswd is student login for canvas
const char* ntpServer = "pool.ntp.org";

// Time Configuration
const long  gmtOffset_sec = -28800;
const int   daylightOffset_sec = 3600; // daylight savings time, 1 hour
unsigned long currTime = millis();
unsigned long prevTime = 0;
struct tm currentTime;
struct tm lastUpdate;
tm getTime(); // checks getting the time

// Various Things
String header;
const long timeoutTime = 2000;
bool isHomed = false;
bool start = false;

// Physical Controller
const int offButton = 27;// swapped with pause, was 27
const int upButton = 14;
const int downButton = 12;
const int onButton = 25;
const int pauseButton = 26; // swapped with off, was 26

int offMode = 0;
int upMode = 0;
int downMode = 0;
int onMode = 0;
int pauseMode = 0;

bool offState = false;
bool onState = false;
bool pauseState = false;

int updateInterval = 5; // update the clock every five minutes, default
String Mode = "Off";

// Starts the lcd using the liquid crystal library
LiquidCrystal lcd(19, 4, 18, 17, 16, 5);


// Starts the Wifi Server
WiFiServer server(80); // uneeded, unless using wifi server controller

/*
 * The setup does the following:
 *  1.Starts the serial connection between the esp and serial moniter if needed, Serial
 *  2.Starts the serial connection between the esp and the btt skr, Serial1
 *  3.Connects to the network
 *  4.Grabs the time for the currenTime struct and prints to the moniter
 */

int i = 0;
 
void setup() {

  pinMode(offButton, INPUT_PULLUP); 
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(onButton, INPUT_PULLUP);
  pinMode(pauseButton, INPUT_PULLUP);

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX, TX);  // indicates Rx on pin 36, Tx on pin 32, rx white, tx yellow
  //Serial1.begin(115200, SERIAL_8N1, 36, 32);  // indicates Rx on pin 36, Tx on pin 32

  //LCD Setup, (columns and rows), and prints the current time
  lcd.begin(16, 2);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(2500);
    Serial.println(".");
    
    lcd.setCursor(0,0);
    lcd.print("Connecting to:");
    lcd.setCursor(0,1);
    lcd.print(ssid);
    lcd.setCursor(14,1);
    lcd.print(counter);
    counter += 1;
  }
  lcd.clear();
  Serial.println("");
  Serial.println("WiFi connected.");

  Serial.println("IP Adress: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  currentTime = getTime();
  counter = 0;
  while (!currentTime.tm_hour){
    delay(2500);
    currentTime = getTime(); // currently getTime() prints the time, will want to display on a display

    lcd.setCursor(0,0);
    lcd.print("Getting Time: ");
    lcd.setCursor(14,0);
    lcd.print(counter);
    lcd.setCursor(0,1);
    lcd.print(currentTime.tm_hour);
    lcd.print(":");
    lcd.print(currentTime.tm_min);
    lcd.setCursor(7,1);
    lcd.print("Connected");
    counter += 1;
  }
  lcd.clear();
  
}

void loop(){

  //lcd.setCursor(0,1);
  //lcd.print(i);
  //i++;
  
  //test();
  offState = true;
  displayController();
  mainLoop();
  lcd.clear();
  delay(1000);
  
  
}



////////////////////////
// Control Mechanisms //
////////////////////////

// Control the Clock using Serial
//    - 'y' gets current time and updates time
void controlledSerial(){
  //updateTime(currentTime, lastUpdate);
  //currentTime = getTime();

  if(Serial.available()){
    int ch = Serial.read();
    if(ch == 'y'){
      currentTime = getTime();
      updateTime(currentTime, lastUpdate);
    }
  }
}

// Control the Clock using Button Interface

/*
 * 
 *    16x2 display
 *                  *1 - button to rasie time interval
 *      Time: 00:00 upIn   16/16 chars 
 *      Mode: _____ 00mn   16/16 chars
 *         
 *                  *2 - button to lower time interval
 *        *3   *4   *5
 *        (on, pause, off) go in the ______ after mode:
 *      
 *      on will turn the clock on, will also unpause
 *        this means the clock will erase the current time, if any
 *        and will draw the current time
 *        
 *        1st press
 *        on = true
 *        pause = false
 *        
 *        2nd press - no change
 *        on = true
 *        pause = false
 *        
 *      pause will pause the current loop, it will cease all movement
 *        and return home, pressing again will be equivalent to pressing
 *        the on button
 *        
 *        1st press
 *        on = true
 *        pause = true
 *        
 *        2nd press - will unpause
 *        on = true
 *        pause = false
 *         
 *      off will tell the clock to erase the current time, and will send
 *        the carriage home, pressing again will do nothing
 *        
 *        1st press
 *        on = false
 *        pause = faslse
 *        
 *        2nd press - no change
 *        on = fasle
 *        pause = false
 *        
 *        needs defined buttons 1-5
 *        boolean on, pause, default to false for both
 *        
 *        aftersetup, the on btton needs to be pressed, then displays here
 *        
 *        during setup, will discuss wifi connection stuff, only press on after that is verified
 *        
 *        
 *        functions- on, pause, off, intervalChange
 *        
 *       
 */


////////////////////////////////////////////////
// Various Helpful Functions to Test Hardware //
////////////////////////////////////////////////

void test(){
  reHome();

  //drawDigit(0, digitOneOffset, false);
  //delay(500);
  drawDigit(5, digitOneOffset, false);
  delay(500);
  drawDigit(2, digitTwoOffset, false);
  delay(500);
  dividingColon(colonOffset, false);
  delay(500);
  drawDigit(6, digitThreeOffset, false);
  delay(500);
  drawDigit(7, digitFourOffset, false);
  delay(500);
  

  
  //drawDigit(0, digitTwoOffset, true);
  //delay(500);
  //drawDigit(0, colonOffset, true);
  //delay(500);
  
  reHome();
 
  capMarker();
  delay(2000);
}

//is this correct? After eraseing the colon, using digitTwoOffset, and then colonOffset?
void eraseAll(){
  reHome();
  drawDigit(8, digitOneOffset, true);
  delay(500);
  drawDigit(8, digitTwoOffset, true);
  delay(500);
  dividingColon(colonOffset, true);
  delay(500);
  drawDigit(8, digitTwoOffset, true);
  delay(500);
  drawDigit(8, colonOffset, true);
  delay(500);
  reHome();
  capMarker();
}


///////////////////////
// General Functions //
///////////////////////


// Check if needs to draw out the current time, 
//  and does so if more than set update interval 
//  has passed when called, erase if needed
void updateTime(tm currentTime, tm lastUpdate){
  reHome();
  

  if(!lastUpdate.tm_hour){
    drawTime(currentTime.tm_hour, currentTime.tm_min, false);
    lastUpdate = currentTime;
  }
  else{
      int checkTime = lastUpdate.tm_min;
      if(currentTime.tm_min == 0){
        checkTime = currentTime.tm_min;
      }
      if((currentTime.tm_min - checkTime) > updateInterval){ 
      drawTime(lastUpdate.tm_hour, lastUpdate.tm_min, true);
      drawTime(currentTime.tm_hour, currentTime.tm_min, false);
      lastUpdate = currentTime;
      // perhaps more time, until the update is finished
      delay(2000);
    }
  }

  reHome();
  capMarker();
}

// Erases all the digits and draws current time
void drawNow(){
  reHome();
  drawTime(lastUpdate.tm_hour, lastUpdate.tm_min, true);
  drawTime(currentTime.tm_hour, currentTime.tm_min, false);
  lastUpdate = currentTime;
  reHome();
  capMarker();
}

// gets the time and assigns to struct currentTime
tm getTime(){
  struct tm date;
  if(!getLocalTime(&date)){
    Serial.println("Failed to get time");
    return date = {};
  }  
  Serial.printf("time: %d:%d\n", date.tm_hour, date.tm_min);
  //lcd.printf("Time: %d:%d", date.tm_hour, date.tm_min);
  return date;
}

// Main control to draw out time
void drawTime(int hrs, int mins, bool erase){
  hrs=hrs%12;   //convert 24 hour time to 12 hour time
  drawDigit((hrs/10)%10, digitOneOffset, erase);    // 10s place of hours // unsure if this is correct ****************
  drawDigit(hrs%10, digitTwoOffset, erase);        // 1s place of hours
  dividingColon(colonOffset, erase);
  drawDigit((mins/10)%10, digitThreeOffset, erase); // 10s place of mins
  drawDigit(mins%10, digitFourOffset, erase);      //1s place of mins
}

void mainLoop(){
  while(true){
    if(offState == true){
      offLoop();
    }
    else if(onState == true){
      onLoop();
    }
    else if(pauseState == true){
      pauseLoop();
    }
    
  }
}

void offLoop() {

  
  
  modeUpdateController();
  while(onState == false){
    delay(1000);
    timeUpdateController();
    onMode = digitalRead(onButton);
    offMode = digitalRead(offButton);
    upMode = digitalRead(upButton);
    downMode = digitalRead(downButton);
    if(onMode == LOW && offMode == LOW){
      calibrationLoop();
    }  
    else if(onMode == LOW){
      Mode = "On   ";
      onState = true;
      break;
    }
    else if(upMode == LOW){
      upController();
    }
    else if(downMode == LOW){
      downController();
    }
  }
  offState = false;
  return;
}

void onLoop(){
  
  modeUpdateController();
  while(onState == true){
    delay(1000);
    timeUpdateController();
    offMode = digitalRead(offButton);
    pauseMode = digitalRead(pauseButton);
    upMode = digitalRead(upButton);
    downMode = digitalRead(downButton);
    if(offMode == LOW){
      Mode = "Off  ";
      onState = false;
      offState = true;
      break;
    }
    else if(pauseMode == LOW){
      Mode = "Pause";
      onState = false;
      pauseState = true;
      break;
    }
    else if(upMode == LOW){
      upController();
    }
    else if(downMode == LOW){
      downController();
    }
    updateTime(currentTime, lastUpdate);
  }
  return;
}

void pauseLoop(){

  markerHome();
  reHome();
  capMarker();

  modeUpdateController();
  while(pauseState == true){
    delay(1000);
    timeUpdateController();
    onMode = digitalRead(onButton);
    pauseMode = digitalRead(pauseButton);
    offMode = digitalRead(offButton);
    upMode = digitalRead(upButton);
    downMode = digitalRead(downButton);
    if(pauseMode == LOW){
      Mode = "On   ";
      onState = true;
      pauseState = false;
      break;
    }
    else if(offMode == LOW){
      Mode = "Off  ";
      offState = true;
      pauseState = false;
      break;
    }
    else if(onMode == LOW){
      Mode = "On   ";
      onState = true;
      pauseState = false;
      break;
    }
    else if(upMode == LOW){
      upController();
    }
    else if(downMode == LOW){
      downController();
    } 
  }
  return;
}

// calibrates the hight of the marker and eraser
// first raises the marker and has the user use the buttons to lower it into writing disatance
// then raises the eraser and has the user lower it until it erases
// then raises the marker and sets it over to be capped and as the user lower it to be capped
// then goes back to home
void calibrationLoop(){
  onMode = digitalRead(onButton);
  pauseMode = digitalRead(pauseButton);
  offMode = digitalRead(offButton);
  upMode = digitalRead(upButton);
  downMode = digitalRead(downButton);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibration Mode");
  lcd.setCursor(0,1);
  lcd.print("Remove Fingers");
  
  while(onMode == LOW || offMode == LOW){
    
    onMode = digitalRead(onButton);
    offMode = digitalRead(offButton);
  }
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrate Marker +/-: ");
  //lcd.print(writingHeight);
  lcd.setCursor(0,1);
  lcd.print("press |> when done");

  // home?
  // raise marker
  // move marker to spot
  // lower reasonable amount
  
  while(onMode == HIGH){
    delay(1000); // 1 second
    //markerHeight -= 1;
    // move marker down
    //lcd.setCursor(0,22);
    //lcd.print("  ");
    //lcd.setCursor(0,22);
    //lcd.print(markerHeight);
    onMode = digitalRead(onButton);
    pauseMode = digitalRead(pauseButton);
  }
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrate Eraser +/-: ");
  //lcd.print(eraserHeight);
  lcd.setCursor(0,1);
  lcd.print("press || when done");

  // raise marker
  // move marker to spot
  // lower reasonable amoun
  
  
  while(pauseMode == HIGH){
    delay(1000); // 1 second
    //eraserHeight -= 1;
    // move eraser down
    //lcd.setCursor(0,22);
    //lcd.print("  ");
    //lcd.setCursor(0,22);
    //lcd.print(eraserHeight);
    onMode = digitalRead(onButton);
    pauseMode = digitalRead(pauseButton);
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrate Marker +/-: ");
  //lcd.print(homeHeight);
  lcd.setCursor(0,1);
  lcd.print("press |> when done");

  // home?
  // raise marker
  // move marker to home
  // lower reasonable amount
  
  while(onMode == HIGH){
    delay(1000); // 1 second
    //homeHeight -= 1;
    // move eraser down
    //lcd.setCursor(0,22);
    //lcd.print("  ");
    //lcd.setCursor(0,22);
    //lcd.print(eraserHeight);
    onMode = digitalRead(onButton);
    pauseMode = digitalRead(pauseButton);
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("After rehome");
  lcd.setCursor(0,1);
  lcd.print("[] to end");
  
  // raise marker
  // home cradle
  // reseat cradle
  
  while(offMode == HIGH){
    onMode = digitalRead(onButton);
    pauseMode = digitalRead(pauseButton);
    offMode = digitalRead(offButton);    
  }
  lcd.clear();
  displayController();
  return;
}

void onController(){
  Mode = "On   ";
  pauseState = false;
  offState = false;
  
  displayController();
  onLoop();
}

void pauseController(){
  displayController();
}

void offController(){
  Mode = "Off  ";
  pauseState = false;
  onState = false;
  
  displayController();
  offLoop();
}

void upController(){
  if(updateInterval == 60){updateInterval = 1;}
  else{updateInterval += 1;}
  updateTimeInterval();
}

void downController(){
  if(updateInterval == 1){updateInterval = 60;}
  else{updateInterval -= 1;}
  updateTimeInterval();
}

/* should be 24 characters long
 *      Time: 00:00 updIntvl: 00  24/24 chars 
 *      Mode: _____ ____________   16/24 chars
 */
void displayController(){
  //(column, row)
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time: ");
  currentTime = getTime();

  lcd.setCursor(6,0);
  lcd.print(currentTime.tm_hour);
  lcd.print(":");
  lcd.print(currentTime.tm_min);
  
  lcd.setCursor(12,0);
  lcd.print("updIntvl:");
  
  lcd.setCursor(23,0);
  lcd.print(updateInterval);
  
  lcd.setCursor(0,1);
  lcd.print("Mode: ");

  lcd.setCursor(6,1);
  lcd.print(Mode);  
  return;
}

void updateTimeInterval(){
  lcd.setCursor(22,0);
  lcd.print("  ");
  lcd.setCursor(22,0);
  lcd.print(updateInterval);
}

void timeUpdateController(){
  currentTime = getTime();
  lcd.setCursor(6,0);
  lcd.print(currentTime.tm_hour);
  lcd.print(":");
  lcd.print(currentTime.tm_min);
  return;
}

void modeUpdateController(){
  lcd.setCursor(6,1);
  lcd.print(Mode);  
  return;
}

// Sending command to both the serial moniter and BTT SKR
void sendCommand(char* command){
    Serial.print(command);
    Serial.print("\n");
    Serial1.print(command);
    Serial1.print("\n");
    delay(500);
}

////////////////////////////////
// Carriage Control Functions //
////////////////////////////////

// Homes the marker, caps it
void rest(){
  reHome();
  capMarker();
}

// Puts the eraser down, ready to erase
void eraserDown(){
  printf(";eraserDown\n");
  sendCommand("M280 P0 S10"); //this is a guess, needs to be changed when eraser is put on
}

// Raises the Marker
void markerHome(){
  printf(";markerRaised\n");
  sendCommand("M280 P0 S40");
}

// Brings the Marker to (1, 4)
void capMarker(){
  printf(";capMarker\n");
  sendCommand("G0 X1 Y4");
  markerCap();
 
}

// Lowers to the marker, ready to write
void markerWrite(){
  printf(";markerWrite\n");
  sendCommand("M280 P0 S135");
}

// Lowers the Marker enough to cap it, sends rest command to motors
void markerCap(){
  printf(";markerCap\n");
  sendCommand("M280 P0 S127");
  sendCommand("M84");
}

// Allows reHome() call with no arguments, assumed homing both axis
void reHome(){
  reHome(true, true);
}

// Sets isHome to true, rehomes x, then y axis, waits for finished before any new commands "M400"
void reHome(bool x, bool y){
  isHomed=true;
  markerHome();
  char xChar = ' ';
  if (x){xChar='X';}
  char yChar = ' ';
  if (y){yChar='Y';}
  char nextCommand[COMMAND_LENGTH];
  snprintf(nextCommand,COMMAND_LENGTH, "G28 %c%c",xChar,yChar);
  sendCommand(nextCommand); 
  sendCommand("M400");
}

// Accepts (x,y) coordinate, travels straight to location
void goToXY(double x, double y){
  char nextCommand[COMMAND_LENGTH];
  snprintf(nextCommand,COMMAND_LENGTH, "G1 X%fY%f F6000",x,y); 
  // F10000, 8000 12 well, colon half way and fails on 3 default speed, try to mess around with to fix hardware inconsistentcies
  // 12000 to fast, works, but seems to lose itself at the end, accuracy wise, but no stalls
  // 6000 seems to work rather well
  // setting max acceleration, hoepfully to keep far cradle from getting stuck
  //sendCommand("M201");
  sendCommand(nextCommand);
  sendCommand("M400");
  //delay(2000);
}


///////////////////////////////////////
// Movement Logic For Drawing Digits //
///////////////////////////////////////

void drawDigit(int number, int digit, int erase){
  switch (number){
    case 0:
      digitZero(digit, erase);
      break;
    case 1:
      digitOne(digit, erase);
      break;
    case 2:
      digitTwo(digit, erase);
      break;
    case 3:
      digitThree(digit, erase);
      break;
    case 4:
      digitFour(digit, erase);
      break;
    case 5:
      digitFive(digit, erase);
      break;
    case 6:
      digitSix(digit, erase);
      break;
    case 7:
      digitSeven(digit, erase);
      break;
    case 8:
      digitEight(digit, erase);
      break;
    case 9:
      digitNine(digit, erase);
      break;
    default:
      printf(";error, invalid number passed to drawDigit");
  }
}

// Patterns for the Digits, 0-9
// *** MAKE SURE MARKER IS UP BEFORE CALLING ***
void digitZero(int digit, bool erase){
  segmentF(digit, erase);
  segmentE(digit, erase);
  segmentD(digit, erase);
  segmentA(digit, erase);
  segmentB(digit, erase);
  segmentC(digit, erase);
}

void digitOne(int digit, bool erase){
  segmentB(digit, erase);
  segmentC(digit, erase);
}

void digitTwo(int digit, bool erase){ // flawed digit?
  segmentA(digit, erase);
  segmentB(digit, erase);
  segmentG(digit, erase);
  segmentE(digit, erase);
  segmentD(digit, erase);
}

void digitThree(int digit, bool erase){
  segmentA(digit, erase);
  segmentB(digit, erase);
  segmentC(digit, erase);
  segmentD(digit, erase);
  segmentG(digit, erase);
}

void digitFour(int digit, bool erase){
  segmentF(digit, erase);
  segmentG(digit, erase);
  segmentC(digit, erase);
  segmentB(digit, erase);
}

void digitFive(int digit, bool erase){
  segmentA(digit, erase);
  segmentF(digit, erase);
  segmentG(digit, erase);
  segmentC(digit, erase); 
  segmentD(digit, erase);
}

void digitSix(int digit, bool erase){
  segmentF(digit, erase);
  segmentG(digit, erase);
  segmentC(digit, erase); 
  segmentE(digit, erase);
  segmentD(digit, erase);
  segmentA(digit, erase);
}

void digitSeven(int digit, bool erase){
  segmentA(digit, erase);
  segmentB(digit, erase);
  segmentC(digit, erase);
}

void digitEight(int digit, bool erase){
  segmentF(digit, erase);
  segmentE(digit, erase);
  segmentD(digit, erase);
  segmentA(digit, erase);
  segmentB(digit, erase);
  segmentC(digit, erase);
  segmentG(digit, erase);
}

void digitNine(int digit, bool erase){
  segmentF(digit, erase);
  segmentG(digit, erase);
  segmentC(digit, erase); 
  segmentA(digit, erase);
  segmentB(digit, erase);
}

// Patterns for the Segments a-g, and colon
// *** MAKE SURE MARKER IS UP BEFORE CALLING ***
void dividingColon(int digit, bool erase){
  markerHome();
  
  int X = 15 + digit;
  int Y = 250;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  //now at start of top one
  sendCommand("G3 I0 J10 F1500");
  
  markerHome();

  Y -= 100;
  goToXY(X, Y);
  
  if(erase){eraserDown();}
  else{markerWrite();}
  
  //now at start of bottom one
  sendCommand("G2 I0 J10 F1000"); // 3000 seems to fast, only does like half, 1500 does 3/4

  markerHome();
}

void segmentA(int digit, bool erase){
  markerHome();

  int X = aX + digit;
  int Y = aY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  X += shortLength;
  goToXY(X, Y);
  
  markerHome();
}

void segmentB(int digit, bool erase){
  markerHome();
  
  int X = bX + digit;
  int Y = bY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  Y -= longLength;
  goToXY(X, Y);
  
  markerHome();
}

void segmentC(int digit, bool erase){
  markerHome();
  
  int X = cX + digit;
  int Y = cY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  Y -= longLength;
  goToXY(X, Y);
  
  markerHome();
}

void segmentD(int digit, bool erase){
  markerHome();
  
  int X = dX + digit;
  int Y = dY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  X += shortLength;
  goToXY(X, Y);
  
  markerHome();
}

void segmentE(int digit, bool erase){
  markerHome();
  
  int X = eX + digit;
  int Y = eY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  Y -= longLength;
  goToXY(X, Y);
  
  markerHome();
}

void segmentF(int digit, bool erase){
  markerHome();
  
  int X = fX + digit;
  int Y = fY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  Y -= longLength;
  goToXY(X, Y);
  
  markerHome();
}

void segmentG(int digit, bool erase){
  markerHome();
 
  int X = gX + digit;
  int Y = gY;
  
  if(erase){
    X += eraserX;
    Y += eraserY;
    goToXY(X, Y);
    eraserDown();
  }
  else{
    goToXY(X, Y);
    markerWrite();
  }

  X += shortLength;
  goToXY(X, Y);
  
  markerHome();
}

// old, not using wifi server to control
/*
void wifiLoop(){
  WiFiClient client = server.available();

  if(client){
    String currentLine = "";
    while(client.connected()){
      currTime = millis();
      if(start == true){
        clockLoop();
        Serial.println("checking for update");
      }
      if(client.available()){
        char c = client.read();
        Serial.write(c);
        header += c;
        if(c == '\n'){
          if(currentLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if(header.indexOf("GET /updateInterval/2") >= 0){
              Serial.println("update interval: 2 minutes");
              updateInterval = 2;
            }
            else if(header.indexOf("GET /updateInterval/5") >= 0){
              Serial.println("update interval: 5 minutes");
              updateInterval = 5;
            }
            else if(header.indexOf("GET /updateInterval/10") >= 0){
              Serial.println("update interval: 10 minutes");
              updateInterval = 10;
            }
            else if(header.indexOf("GET /updateInterval/20") >= 0){
              Serial.println("update interval: 20 minutes");
              updateInterval = 20;
            }
            else if(header.indexOf("GET /start") >= 0){
              Serial.println("starting the clock");
              start = true;
            }
            else if(header.indexOf("GET /erase") >= 0){
              Serial.println("eraseing the clock");
              eraseAll();
            }
            else if(header.indexOf("GET /now") >= 0){
              Serial.println("drawing current time");
              drawNow();
            }
            else if(header.indexOf("GET /stop") >= 0){
              Serial.println("stopping the clock");
              rest();
            }
          

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
  
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
  
            client.println("<body><h1> CNC Clock Controller</h1>");
  
            client.println("<p><a href=\"/start\"><button class=\"button button2\">start clock</button></a></p>");
            client.println("<p><a href=\"/erase\"><button class=\"button button2\">erase clock</button></a></p>");
            client.println("<p><a href=\"/now\"><button class=\"button button2\">draw time</button></a></p>");
            client.println("<p><a href=\"/stop\"><button class=\"button button2\">stop now</button></a></p>");
  
            switch(updateInterval){
              case 2:
                client.println("<p><a href=\"/updateInterval/2\"><button class=\"button\">2 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/5\"><button class=\"button button2\">5 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/10\"><button class=\"button button2\">10 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/20\"><button class=\"button button2\">20 Minute</button></a></p>");
                break;
  
              case 5:
                client.println("<p><a href=\"/updateInterval/2\"><button class=\"button button2\">2 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/5\"><button class=\"button\">5 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/10\"><button class=\"button button2\">10 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/20\"><button class=\"button button2\">20 Minute</button></a></p>");
                break;
  
              case 10:
                client.println("<p><a href=\"/updateInterval/2\"><button class=\"button button2\">2 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/5\"><button class=\"button button2\">5 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/10\"><button class=\"button\">10 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/20\"><button class=\"button button2\">20 Minute</button></a></p>");
                break;
  
              case 20:
                client.println("<p><a href=\"/updateInterval/2\"><button class=\"button button2\">2 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/5\"><button class=\"button button2\">5 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/10\"><button class=\"button button2\">10 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/20\"><button class=\"button\">20 Minute</button></a></p>");
                break;
  
              default:
                client.println("<p><a href=\"/updateInterval/2\"><button class=\"button button2\">2 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/5\"><button class=\"button button2\">5 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/10\"><button class=\"button button2\">10 Minute</button></a></p>");
                client.println("<p><a href=\"/updateInterval/20\"><button class=\"button button2\">20 Minute</button></a></p>");
                break;
              }
            }
            else{
              currentLine = "";
            }
          }
          else if(c != '\r'){
            currentLine += c;
          }  
      }
    }
  }
  header = "";
  client.stop();
  Serial.println("Client disconnected");
  Serial.println("");
}
*/
