#include <EEPROM.h>
#include <Bounce2.h>
#define WITH_LCD 1
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>
#define BUTTON_PIN_1 2
#define BUTTON_PIN_2 3
#define StepsPerNotch (1)

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//Initialize Variables
ClickEncoder *encoder;

int16_t last = -1, value;
int pwmToMotor = 6;  //Set PWM Pin
const int motorEnable = 5; //Set Motor Enable Pin
int profileAddress = 0, rpm = 0, tempRpm = 0, tempTime = 0, timerValueHours = 0, timerTimerValueHours = 0, cursorPosition = 10, encoderValue = 0, tempHours = 0;
byte currentPage = 0, drawnPage = 0, numberOfSelections = 0, selectionValue = 0, timerValueMinutes = 0, timerTimerValueMinutes = 0, arrayNum = 0, currentProfile = 0, charNum = 0, timerDutyCycle = 0, tempMins = 0;
unsigned long timeRemainingMs = 0, timeRemainingMins = 0, previousMillis = 0, timerTimeRemainingMins = 0, timerTimeRemainingMs = 0;
char Str2[50] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-', '+', '=', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
char tempProfileName[16];
boolean isFirstHours = true, isFirstMins = true, isStartPressed = false, isStopPressed = false;
typedef struct {
  unsigned long runTimeInMs;
  int dutyCycle;
  char profileName[16];
  int profileLength;
} settingData;

settingData settings = {
  0,
  0,
  ' ',
  0
};

void timerIsr() {
  encoder->service();
}

Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce(); 


//****************************************************************************************************************************************************************************************
//                                                                                            SETUP BEGIN
//****************************************************************************************************************************************************************************************


void setup() {
  
  pinMode(pwmToMotor, OUTPUT);  //Set PWM Pin as an Output
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);  //Set Red Button Pin as an Input
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);  //Set Green Button Pin as an Input
  pinMode(motorEnable, OUTPUT);  //Set Motor Enable Pin as an Output
  debouncer1.attach(BUTTON_PIN_1);
  debouncer1.interval(50);
  debouncer2.attach(BUTTON_PIN_2);
  debouncer2.interval(50);
  
  encoder = new ClickEncoder(A1, A0, A2);
 
  tft.begin();

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    while (1);
  }

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 

  tft.setRotation(1); //Set the screen rotation to landscape
  drawHomeScreen(); //Draw the home screen
  
}

//****************************************************************************************************************************************************************************************
//                                                                                            SETUP END
//****************************************************************************************************************************************************************************************




//****************************************************************************************************************************************************************************************
//                                                                                            LOOP BEGIN
//****************************************************************************************************************************************************************************************

void loop() {
  debouncer1.update();
  debouncer2.update();

  int value1 = debouncer1.read();
  int value2 = debouncer2.read();

  if (value1 == LOW) {
    digitalWrite(5, HIGH);
  }

  if (value2 == LOW) {
    analogWrite(pwmToMotor, 0);
    delay(1500);
    digitalWrite(5, LOW);
  } 



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 1 (HOME SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------                                                                                            
  
// If current page is Home Screen 
  
  if (currentPage == 1){  //If current page is the Home Screen
  
  

    if (drawnPage != 1){  //draw the home screen if for some reason you got here without drawing it yet
      drawHomeScreen();
    }
    
    if (ctp.touched()){   
      // Retrieve a point  
      TS_Point p = ctp.getPoint(); 
      // rotate coordinate system
      // flip it around to match the screen.
      p.x = map(p.x, 0, 240, 240, 0);
      p.y = map(p.y, 0, 320, 320, 0);
      int y = tft.height() - p.x;
      int x = p.y;  

      // Check for touch on top button and send to Manual screen  
      if((x > 30) && (x < 289)) {  //set the touch area bounds Xmin and Xmax
      if ((y > 30) && (y < 70)) { //set the touch area bounds Ymin and Ymax
        drawManualScreen();  //If touch point lands in area defined above, draw the Manual screen
      }
      }
  
      // Check for touch on top button and send to Timer screen  
      if((x > 30) && (x < 289)) {  //set the touch area bounds Xmin and Xmax
      if ((y > 80) && (y < 120)) { //set the touch area bounds Ymin and Ymax
        drawTimerScreen();  //If touch point lands in area defined above, draw the Manual screen
      }
      }
  
      // Check for touch on top button and send to Auto screen  
      if((x > 30) && (x < 289)) {  //set the touch area bounds Xmin and Xmax
      if ((y > 130) && (y < 170)) { //set the touch area bounds Ymin and Ymax
        drawAutoScreen();  //If touch point lands in area defined above, draw the Manual screen
      }
      }
  
      // Check for touch on top button and send to Profile screen  
      if((x > 30) && (x < 289)) {  //set the touch area bounds Xmin and Xmax
      if ((y > 180) && (y < 220)) { //set the touch area bounds Ymin and Ymax
        drawProfileChooser();  //If touch point lands in area defined above, draw the Manual screen
      }
      }
    }
  }



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 2 (MANUAL) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// If current page is Manual Screen 
  if (currentPage == 2){
    if (drawnPage != 2){
      drawManualScreen();
    }

    encoder->setAccelerationEnabled(true);
    value += encoder->getValue();
  
    if (value != last) {
      last = value;
    } 
     
    if (value < 0) {
      value = 0;
    }
    
    if (value > 3000) {
      value = 3000;
    }
  
    settings.dutyCycle = map(value, 0, 3000, 0, 254);
    analogWrite(pwmToMotor, settings.dutyCycle);
    rpm = map(settings.dutyCycle, 0, 254, 0, 3000);
    tft.setTextSize(2);  
    tft.setCursor(95, 73);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(rpm);
    tft.print("   ");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);



    if (digitalRead(5) == HIGH) {
      tft.setCursor(25, 190);
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setTextSize(3);
      tft.print("Wait");
      if (isStartPressed == false) {
        tft.fillRect(105, 180, 320, 240, ILI9341_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        tft.setCursor(125, 182);
        tft.print("Press red stop");
        tft.setCursor(115, 202); 
        tft.print("button to cancel");
      }
      isStartPressed = true;
      isStopPressed = false;
    }
    
    if (digitalRead(5) == LOW) {

      tft.setCursor(25, 190);
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setTextSize(3);
      tft.print("Back");

      if (isStopPressed == false) {
        
        tft.fillRect(105, 180, 320, 240, ILI9341_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        tft.setCursor(107, 182);
        tft.print("Set RPM and press");
        tft.setCursor(135, 202); 
        tft.print("start button");
        isStopPressed = true;
        isStartPressed = false;
      }
      
      
    }
    
    if (ctp.touched()){   
      // Retrieve a point  
      TS_Point p = ctp.getPoint(); 
      // rotate coordinate system
      // flip it around to match the screen.
      p.x = map(p.x, 0, 240, 240, 0);
      p.y = map(p.y, 0, 320, 320, 0);
      int y = tft.height() - p.x;
      int x = p.y;  
      if (digitalRead(5) == LOW) {
        if((x > 20) && (x < 100)) {
        if ((y > 180) && (y < 220)) {
  
          drawHomeScreen();
        
        }
        }
      }
    }  
  }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 3 (TIMER SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Timer Screen
   
  if (currentPage == 3){
  if (selectionValue == 1) {
    encoder->setAccelerationEnabled(true);
    tempRpm += encoder->getValue();
    
    if (tempRpm != last) {
      last = tempRpm;
    } 
     
    if (tempRpm< 0) {
      tempRpm = 0;
    }
    
    if (tempRpm > 3000) {
      tempRpm = 3000;
    }
  
    timerDutyCycle = map(tempRpm, 0, 3000, 0, 254);
    rpm = map(timerDutyCycle, 0, 254, 0, 3000);
    
  }
  
  if (selectionValue == 2) {
    
    encoder->setAccelerationEnabled(false);
    tempHours += (encoder->getValue());
    timerTimerValueHours = tempHours * 0.55;
    
    
    if (timerTimerValueHours != last) {
      last = timerTimerValueHours;
    }  
    
    if (timerTimerValueHours < 0) {
      timerTimerValueHours = 0;
    }
    
    if (timerTimerValueHours > 999) {
      timerTimerValueHours = 999;
    }

  }
  
  if (selectionValue == 3) {
     
    encoder->setAccelerationEnabled(false);
    tempMins += (encoder->getValue());
    timerTimerValueMinutes = tempMins * 0.5;
  
    if (timerTimerValueMinutes != last) {
      last = timerTimerValueMinutes;
    }  
    
    if (timerTimerValueMinutes < 0) {
      timerTimerValueMinutes = 0;
    }
    
    if (timerTimerValueMinutes > 59) {
      timerTimerValueMinutes = 59;
    }
  }
  
  timerTimeRemainingMs = (timerTimerValueHours * 3600000) + (timerTimerValueMinutes * 60000);
  timerTimeRemainingMins = (timerTimeRemainingMs / 60000);
  
  if (selectionValue == 1) {

    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   "); 
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(95, 103);
    tft.print(timerTimerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.print(timerTimerValueMinutes);
    tft.print("   ");    

  }

  if (selectionValue == 2) {

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   "); 
    tft.setCursor(95, 103);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(timerTimerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(timerTimerValueMinutes);
    tft.print("   ");

    

  }

  if (selectionValue == 3) {

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   ");
    tft.setCursor(95, 103);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(timerTimerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(timerTimerValueMinutes);
    tft.print("   ");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    

  }

  ClickEncoder::Button b = encoder->getButton();
 
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
      selectionValue++;
      if (selectionValue > numberOfSelections) {
        selectionValue = 1;
      }
      
      break;
      
    } 
  }
          
  debouncer1.update();
  int value1 = debouncer1.read();
  
  if (value1 == LOW) {
    selectionValue = 1;
    timerLoop(timerTimeRemainingMs, timerDutyCycle);
  }

  if (ctp.touched()){   
    // Retrieve a point  
    TS_Point p = ctp.getPoint(); 
    // rotate coordinate system
    // flip it around to match the screen.
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {
      selectionValue = 1;
      drawHomeScreen();
    
    }
    }
   }
 }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 4 (AUTOMATIC SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  

// If current page is Auto Screen 

  if (currentPage == 4){



  if (ctp.touched()){   
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {
    
      drawHomeScreen();
    
    }
    }
    
    if ((x > 14) && (x < 64)){
    if ((y > 64) && (y < 114)) {
    
      drawRunScreen(1);
    
    }
    }
    
    if ((x > 74) && (x < 124)){
    if ((y > 64) && (y < 114)) {
    
      drawRunScreen(2);
    
    }
    }
    
    if ((x > 134) && (x < 184)){
    if ((y > 64) && (y < 114)) {
    
      drawRunScreen(3);
    
    }
    }
    
    if ((x > 194) && (x < 244)){
    if ((y > 64) && (y < 114)) {
    
      drawRunScreen(4);
    
    }
    }
    if ((x > 254) && (x < 304)){
    if ((y > 64) && (y < 114)) {
    
      drawRunScreen(5);
    
    }
    }
    
    if ((x > 14) && (x < 64)){
    if ((y > 105) && (y < 155)) {
    
      drawRunScreen(6);
    
    }
    }
    
    if ((x > 74) && (x < 124)){
    if ((y > 105) && (y < 155)) {
    
      drawRunScreen(7);
    
    }
    }
    
    if ((x > 134) && (x < 184)){
    if ((y > 105) && (y < 155)) {
    
      drawRunScreen(8);
    
    }
    }
    
    if ((x > 194) && (x < 244)){
    if ((y > 105) && (y < 155)) {
    
      drawRunScreen(9);
    
    }
    }
    
    if ((x > 254) && (x < 304)){
    if ((y > 105) && (y < 155)) {
    
      drawRunScreen(10);
    
    }
    }
  }

 }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 5 (PROFILE CHOOSER SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// If current page is Profile Screen 

  if (currentPage == 5){
  if (ctp.touched()){    
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;
      
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {

      drawHomeScreen();
  
    }
    }
    
    if ((x > 14) && (x < 64)){
    if ((y > 64) && (y < 114)) {

      drawProfileEditor(1);
      
    }
    }
    
    if ((x > 74) && (x < 124)){
    if ((y > 64) && (y < 114)) {

      drawProfileEditor(2);
      
    }
    }
    
    if ((x > 134) && (x < 184)){
    if ((y > 64) && (y < 114)) {

      drawProfileEditor(3);
      
    }
    }
    
    if ((x > 194) && (x < 244)){
    if ((y > 64) && (y < 114)) {

      drawProfileEditor(4);
      
    }
    }
    
    if ((x > 254) && (x < 304)){
    if ((y > 64) && (y < 114)) {

      drawProfileEditor(5);
      
    }
    }
    
    if ((x > 14) && (x < 64)){
    if ((y > 105) && (y < 155)) {

      drawProfileEditor(6);
      
    }
    }
    
    if ((x > 74) && (x < 124)){
    if ((y > 105) && (y < 155)) {

      drawProfileEditor(7);
      
    }
    }
    
    if ((x > 134) && (x < 184)){
    if ((y > 105) && (y < 155)) {

      drawProfileEditor(8);
      
    }
    }
    
    if ((x > 194) && (x < 244)){
    if ((y > 105) && (y < 155)) {

      drawProfileEditor(9);
      
    }
    }
    
    if ((x > 254) && (x < 304)){
    if ((y > 105) && (y < 155)) {

      drawProfileEditor(10);
      
    }
    }
  }
  
 }


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 6 (PROFILE VIEW) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Profile Screen 
  

  if (currentPage == 6){
  if (ctp.touched()){   
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {

      drawProfileChooser();
  
    }
    }
  
    
    if((x > 220) && (x < 300)) {
    if ((y > 180) && (y < 220)) {

      drawEditScreen();
  
    }
    }
  }
 }

 //----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 8 (RENAME SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Profile Screen 
 
  if (currentPage == 8){
  int numberOfCharacters = 15;
  encoder->setAccelerationEnabled(false);
  encoderValue += (encoder->getValue());
  charNum = map(encoderValue, 0, 400, 0, 50);
  
  if (encoderValue != last) {
    last = encoderValue;
  }  
  
  if (encoderValue > 400) {
    encoderValue = 0;
  }
  
  if (encoderValue < 0) {
    encoderValue = 400;
  }
  
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(cursorPosition, 110);
  tft.print(Str2[charNum]);
  tempProfileName[arrayNum] = Str2[charNum];
  
  ClickEncoder::Button b = encoder->getButton();
 
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
      cursorPosition += 12;
      tft.setCursor(cursorPosition, 35);
      arrayNum++;        
      drawSaveButton();
      
        if (cursorPosition > (130 + numberOfCharacters + (numberOfCharacters * 11 - 1))) {
          arrayNum = 1;
          cursorPosition = 130;
        }
      
      break;
    } 
  }

  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::DoubleClicked:
      cursorPosition -= 12;
      arrayNum--;
      tft.setCursor(cursorPosition, 35);
        if (cursorPosition < 130) {
          arrayNum = 1;
          cursorPosition = 130;
        }
      
      break;
    } 
  } 
 
  if (ctp.touched()){   
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {

      memset(tempProfileName, 0, sizeof(tempProfileName));
      drawProfileChooser();
  
    }
    }
    
  if ((x > 120) && (x < 200)){
  if ((y > 180) && (y < 220)){
  
    memset(settings.profileName, 0, sizeof(settings.profileName));
  
  for (int i = 0; i < sizeof(tempProfileName); i++) {
    settings.profileName[i] = tempProfileName[i];
  }
  
  int j = 0;
  
  while (settings.profileName[j] != '\0') {
    j++;
  }
    
  int address = ((currentProfile - 1) * 100);
   
  for (int i = 0; i < j; i++) {
    EEPROM.put(address + i, settings.profileName[i]);
  }
  
    EEPROM.put(address + 35, j);
  
  memset(tempProfileName, 0, sizeof(tempProfileName));
  drawProfileEditor(currentProfile);

  }
  }      
 }
    
 }


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 9 (RUN PROFILE) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// If current page is Profile Screen 
  
  if (currentPage == 9){

  debouncer1.update();
  int value1 = debouncer1.read();
  if (value1 == LOW) {
  timerLoop(settings.runTimeInMs, settings.dutyCycle); 
  }

  if (ctp.touched()){    
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {

      drawAutoScreen();
  
    }
    }

  }
 }


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 10 (EDIT PROFILE) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Profile Screen 
  
 
  if (currentPage == 10){
  if (ctp.touched()){   
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {

      drawProfileEditor(currentProfile);
  
    }
    }


     if ((x > 120) && (x < 200)){
     if ((y > 180) && (y < 220)){

      
        drawRenameScreen(currentProfile);    
      
    
    }
    }   
    
    if((x > 220) && (x < 300)) {
    if ((y > 180) && (y < 220)) {

      
      drawSpecScreen(currentProfile);
       
    }
    }
  }
 }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            PAGE 11 (EDIT SPEC) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Profile Screen 
  
 
  if (currentPage == 11){

  if (selectionValue == 1) {
    
    encoder->setAccelerationEnabled(true);
    tempRpm += encoder->getValue();
    
    if (tempRpm != last) {
      last = tempRpm;
    }  
    
    if (tempRpm < 0) {
      tempRpm = 0;
    }
    
    if (tempRpm > 3000) {
      tempRpm = 3000;
    }
  
    settings.dutyCycle = map(tempRpm, 0, 3000, 0, 254);
    rpm = map(settings.dutyCycle, 0, 254, 0, 3000);
    
  }
  
  if (selectionValue == 2) {

    if (isFirstHours == true) {
      last = timerValueHours;
      tempHours = timerValueHours*2;
      isFirstHours = false;
    }
      
    encoder->setAccelerationEnabled(false);
    tempHours += (encoder->getValue());    
    timerValueHours = tempHours * 0.5;
  
    if (timerValueHours != last) {
      last = timerValueHours;
    }  
    
    if (timerValueHours < 0) {
      timerValueHours = 0;
    }
    
    if (timerValueHours > 999) {
      timerValueHours = 999;
    }

  }
  
  if (selectionValue == 3) {
    
    if (isFirstMins == true) {
      last = timerValueMinutes;
      tempMins = timerValueMinutes*2;
      isFirstMins = false;
    }

    encoder->setAccelerationEnabled(false);
    tempMins += (encoder->getValue());
    timerValueMinutes = tempMins * 0.5;
  
    if (timerValueMinutes != last) {
      last = timerValueMinutes;
    }  
    
    if (timerValueMinutes < 0) {
      timerValueMinutes = 0;
    }
    
    if (timerValueMinutes > 59) {
      timerValueMinutes = 59;
    }
  }
  
  timeRemainingMs = (timerValueHours * 3600000) + (timerValueMinutes * 60000);
  timeRemainingMins = (timeRemainingMs / 60000);
  

  if (selectionValue == 1) {

    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   "); 
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(95, 103);
    tft.print(timerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.print(timerValueMinutes);
    tft.print("   ");    

  }

  if (selectionValue == 2) {

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   "); 
    tft.setCursor(95, 103);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(timerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(timerValueMinutes);
    tft.print("   ");

  }

  if (selectionValue == 3) {

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   ");
    tft.setCursor(95, 103);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(timerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    tft.print(timerValueMinutes);
    tft.print("   ");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    

  }



  ClickEncoder::Button b = encoder->getButton();
 
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
      selectionValue++;
      if (selectionValue > 3) {
        selectionValue = 1;        
      }
      break;
    } 
  }
  

  if (ctp.touched()){    
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {

      drawProfileEditor(currentProfile);
  
    }
    }
  
    
    if((x > 220) && (x < 300)) {
    if ((y > 180) && (y < 220)) {

      drawRenameScreen(currentProfile);
  
    }
    }

     if ((x > 120) && (x < 200)){
     if ((y > 180) && (y < 220)){
            
      int address = ((currentProfile - 1) * 100);
      
      EEPROM.put(address + 40, settings.dutyCycle);
      EEPROM.put(address + 50, timeRemainingMs);  
      drawProfileEditor(currentProfile);
    
    }
    }      
  }
 }

}

//****************************************************************************************************************************************************************************************
//                                                                                             LOOP END
//****************************************************************************************************************************************************************************************


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                 DRAW SPEC SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawSpecScreen(int profNumber) {

  currentProfile = profNumber;
  digitalWrite(5, LOW);  
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  drawSaveButton();
  encoderValue = 0;
  arrayNum = 0;  
  drawBackButton();
  selectionValue = 1;
  tempHours = (settings.runTimeInMs / 3600000);
  tempMins = ((settings.runTimeInMs % 3600000) / 60000);
  tempRpm = map(settings.dutyCycle, 0, 254, 0, 3000);
  timerValueHours = (settings.runTimeInMs / (60000 * 60));
  timerValueMinutes = ((settings.runTimeInMs % 3600000) / 60000);
  tempHours = (settings.runTimeInMs / (60000 * 60));
  tempMins = ((settings.runTimeInMs % 3600000) / 60000);
  isFirstHours = true;
  isFirstMins = true;
  drawProfileInfoBlock(2);
  drawBackButton();
  drawSaveButton();
  currentPage = 11;
  drawnPage = 11;

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                 DRAW RENAME SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawRenameScreen(int profNumber) {

  digitalWrite(5, LOW);  
  tft.fillScreen(ILI9341_BLACK);
  centerText("Rename Profile", 2, 10);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  centerText("Turn knob to change character.  Press knob to", 1, 29);
  centerText("accept and choose next character.  Double press knob", 1, 39);
  centerText("to move back one character.  15 character maximum", 1, 49);
  tft.setTextSize(2);
  tft.setCursor(10, 80);
  tft.print("Old Name: ");
  printProfileName(profNumber);
  tft.setCursor(10, 110);
  tft.print("New Name: ");
  drawSaveButton();
  cursorPosition = 130;
  encoderValue = 0;
  arrayNum = 0;  
  drawBackButton();
  currentPage = 8;
  drawnPage = 8;
  memset(tempProfileName, 0, sizeof(tempProfileName));
  currentProfile = profNumber;
  selectionValue = 1;
  

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                 DRAW RUN SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawRunScreen(int profNumber) {
  
  currentProfile = profNumber;
  tft.fillScreen(ILI9341_BLACK);
  eepromGetAll(profNumber);
  drawProfileInfoBlock(1);
  arrayNum = 0;  
  drawBackButton();
  currentPage = 9;
  drawnPage = 9;
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(104, 182);
  tft.print("Press start button");
  tft.setCursor(125, 202); 
  tft.print("to run profile");
  

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR PRINTING PROFILE NAME
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void printProfileName (int profNumber) {
      
  int address = ((profNumber - 1) * 100);

  EEPROM.get(address + 35, settings.profileLength);
  
  for (int i = 0; i < settings.profileLength; i++) {
    EEPROM.get(address + i, settings.profileName[i]);
  }
    tft.print(settings.profileName);
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                 DRAW PROFILE EDITOR
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawProfileEditor(int profNumber) {
  
  currentProfile = profNumber; 
  eepromGetAll(profNumber);
  tft.fillScreen(ILI9341_BLACK);
  drawProfileInfoBlock(2);
  drawEditButton();
  drawBackButton();
  currentPage = 6;
  drawnPage = 6;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         DRAW HOME SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawHomeScreen() {

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(110, 10);
  tft.print("Welcome!");
  tft.setTextSize(3);
  tft.setCursor(60, 39);
  tft.print("Manual Mode");
  tft.drawRoundRect(30, 30, 260, 40, 5, ILI9341_ORANGE);
  tft.setCursor(68, 89);
  tft.print("Timer Mode");
  tft.drawRoundRect(30, 80, 260, 40, 5, ILI9341_ORANGE);
  tft.setCursor(35, 139);
  tft.print("Automatic Mode");
  tft.drawRoundRect(30, 130, 260, 40, 5, ILI9341_ORANGE);
  tft.setCursor(90, 189);
  tft.print("Profiles");
  tft.drawRoundRect(30, 180, 260, 40, 5, ILI9341_ORANGE);  
  currentPage = 1;
  drawnPage = 1;  
 
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW MANUAL SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawManualScreen() {
  digitalWrite(5, LOW);
  tft.fillScreen(ILI9341_BLACK);
  drawBackButton();
  drawProfileInfoBlock(3);
  centerText("Manual Mode", 2, 12);
  centerText("Use knob to set RPM", 1, 30);
  currentPage = 2;
  drawnPage = 2;
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(107, 182);
  tft.print("Set RPM and press");
  tft.setCursor(135, 202); 
  tft.print("start button");
  isStopPressed = false;
  isStartPressed = false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         DRAW TIMER SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawTimerScreen() {
  digitalWrite(5, LOW);  

  tft.fillScreen(ILI9341_BLACK);
  drawProfileInfoBlock(4);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  centerText("Timer Mode", 2, 12);
  centerText("Press knob to change selection", 1, 30);

  tempRpm = 0;
  tempTime = 0;
  timerDutyCycle = 0;
  timerTimeRemainingMins = 0;
  timerTimerValueHours = 0;
  timerTimerValueMinutes = 0;
  timerTimeRemainingMs = 0; 
  numberOfSelections = 3;
  tempHours = 0;
  tempMins = 0;
  drawBackButton();
  currentPage = 3;
  drawnPage = 3;
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(104, 182);
  tft.print("Press start button");
  tft.setCursor(134, 202); 
  tft.print("to run timer");
  selectionValue = 1;  

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         DRAW EDIT SELECTION SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawEditScreen() {

  drawProfileInfoBlock(2);
  drawBackButton();
  drawNameButton();
  drawSpecButton();
  currentPage = 10;
  drawnPage = 10;
  

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                      DRAW AUTOMATIC SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawAutoScreen() {

  digitalWrite(5, LOW);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(110, 10);
  centerText("Automatic Mode", 2, 12);
  draw10Buttons();
  drawBackButton();
  currentPage = 4;
  drawnPage = 4;
  

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                       DRAW PROFILE CHOOSER SCREEN
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawProfileChooser() {

  digitalWrite(5, LOW);
  arrayNum = 0;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(60, 10);
  centerText("Profile Manager", 2, 12);
  draw10Buttons();
  drawBackButton();
  currentPage = 5;
  drawnPage = 5;
  

}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW BACK BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawBackButton () {
  tft.drawRoundRect(20, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(25, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Back");
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW SAVE BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawSaveButton () {
  tft.drawRoundRect(120, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(125, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Save");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW RUN BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawRunButton () {
  tft.drawRoundRect(120, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(130, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Run");
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW EDIT BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawEditButton () {
  tft.drawRoundRect(220, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(225, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Edit");
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW NAME BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawNameButton () {
  tft.drawRoundRect(120, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(125, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Name");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW SPEC BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawSpecButton () {
  tft.drawRoundRect(220, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(225, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Spec");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                     CODE FOR TIMER LOOP
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void timerLoop (unsigned long timeInMs, int duty) {

  
  unsigned long startTime = millis();
  unsigned long finishTime = startTime + timeInMs;
  unsigned long nowTime; 
  int secondsTimer = 0;


  digitalWrite(5, HIGH);
  analogWrite(pwmToMotor, duty);
  tft.setCursor(25, 190);
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Wait");
  tft.fillRect(103, 180, 320, 240, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(125, 182);
  tft.print("Press red stop");
  tft.setCursor(115, 202); 
  tft.print("button to cancel");
  tft.setCursor(190, 73);
  tft.print("Time Left");
  tft.drawLine(190 , 95, 300, 95, ILI9341_WHITE);
  tft.setCursor(190, 103);
  tft.print("   :  :  ");

  do{   
    debouncer2.update();
    int value2 = debouncer2.read();
    
    if (value2 == LOW) {
      analogWrite(pwmToMotor, 0);
      delay(1500);
      digitalWrite(5, LOW);
      drawHomeScreen();
      return;
    }    
  
  nowTime = millis();
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);



  // HOURS
  if ((finishTime - nowTime) / 3600000 > 99) {

      tft.setCursor(190, 103);
      tft.print((finishTime - nowTime) / 3600000);
    }  else if ((finishTime - nowTime) / 3600000 > 9) {

      tft.setCursor(190, 103);
      tft.print("0");
      tft.print((finishTime - nowTime) / 3600000);
    }  else {

      tft.setCursor(190, 103);
      tft.print("00");
      tft.print((finishTime - nowTime) / 3600000);
    }



  // MINUTES
  tft.setCursor(238, 103);
    
    if (((finishTime - nowTime) % 3600000) / 60000 > 9) {
      tft.print(((finishTime - nowTime) % 3600000) / 60000);
    } else {
      tft.print("0");
      tft.print(((finishTime - nowTime) % 3600000) / 60000);
    }

  // SECONDS

 tft.setCursor(272, 103);
   secondsTimer = ((finishTime - nowTime) % 3600000 / 1000);
   if (secondsTimer > 59) {
    secondsTimer = secondsTimer % 60; 
   }
   if (secondsTimer < 10) {
    tft.print(" ");
    tft.print(secondsTimer);
   } else {
   tft.print(secondsTimer);
   }
  } while (nowTime < finishTime); 
  

  analogWrite(pwmToMotor, 0);
  delay(1500);
  digitalWrite(5, LOW);
  settings.dutyCycle = 0;
  rpm = 0;
  drawHomeScreen();  
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR DRAWING SELECTION BUTTONS
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void draw10Buttons () {
  tft.drawRoundRect(14, 45, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(31, 60);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("1");
  tft.drawRoundRect(74, 45, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(90, 60);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("2");
  tft.drawRoundRect(134, 45, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(150, 60);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("3");
  tft.drawRoundRect(194, 45, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(210, 60);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("4");
  tft.drawRoundRect(254, 45, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(270, 60);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("5");
  tft.drawRoundRect(14, 105, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(31, 120);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("6");
  tft.drawRoundRect(74, 105, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(90, 120);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("7");
  tft.drawRoundRect(134, 105, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(150, 120);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("8");
  tft.drawRoundRect(194, 105, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(210, 120);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("9");
  tft.drawRoundRect(254, 105, 50, 50, 5, ILI9341_ORANGE);
  tft.setCursor(260, 120);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("10");

}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR GETTING EVERYTHING FROM EEPROM
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void eepromGetAll(int profNumber) {
  int address = ((profNumber - 1) * 100);

  memset(settings.profileName, 0, sizeof(settings.profileName));
   
  EEPROM.get(address + 35, settings.profileLength);
      
  for (int i = 0; i < settings.profileLength; i++) {
    EEPROM.get(address + i, settings.profileName[i]);
  }  
  EEPROM.get(address + 40, settings.dutyCycle);
  EEPROM.get(address + 50, settings.runTimeInMs);  
  
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR GETTING NAME ONLY FROM EEPROM
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void eepromGetName(int profNumber) {
  int address = ((profNumber - 1) * 100);
  EEPROM.get(address + 35, settings.profileLength);
      
  for (int i = 0; i < settings.profileLength; i++) {
    EEPROM.get(address + i, settings.profileName[i]);
  }   
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR GETTING STATS ONLY FROM EEPROM
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void eepromGetStats(int profNumber) {
  int address = ((profNumber - 1) * 100);
  EEPROM.get(address + 40, settings.dutyCycle);
  EEPROM.get(address + 50, settings.runTimeInMs);  
  
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                     DRAW PROFILE INFO BLOCK
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawProfileInfoBlock(int infoType) {
  
  //Always the same no matter what Info block is requested.
  int hours, mins;
  hours = (settings.runTimeInMs / 3600000);
  mins = ((settings.runTimeInMs % 3600000) / 60000);
  rpm = map(settings.dutyCycle, 0, 254, 0, 3000);

  String profileString = settings.profileName;
  int nameLength = profileString.length();
    

  //Start Customization

  if (infoType == 1) {   
                                  // Full Info Block
  tft.fillRect(0, 0, 320, 179, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  if (nameLength == 0) {
    
    tft.setCursor(85, 22);
    tft.print("Profile:  ");
    tft.print(currentProfile);
    
    
    
  } else {

    
    tft.setCursor((300/2) - ((nameLength * 10)) / 2, 22);
    tft.print(settings.profileName);

    }

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.drawLine(20 , 45, 300, 45, ILI9341_WHITE);
    tft.setCursor(20, 73);
    tft.print("RPM:");
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.setCursor(20, 103);
    tft.print("Hours: ");
    tft.setCursor(95, 103);
    tft.print(hours);
    tft.setCursor(20, 133);
    tft.print("Mins: ");
    tft.setCursor(95, 133);
    tft.print(mins);
    tft.setCursor(190, 73);
    tft.print("Time Left");
    tft.drawLine(190 , 95, 300, 95, ILI9341_WHITE);
    tft.setCursor(190, 103);
    tft.print("000:00:00");

    if (hours > 99) {

      tft.setCursor(190, 103);
      tft.print(hours);
    }  else if (hours > 9) {

      tft.setCursor(190, 103);
      tft.print("0");
      tft.print(hours);
    }  else {

      tft.setCursor(190, 103);
      tft.print("00");
      tft.print(hours);
    }

    tft.setCursor(238, 103);
    
    if (mins > 9) {
        tft.print(mins);
      } else {
        tft.print("0");
        tft.print(mins);
    }
  
  }

  if (infoType == 2) {                                   // No Time Remaining Block
  tft.fillRect(0, 0, 320, 179, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  if (nameLength == 0) {
    
    tft.setCursor(85, 22);
    tft.print("Profile:  ");
    tft.print(currentProfile);
    
    
    
  } else {

    
    tft.setCursor((300/2) - ((nameLength * 10)) / 2, 22);
    tft.print(settings.profileName);

  }
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.drawLine(20 , 45, 300, 45, ILI9341_WHITE);
    tft.setCursor(20, 73);
    tft.print("RPM:");
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.setCursor(20, 103);
    tft.print("Hours: ");
    tft.setCursor(95, 103);
    tft.print(hours);
    tft.setCursor(20, 133);
    tft.print("Mins: ");
    tft.setCursor(95, 133);
    tft.print(mins);
    tft.setTextColor(0x2945, 0x0000);
    tft.setCursor(190, 73);
    tft.print("Time Left");
    tft.drawLine(190 , 95, 300, 95, 0x2945);
    tft.setCursor(190, 103);
    tft.print("000:00:00");

    if (hours > 99) {

      tft.setCursor(190, 103);
      tft.print(hours);
    }  else if (hours > 9) {

      tft.setCursor(190, 103);
      tft.print("0");
      tft.print(hours);
    }  else {

      tft.setCursor(190, 103);
      tft.print("00");
      tft.print(hours);
    }

    tft.setCursor(238, 103);
    
    if (mins > 9) {
      tft.print(mins);
    } else {
      tft.print("0");
      tft.print(mins);
    }

  }

    if (infoType == 3) {
    tft.fillRect(0, 0, 320, 179, ILI9341_BLACK);                                   // RPM Only Block
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.drawLine(20 , 45, 300, 45, ILI9341_WHITE);
    tft.setCursor(20, 73);
    tft.print("RPM:");
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.setTextColor(0x2945, 0x0000);
    tft.setCursor(20, 103);
    tft.print("Hours: ");
    tft.setCursor(95, 103);
    tft.print(hours);
    tft.setCursor(20, 133);
    tft.print("Mins: ");
    tft.setCursor(95, 133);
    tft.print(mins);
    tft.setCursor(190, 73);
    tft.print("Time Left");
    tft.drawLine(190 , 95, 300, 95, 0x2945);
    tft.setCursor(190, 103);
    tft.print("Hours: ");
    tft.setCursor(265, 103);
    tft.print(hours);
    tft.setCursor(190, 133);
    tft.print("Mins: ");
    tft.setCursor(265, 133);
    tft.print(mins);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }

    if (infoType == 4) {                                   // No Time Remaining Block for Timer
    tft.fillRect(0, 0, 320, 179, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.drawLine(20 , 45, 300, 45, ILI9341_WHITE);
    tft.setCursor(20, 73);
    tft.print("RPM:");
    tft.setCursor(20, 103);
    tft.print("Hours: ");
    tft.setCursor(20, 133);
    tft.print("Mins: ");
    tft.setTextColor(0x2945, 0x0000);
    tft.setCursor(190, 73);
    tft.print("Time Left");
    tft.drawLine(190 , 95, 300, 95, 0x2945);
    tft.setCursor(190, 103);
    tft.print("000:00:00");

  }
  
}

void centerText(String textValue, byte textSize, int yPos) {

  int nameLength = textValue.length();
  tft.setTextSize(textSize);
  tft.setCursor(160 - ((nameLength * (textSize * 6))) / 2, yPos);
  tft.print(textValue);

}