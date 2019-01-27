#include <EEPROM.h>           // For manipulating EEPROM 
#include <Bounce2.h>          // Debounce Library 
#include <ClickEncoder.h>     // Encoder Library 
#include <TimerOne.h>         //Timer One Library for debouncer 
#include <Adafruit_GFX.h>     //Core graphics library
#include <SPI.h>              //This is needed for display
#include <Wire.h>             //This is needed for FT6206
#include <Adafruit_FT6206.h>  //Touch screen library
#include <Adafruit_ILI9341.h> //Display Library

#define WITH_LCD 1
#define BUTTON_PIN_1 2        //Start button pin 
#define BUTTON_PIN_2 3        //Stop button pin 
#define StepsPerNotch (1)     //Number of steps per notch for your encoder 

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//Initialize variables for encoder library
ClickEncoder *encoder;
int16_t last = -1, value;

int pwmToMotor = 6;           //Set PWM pin for motor
const int motorEnable = 5;    //Set Motor Enable Pin

//Various variables
int profileAddress = 0, rpm = 0, tempRpm = 0, tempTime = 0, timerValueHours = 0, timerTimerValueHours = 0, cursorPosition = 10, encoderValue = 0, tempHours = 0;
byte currentPage = 0, numberOfSelections = 0, selectionValue = 0, timerValueMinutes = 0, timerTimerValueMinutes = 0, arrayNum = 0, currentProfile = 0, charNum = 0, timerDutyCycle = 0, tempMins = 0;
unsigned long timeRemainingMs = 0, timeRemainingMins = 0, previousMillis = 0, timerTimeRemainingMins = 0, timerTimeRemainingMs = 0;
boolean isFirstHours = true, isFirstMins = true, isStartPressed = false, isStopPressed = false;
char tempProfileName[16];

//Array of characters for profile names
char Str2[50] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-', '+', '=', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};

//Profile Structure
typedef struct {
  unsigned long runTimeInMs;
  int dutyCycle;
  char profileName[16];
  int profileLength;
} settingData;

//Initialize structure to banks on boot
settingData settings = {
  0,
  0,
  ' ',
  0
};

//Settings for debouncer librbary
void timerIsr() {
  encoder->service();
}
Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce(); 


//****************************************************************************************************************************************************************************************
//                                                                                              SETUP BEGIN
//****************************************************************************************************************************************************************************************


void setup() {
  
  pinMode(pwmToMotor, OUTPUT);          //Set PWM Pin as an Output
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);  //Set Red Button Pin as an Input
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);  //Set Green Button Pin as an Input
  pinMode(motorEnable, OUTPUT);         //Set Motor Enable Pin as an Output

  
  //Button debounce settings 
  debouncer1.attach(BUTTON_PIN_1);
  debouncer1.interval(50); //milliseconds of debounce
  debouncer2.attach(BUTTON_PIN_2);
  debouncer2.interval(50); //milliseconds of debounce
  
  //Encoder pins A0, A1 and A2
  encoder = new ClickEncoder(A1, A0, A2);
 
  //Start the touchscreen  and set the touch sensitivity
  tft.begin();
  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    while (1);
  }

  //TimerOne timer stuff (defaults)
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 

  tft.setRotation(1); //Set the screen rotation to landscape
  drawHomeScreen(); //Draw the home screen
  
}

//****************************************************************************************************************************************************************************************
//                                                                                               SETUP END
//****************************************************************************************************************************************************************************************




//****************************************************************************************************************************************************************************************
//                                                                                              LOOP BEGIN
//****************************************************************************************************************************************************************************************

void loop() {

  //Check for button presses
  debouncer1.update();
  debouncer2.update();

  int value1 = debouncer1.read();
  int value2 = debouncer2.read();

  if (value1 == LOW) {            //If button 1 is pressed during the main code loop
    digitalWrite(5, HIGH);        //Set the output pin to the motor high, enabling the motor to run at whatever PWM is set on pin pwmToMotor
  }

  if (value2 == LOW) {            //If button 2 is pressed during the main code loop
    analogWrite(pwmToMotor, 0);   //Set motor speed to zero
    delay(1500);                  //Wait x milliseconds for motor to spin down lest it stop immediately
    digitalWrite(5, LOW);         //Disable the motor
  } 



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                       PAGE 1 (HOME SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------                                                                                            
  
// If current page is Home Screen 
  
  if (currentPage == 1){          //If current page is the Home Screen
  
    //Touchscreen code from example files.  Will be seen a lot.  It just registers a touch in the rectangle between point x1,y1 and x2,y2 and does something.  
    //"If touched find out where, if it's in this range do x, if it's in this rage do y, ...

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
        drawTimerScreen();  //If touch point lands in area defined above, draw the Timer screen
      }
      }
  
      // Check for touch on top button and send to Auto screen  
      if((x > 30) && (x < 289)) {  //set the touch area bounds Xmin and Xmax
      if ((y > 130) && (y < 170)) { //set the touch area bounds Ymin and Ymax
        drawAutoScreen();  //If touch point lands in area defined above, draw the Automatic screen
      }
      }
  
      // Check for touch on top button and send to Profile screen  
      if((x > 30) && (x < 289)) {  //set the touch area bounds Xmin and Xmax
      if ((y > 180) && (y < 220)) { //set the touch area bounds Ymin and Ymax
        drawProfileChooser();  //If touch point lands in area defined above, draw the Profiles screen
      }
      }
    }
  }



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        PAGE 2 (MANUAL) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// If current page is Manual Screen 
  if (currentPage == 2){

    encoder->setAccelerationEnabled(true);  //Enable or disable encoder accelleration
    value += encoder->getValue();           //Get the value from the encoder routine and add it to the RPM value, 'value'
  
    if (value != last) {                    //If it's different then it was 'last' time the loop was run, change the 'last' variable to the new value
      last = value;
    } 
     
    if (value < 0) {                        //Keep the motor RPM from going below zero
      value = 0;
    }
    
    if (value > 3000) {                     //Keep the motor RPM from going above 3000
      value = 3000;
    }
  
    settings.dutyCycle = map(value, 0, 3000, 0, 254);   //Map the RPM 'value' variable to 'dutyCycle' which next is output to the motor on the motor output pin.  The motor sees 0 as zero RPM and 254 as maximum RPM (e.g. 3,000)
    analogWrite(pwmToMotor, settings.dutyCycle);        //Write 'dutyCycle' to the motor output pin
    rpm = map(settings.dutyCycle, 0, 254, 0, 3000);     //Get a human readable RPM value by mapping the 'dutyCycle' back to RPM.  Since the motor can't determine more than 256 "speeds" from the analog output, this should set the RPM displayed to a more accurate value


    tft.setTextSize(2);                                 //Will be seeing this a lot due to how the touchscreen library works.  Set a text size then
    tft.setCursor(95, 73);                              //Set at what pixel to start drawing then
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK);       //Set text color (foreground, background)
    tft.print(rpm);                                     //Print the human readable RPM value
    tft.print("   ");                                   //Print some spaces after to cover up any leftover characters going from XXXX to XXX or XXX to XX, etc.  They don't get erased from the screen after they are written unless you write over them with something - in this case, blank spaces.
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);     //Set text color again to the "default" white on black so the next time something is written it's not written in red



    if (digitalRead(5) == HIGH) {                               //If the start button is pressed
      tft.setCursor(25, 190);                             
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setTextSize(3);
      tft.print("Wait");
      if (isStartPressed == false) {                            //Check to see if this proceeding code has already been written to the screen.  If not, each time the program loops through it will erase and re-write the text causing flicker
        tft.fillRect(105, 180, 320, 240, ILI9341_BLACK);        //Draw a rectabgle filled with black pixels to cover up the text that was here before so it can be replaced
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
        tft.setCursor(125, 182);
        tft.print("Press red stop");                            //with this text
        tft.setCursor(115, 202); 
        tft.print("button to cancel");
      }
      isStartPressed = true;                                    //Set the variable for the start button press to true so the preceeding section isn't written the next time through the loop
      isStopPressed = false;                                    //Set the variable for the stop button press to false so it is reset for when you want to hit the stop button
    }
    
    if (digitalRead(5) == LOW) {                                //Same as above but for the stop button

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
    
    if (ctp.touched()){                                          //Check to see if someone touched the area where the "back" button is and of so, draw the home screen and start the loop over again
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
//                                                                                       PAGE 3 (TIMER SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Timer Screen
   
  if (currentPage == 3){

  if (selectionValue == 1) {                                  //First text field, "RPM"
                                                              //selectionValue is used to keep track of which text field you are on.  Pressing the knob increments this value by one, moving to the next text section
    encoder->setAccelerationEnabled(true);                    //Set RPM again, this time for setting up a timed run
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
  
  if (selectionValue == 2) {                                //Second text field "Hours"
    
    encoder->setAccelerationEnabled(false);
    tempHours += (encoder->getValue());                     //Set the conder value to a temporary value, 'tempHours'
    timerTimerValueHours = tempHours * 0.55;                //Excuse the redundancy in timerTimerValueHours but I already used timerValueHours before I got here...
    
    
    if (timerTimerValueHours != last) {                     //If the value is different, change it
      last = timerTimerValueHours;
    }  
    
    if (timerTimerValueHours < 0) {                         //If it's less than zero, set it back to zero.  There is no negative time here.
      timerTimerValueHours = 0;
    }
    
    if (timerTimerValueHours > 999) {                       //Optional I guess but I decided to cap the maximum hours to 999 just for simplicity.  I very much doubt this will need to run much more than 24-48 hours
      timerTimerValueHours = 999;
    }

  }
  
  if (selectionValue == 3) {                                //Third field "minutes"
     
    encoder->setAccelerationEnabled(false);
    tempMins += (encoder->getValue());
    timerTimerValueMinutes = tempMins * 0.5;
  
    if (timerTimerValueMinutes != last) {                  //Same as before but with minutes capped at 59 caus then you'd just use an hour
      last = timerTimerValueMinutes;
    }  
    
    if (timerTimerValueMinutes < 0) {
      timerTimerValueMinutes = 0;
    }
    
    if (timerTimerValueMinutes > 59) {
      timerTimerValueMinutes = 59;
    }
  }
  
  timerTimeRemainingMs = (timerTimerValueHours * 3600000) + (timerTimerValueMinutes * 60000);       //Get the total number of milliseconds to run by multiplying the hours by 3.6 million and the minutes by 60,000 and add them together and store that in a variable for later
  //timerTimeRemainingMins = (timerTimeRemainingMs / 60000);                          
  
  if (selectionValue == 1) {                              //If the selection value is on 1, meaning "RPM"

    // tft.setTextColor(ILI9341_RED, ILI9341_BLACK);      //set the RPM text red and write the RPM value
    tft.setTextSize(2);
    tft.setCursor(95, 73);
    tft.print(rpm);
    tft.print("   "); 
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);       //Set the other text back to white if they were red before
    tft.setCursor(95, 103);
    tft.print(timerTimerValueHours);
    tft.print("   ");
    tft.setCursor(95, 133);
    tft.print(timerTimerValueMinutes);
    tft.print("   ");    

  }

  if (selectionValue == 2) {                              //repeat for hours

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

  if (selectionValue == 3) {                              //repeat for minutes

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

  ClickEncoder::Button b = encoder->getButton();          //This is looking out for encoder wheel presses and if it sees one it adds one to the selectionValue variable moving to the next selection on next loop
 
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:
      selectionValue++;
      if (selectionValue > numberOfSelections) {          //When this page is first drawn by calling drawTimerScreen() we set the 'numberOfSelections' for this page so that when the encoder wheel is pressed after the third time, it loops back and starts with selection 1 again
        selectionValue = 1;
      }
      
      break;
      
    } 
  }
          
  debouncer1.update();                                    //Check for start button presses
  int value1 = debouncer1.read();
  
  if (value1 == LOW) {                                    //If the start button is pressed, start the timerLoop() function with the value for the number of milliseconds to run and the dutyCycle (RPM) to write to the output pin
    //selectionValue = 1;
    timerLoop(timerTimeRemainingMs, timerDutyCycle);
  }

  if (ctp.touched()){                                     //If the back button is pressed, set the selectionValue back to one and draw the home screen
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
//                                                                                     PAGE 4 (AUTOMATIC SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  

// If current page is Auto Screen 

  if (currentPage == 4){                              //The entierty of this page is checking for where you press on the screen and passing that profile number to the drawRunScreen() function



  if (ctp.touched()){   
    TS_Point p = ctp.getPoint();                      //If you press "back"
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {
    if ((y > 180) && (y < 220)) {
    
      drawHomeScreen();
    
    }
    }
    
    if ((x > 14) && (x < 64)){                        //If you press profile 1
    if ((y > 64) && (y < 114)) {
    
      drawRunScreen(1);
    
    }
    }
    
    if ((x > 74) && (x < 124)){                       //etc...
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
//                                                                                   PAGE 5 (PROFILE CHOOSER SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// If current page is Profile Chooser Screen 

  if (currentPage == 5){                              //Same as Suto screen, just checking for screen presses and calling the function to draw the profile edit screen with the profile you touched
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
//                                                                                      PAGE 6 (PROFILE VIEW) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Profile View Screen 
  

  if (currentPage == 6){                //Check and see if you press "back" and go back to the profile chooser page
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
  
    
    if((x > 220) && (x < 300)) {        //or if you press "edit" go to the editor page
    if ((y > 180) && (y < 220)) {

      drawEditScreen();
  
    }
    }
  }
 }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                      PAGE 8 (RENAME SCREEN) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Rename Screen 
 
  if (currentPage == 8){
  int numberOfCharacters = 15;                                //Define the maximum number of characters
  encoder->setAccelerationEnabled(false);                     //Set accelleration to false or it's nearly impossible to choose any specific letter
  encoderValue += (encoder->getValue());
  charNum = map(encoderValue, 0, 400, 0, 50);                 //Map the value of the encoder to a character from the array initialized at the start which contains all the available characters.  I used 400 values to give more 
                                                              //resolution to the encoder wheel.  Higher values take more turns to move to the next letter, lower values take fewer.
  if (encoderValue != last) {
    last = encoderValue;
  }  
  
  if (encoderValue > 400) {                                   //If it goes above 400 (the last character in the array) loop back to the begining, letter "A"
    encoderValue = 0;
  }
  
  if (encoderValue < 0) {                                     //Opposite of above
    encoderValue = 400;
  }
  
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(cursorPosition, 110);                         //We set cursorPosition when we wrote this screen to the location where we want the characters to start at on the screen, so set the text position there
  tft.print(Str2[charNum]);                                   //Print the character from the list of caracter array
  tempProfileName[arrayNum] = Str2[charNum];                  //Write that character to the 'tempProfileName' array (which we will use later) in the address 'arrayNum' which is set to zero when the screen first loads  
  
  ClickEncoder::Button b = encoder->getButton();              //Check for encoder knob presses to move to the next character
 
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Clicked:                             //If the button is clicked
      cursorPosition += 12;                                   //Add 12 to the cursor location which is the space of a character at this font size
      tft.setCursor(cursorPosition, 35);                      //Move the cursor to that next text position, leaving the character that was on the screen and written into the temporary array, there
      arrayNum++;                                             //Move to the next array address
      drawSaveButton();                                       //Draw the button allowing you to save the profile name, only after you've at least selected one character
      
        if (cursorPosition > (130 + numberOfCharacters + (numberOfCharacters * 11 - 1))) {    //If you click past the last character, loop back to the begining
          arrayNum = 1;
          cursorPosition = 130;
        }
      
      break;
    } 
  }

  if (b != ClickEncoder::Open) {                              //Same as above but this time a double click of the encoder knob
    switch (b) {
      case ClickEncoder::DoubleClicked:
      cursorPosition -= 12;                                   //Move back one on the screen and one in the array
      arrayNum--;
      tft.setCursor(cursorPosition, 35);
        if (cursorPosition < 130) {                           //But don't loop back to the end of the array - I found that behavior... odd.
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
    if((x > 20) && (x < 100)) {                               //When the "back" button area of the screen is pressed
    if ((y > 180) && (y < 220)) {

      memset(tempProfileName, 0, sizeof(tempProfileName));    //Erase anything in the tempProfileString to prepare it for use later
      drawProfileEditor(currentProfile);
  
    }
    }
    
  if ((x > 120) && (x < 200)){                                      //When the "save" button area of the screen is pressed
  if ((y > 180) && (y < 220)){
  
    memset(settings.profileName, 0, sizeof(settings.profileName));  //Erase what is in the 'settings' structure, settings.profileName - the name of the currently selected profile
  
  for (int i = 0; i < sizeof(tempProfileName); i++) {               //Loop through the temporary array that was filled with characters from the user re-write them into the settings.profileName
    settings.profileName[i] = tempProfileName[i];
  }
  
  int j = 0;                                                        //declare a temp variable; I don't know why 'j'
  
  while (settings.profileName[j] != '\0') {                         //Preparing for writing to EEPROM - loop through the settings.profileName array and look for the first blank character which signifies the end of the name
    j++;                                                            //and increment 'j' by one
  }
    
  int address = ((currentProfile - 1) * 100);                       //Setup the address location in the EEPROM we want to write to.  1 becomes 0, 2 becomes 100, etc.  0-99 is for profile 1, 100-199 for profile 2, etc.  0-35 (or 100-135, etc) are reserved for the profile name
   
  for (int i = 0; i < j; i++) {
    EEPROM.put(address + i, settings.profileName[i]);               //Standard for loop - for the number of characters counted 'j' above put the settings.profileName address of 'i' into EEPROM at the address 'j' (0,1,2 or 100,101,102, etc)
  }
  
    EEPROM.put(address + 35, j);                                    //When done, put the length of the name 'j' into address + 35 (35, 135, 235, et) so we can know how lonf the name is when we go looking for it later.  This keeps it from reading characters from a longer name that used that same starting address
                                                                    //I tried to write blanks into EEPROM to overwrite the old name but got unpredictable results.  Plus EEPROM has limited writes so that's not the best idea when we can just specify how many characters to get later
  
  memset(tempProfileName, 0, sizeof(tempProfileName));              //When done erase the temporary array for use next time
  drawProfileEditor(currentProfile);                                //And go back to the profile screen for the currently selected profile

  }
  }      
 }
    
 }

 //------------------------------------------------------------------------------------I guess I never mage a page 7----------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                      PAGE 9 (RUN PROFILE) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// If current page is Run Screen 
  
  if (currentPage == 9){

  debouncer1.update();                                      //Wait for a start button press
  int value1 = debouncer1.read();
  if (value1 == LOW) {
  timerLoop(settings.runTimeInMs, settings.dutyCycle);      //Then start the same timer loop used for the manual timer mode, but pass it the number of milliseconds to run from the currently selected profile structure, as well as the speed at which to run
  }

  if (ctp.touched()){                                       //Back button touch code, go back to the run profile chooser page
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
//                                                                                     PAGE 10 (EDIT PROFILE) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Edit Screen 
  
 
  if (currentPage == 10){
  if (ctp.touched()){   
    TS_Point p = ctp.getPoint(); 
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;  
    if((x > 20) && (x < 100)) {                   //Check for a button press on "back" and go back to the profile page
    if ((y > 180) && (y < 220)) {

      drawProfileEditor(currentProfile);
  
    }
    }


     if ((x > 120) && (x < 200)){                 //Check for a button press on "name" to go to the rename page
     if ((y > 180) && (y < 220)){

      
        drawRenameScreen(currentProfile);    
      
    
    }
    }   
    
    if((x > 220) && (x < 300)) {                  //Check for a button press on "spec" to go to the spec settings page
    if ((y > 180) && (y < 220)) {

      
      drawSpecScreen(currentProfile);
       
    }
    }
  }
 }

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                      PAGE 11 (EDIT SPEC) CODE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// If current page is Edit Specs Screen 
  
 
  if (currentPage == 11){

  if (selectionValue == 1) {                                      //Same as in the other pages with multiple selections, check what selection you are currently on, watch the encoder for turns and adjust the variable value associated with that selection
    
    encoder->setAccelerationEnabled(true);
    tempRpm += encoder->getValue();
    
    if (tempRpm != last) {                                        //RPM is written to tempRPM in case you don't save your changes it won't have overwritten 'settings.dutyCycle'
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
  
  if (selectionValue == 2) {                                      //Same thing with hours

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
  
  if (selectionValue == 3) {                                      //and minutes
    
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
  
  timeRemainingMs = (timerValueHours * 3600000) + (timerValueMinutes * 60000);  //Do the millisecond addition math again

  if (selectionValue == 1) {                                    //Write the selected text area in red and the others in white, same as on the manual timer page

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



  ClickEncoder::Button b = encoder->getButton();               //Again, check if you click past the number of selections on the page, loop back to the first selection
 
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
    if((x > 20) && (x < 100)) {                                //If back button is pressed
    if ((y > 180) && (y < 220)) {

      drawProfileEditor(currentProfile);
  
    }
    }
  

     if ((x > 120) && (x < 200)){                               //If save button is pressed
     if ((y > 180) && (y < 220)){
            
      int address = ((currentProfile - 1) * 100);               //find out the correct address in EEPROM for these settings as before in the 'rename' page
      
      EEPROM.put(address + 40, settings.dutyCycle);             //At address + 40 (40 for profile 1, 140 for profile 2, etc) store the dutyCycle of the PWM for the motor
      EEPROM.put(address + 50, timeRemainingMs);                //At address + 50 (50 for profile 1, 150 for profile 2, etc) store the number of milliseconds for which to run
      drawProfileEditor(currentProfile);
    
    }
    }      
  }
 }

}

//****************************************************************************************************************************************************************************************
//                                                                                             LOOP END
//****************************************************************************************************************************************************************************************



//****************************************************************************************************************************************************************************************
//*     
//*
//*          Most of the following sections are mostly the same idea.  First, bank the screen by filling it with black to erase anything from previous pages.  Then draw any
//*        text that you want to display on the page you are creating.  Call any button creation functions to create any button images for the page.  It's just arranging the page
//*       the way you want.  Anything written in the preceeding sections (the "pages") are looped though.  So any text or button calls, etc will be redrawn every time the code
//*       loops which often causes flicker.  So anything that won't be changed or if you want to display starting vlues for something that doesn't have a value assigned, do it here.
//*        finally, set currentPage == the page number you want to loop through.  Everything in the main loop is broken into "if page = x" sections so only that section is run
//*       Also, set up any boolean variables for things like knowing if some operation has happened or not.  Set them to the defaults now so you get expected behavior.
//****************************************************************************************************************************************************************************************

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                          DRAW HOME SCREEN (page 1)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawHomeScreen() {

  tft.fillScreen(ILI9341_BLACK);                              
  tft.setTextSize(2);                                         
  tft.setCursor(110, 10);
  tft.print("Welcome!");
  tft.setTextSize(3);
  tft.setCursor(60, 39);
  tft.print("Manual Mode");                                   //Drawing text for the buttons
  tft.drawRoundRect(30, 30, 260, 40, 5, ILI9341_ORANGE);      //and outlining them in an orange rounded rectangle
  tft.setCursor(68, 89);
  tft.print("Timer Mode");
  tft.drawRoundRect(30, 80, 260, 40, 5, ILI9341_ORANGE);
  tft.setCursor(35, 139);
  tft.print("Automatic Mode");
  tft.drawRoundRect(30, 130, 260, 40, 5, ILI9341_ORANGE);
  tft.setCursor(90, 189);
  tft.print("Profiles");
  tft.drawRoundRect(30, 180, 260, 40, 5, ILI9341_ORANGE);  
  currentPage = 1;                                            //set so the main loop knows to loop though page 1 only
 
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         DRAW MANUAL SCREEN (page 2)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawManualScreen() {
  digitalWrite(5, LOW);                                       //Disable the motor, just in case
  tft.fillScreen(ILI9341_BLACK);
  drawBackButton();
  drawProfileInfoBlock(3);
  centerText("Manual Mode", 2, 12);                           //Call my special "center text" function.  It takes the text you want to write, the size you want it to be and the Y coordinate down the screen at which you want to write
  centerText("Use knob to set RPM", 1, 30); 
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(107, 182);
  tft.print("Set RPM and press");
  tft.setCursor(135, 202); 
  tft.print("start button");
  isStopPressed = false;
  isStartPressed = false;            
  currentPage = 2;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                          DRAW TIMER SCREEN (page 3)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawTimerScreen() {
  digitalWrite(5, LOW);  

  tft.fillScreen(ILI9341_BLACK);
  drawProfileInfoBlock(4);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  centerText("Timer Mode", 2, 12);
  centerText("Press knob to change selection", 1, 30);
  tempRpm = 0;                                                //Set variuous variables needed for the looping code on the page.  You can't set them in the loop or they will be overwritten on each loop
  tempTime = 0;
  timerDutyCycle = 0;
  //timerTimeRemainingMins = 0;
  timerTimerValueHours = 0;
  timerTimerValueMinutes = 0;
  timerTimeRemainingMs = 0; 
  numberOfSelections = 3;
  tempHours = 0;
  tempMins = 0;
  drawBackButton();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(104, 182);
  tft.print("Press start button");
  tft.setCursor(134, 202); 
  tft.print("to run timer");
  selectionValue = 1;                                         //Set selectionValue so no matter where it left off, it's back to the first position when you open the page  
  currentPage = 3;

}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW AUTOMATIC SCREEN (page 4)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawAutoScreen() {

  digitalWrite(5, LOW);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(110, 10);
  centerText("Automatic Mode", 2, 12);
  draw10Buttons();                                            //Call the function to draw the 10 profile selection buttons
  drawBackButton();
  currentPage = 4;
  

}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                      DRAW PROFILE CHOOSER SCREEN (page 5)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawProfileChooser() {

  digitalWrite(5, LOW);
  arrayNum = 0;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(60, 10);
  centerText("Profile Manager", 2, 12);
  draw10Buttons();                                            //Call the function to draw the 10 profile selection buttons
  drawBackButton();
  currentPage = 5;
  

}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         DRAW PROFILE EDITOR (page 6)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawProfileEditor(int profNumber) {                      //This function and several others take a 'profNumber' which is just the profile number you wish to use.
  
  currentProfile = profNumber; 
  eepromGetAll(profNumber);
  tft.fillScreen(ILI9341_BLACK);
  drawProfileInfoBlock(2);                                    //This function the profile information on the screen and takes a number designating which 'variant' of that profile block you want written
  drawEditButton();
  drawBackButton();
  currentPage = 6;
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                        DRAW RENAME SCREEN (page 8)
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
  memset(tempProfileName, 0, sizeof(tempProfileName));          //Clear the tempProfileName just in case it wasn't cleared before, somehow
  currentProfile = profNumber;                                  //Set the current profile variable to the passed profile number value
  selectionValue = 1;
  currentPage = 8;
  

}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                          DRAW RUN SCREEN (page 9)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void drawRunScreen(int profNumber) {
  
  currentProfile = profNumber;
  tft.fillScreen(ILI9341_BLACK);
  eepromGetAll(profNumber);                             //Call the function to ger all the profile info from EEPROM
  drawProfileInfoBlock(1);
  arrayNum = 0;                                         //Set the value for the temporary array address to zero in case it's not already
  drawBackButton();
  currentPage = 9;
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(104, 182);
  tft.print("Press start button");
  tft.setCursor(125, 202); 
  tft.print("to run profile");
  

}




//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                      DRAW EDIT SELECTION SCREEN (page 10)
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawEditScreen() {

  drawProfileInfoBlock(2);
  drawBackButton();
  drawNameButton();
  drawSpecButton();
  currentPage = 10;
  

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                         DRAW SPEC SCREEN (page 11)
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
  isFirstHours = true;
  isFirstMins = true;
  drawProfileInfoBlock(2);
  drawBackButton();
  drawSaveButton();
  currentPage = 11;

}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            DRAW BACK BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawBackButton () {
  tft.drawRoundRect(20, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(25, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Back");
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            DRAW SAVE BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawSaveButton () {
  tft.drawRoundRect(120, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(125, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Save");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                            DRAW RUN BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawRunButton () {
  tft.drawRoundRect(120, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(130, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Run");
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                           DRAW EDIT BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawEditButton () {
  tft.drawRoundRect(220, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(225, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Edit");
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                           DRAW NAME BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawNameButton () {
  tft.drawRoundRect(120, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(125, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Name");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                           DRAW SPEC BUTTON
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawSpecButton () {
  tft.drawRoundRect(220, 180, 80, 40, 5, ILI9341_ORANGE);
  tft.setCursor(225, 190);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print("Spec");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                              CODE FOR DRAWING 10 PROFILE SELECTION BUTTONS
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void draw10Buttons () {
  tft.drawRoundRect(14, 45, 50, 50, 5, ILI9341_ORANGE);  //Bet this could be done in a fancy loop?
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
//                                                                                          CODE FOR TIMER LOOP
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void timerLoop (unsigned long timeInMs, int duty) {

                                                            
  unsigned long startTime = millis();                      
  unsigned long finishTime = startTime + timeInMs;          //Take the milisecond value from the moment this function was called and add to it the numbner of milliseconds you were asked to run.  This is the value of milliseconds at which to stop running.
  unsigned long nowTime; 
  int secondsTimer = 0;


  digitalWrite(5, HIGH);                                    //Enable the motor
  analogWrite(pwmToMotor, duty);                            //Set the motor speed on the output pin
  tft.setCursor(25, 190);                                   //Just various display text
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

  do{                                                           //Loop through all this   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    debouncer2.update();
    int value2 = debouncer2.read();
    
    if (value2 == LOW) {                                        //Check and see if someone pressed the stop button
      analogWrite(pwmToMotor, 0);                               //Set the motor speed to zero
      delay(1500);                                              //Wait for the motor to spin down
      digitalWrite(5, LOW);                                     //Turn off the motor enable output
      drawHomeScreen();                                         //Draw the home screen and exit the loop
      return;
    }    
  
  nowTime = millis();                                           //Otherwise, get the time it is now (in milliseonds since booting up) from the millis() function and that's the time it is now, during this trip through this do-while loop
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);



  // HOURS
  if ((finishTime - nowTime) / 3600000 > 99) {                  //Displaying the hours left to run by subtracting the time we are to finish from the time we just determined it is above (all in milliseconds) and dividing by 3.6M which is the number of ms in an hour

      tft.setCursor(190, 103);    
      tft.print((finishTime - nowTime) / 3600000);              //These three sections just see if the hour value is one, two or three digits long and depending, shift the text around on the screen to cover any previous digits that were displayed.  Annoying.
    }  else if ((finishTime - nowTime) / 3600000 > 9) {

      tft.setCursor(190, 103);
      tft.print("0");                                           //Zeros are to cover any left over digits e.g. when 10 -> 9.  The 1 would stay on the screen and then it would read 19 instead of 9 -or 09 as I've chosen to write.  An empty space could be used instead for a blank.
      tft.print((finishTime - nowTime) / 3600000);
    }  else {

      tft.setCursor(190, 103);
      tft.print("00");
      tft.print((finishTime - nowTime) / 3600000);
    }



  // MINUTES
  tft.setCursor(238, 103);
    
    if (((finishTime - nowTime) % 3600000) / 60000 > 9) {       //Same as for hours but with minutes this time, and using modulo to get what is left over from doing the hours calculation.  This remainder is everything left over after the hours are taken out. Neat!
      tft.print(((finishTime - nowTime) % 3600000) / 60000);    //The remainder of the modulo calculation is in milliseconds so to get the minutes we divide by 60,000 instead of 3.6M - 60,000 milliseconds in a minute.
    } else {
      tft.print("0");                                           //Zeros are to cover any left over digits e.g. when 10 -> 9.  The 1 would stay on the screen and then it would read 19 instead of 9 -or 09 as I've chosen to write.  An empty space could be used instead for a blank.
      tft.print(((finishTime - nowTime) % 3600000) / 60000);
    }

  // SECONDS

 tft.setCursor(272, 103);
   secondsTimer = ((finishTime - nowTime) % 3600000 / 1000);    //And finally the seconds value which is, again, the modulo remainder of doing the minutes calculation but only dividing by 1,000 this time since there are 1,000 miliseconds in a second
   if (secondsTimer > 59) {
    secondsTimer = secondsTimer % 60;                           //But this just gives us total number of milliseconds, it could be 24952 which is NOT 0-59 like you'd expect.  So we just modulo again to get rid of anything that is divisible by 60 (the minutes that are already being covered by the code above)
   }                                                            //and discard them.  We don't need to know those because the minutes are already displayed.  We are only interested in 0-59 so modulo throws out everything that isn't a multiple of 60 - that being the numbers 0-59.  Neat again!
   if (secondsTimer < 10) {
    tft.print(" ");
    tft.print(secondsTimer);
   } else {
   tft.print(secondsTimer);
   }
  } while (nowTime < finishTime);                               //While the time (in milliseconds) that it is currently is less than the time (in milliseconds) that was specified to be the end of the cycle ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  

  analogWrite(pwmToMotor, 0);                                   //After exiting the loop (time has elapsed), turn off the motor, let it spin down and go abck to the home screen, writing 0 to RPM so it's ready for the next use
  delay(1500);
  digitalWrite(5, LOW);
  settings.dutyCycle = 0;
  rpm = 0;
  drawHomeScreen();  
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR GETTING EVERYTHING FROM EEPROM
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void eepromGetAll(int profNumber) {                                   //Pass this function the profile number you are interested in
  int address = ((profNumber - 1) * 100);                             //Recalculate that profile numbner into an address 0, 100, 200, etc

  memset(settings.profileName, 0, sizeof(settings.profileName));      //Clear out the settings.profile array
   
  EEPROM.get(address + 35, settings.profileLength);                   //Get the length of the string 'j' that was stored along with the name
      
  for (int i = 0; i < settings.profileLength; i++) {                  //Loop through the addresses one by one until you get to the length of the string that is supposed to be there
    EEPROM.get(address + i, settings.profileName[i]);                 //and store that in the settings.profileName variable
  }  
  EEPROM.get(address + 40, settings.dutyCycle);                       //Also get the duty cycle and store it in settings.dutyCycle
  EEPROM.get(address + 50, settings.runTimeInMs);                     //Ansd the number of milliseconds and store in settings.runTimeInMs
  
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                    CODE FOR PRINTING PROFILE NAME
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void printProfileName (int profNumber) {
      
  int address = ((profNumber - 1) * 100);                               //Calculate the address as before, using the passed profile number

  //memset(settings.profileName, 0, sizeof(settings.profileName));       

  EEPROM.get(address + 35, settings.profileLength);                     //Get the length of the name from EEPROM address +35
  
  for (int i = 0; i < settings.profileLength; i++) {                    //Loop though and fill in the settings.profileLength with that value
    EEPROM.get(address + i, settings.profileName[i]);
  }
    tft.print(settings.profileName);                                    //Print the name at the location that was set before this function was called
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                CODE FOR GETTING STATS ONLY FROM EEPROM
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void eepromGetStats(int profNumber) {                               //Same as the get all function but just getting the RPM and time this time.  So no need to clear arrays or get name length
  int address = ((profNumber - 1) * 100);
  EEPROM.get(address + 40, settings.dutyCycle);
  EEPROM.get(address + 50, settings.runTimeInMs);  
  
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                                     DRAW PROFILE INFO BLOCK
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void drawProfileInfoBlock(int infoType) {                           //This draws out the profile info on the screen for whatever profile is currently active and takes an integer to select which "style" of profile you want
  
  //Always the same no matter what Info block is requested.
  int hours, mins;                                                  //Do the math to turn the stored settings.runTimeInMs into useable hours and minutes
  hours = (settings.runTimeInMs / 3600000);
  mins = ((settings.runTimeInMs % 3600000) / 60000);
  rpm = map(settings.dutyCycle, 0, 254, 0, 3000);                   //Get the RPM from mapping settings.dutyCyle to a value between 0-3000 - it's not perfect but it works

  String profileString = settings.profileName;                      //Take the character arrat settings.profileName and turn it into a string so we can
  int nameLength = profileString.length();                          //see how long that name is so we know for later
    

  //Start Customization

  if (infoType == 1) {                                   // All Info Block
                                 
  tft.fillRect(0, 0, 320, 179, ILI9341_BLACK);                      //Clear the area of the screen where this goes
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2); 
  if (nameLength == 0) {                                            //If the length of the name is zero that means it has no name, so just print Profile and the current profile number instead of a name
    
    tft.setCursor(85, 22);
    tft.print("Profile:  ");
    tft.print(currentProfile);
    
    
    
  } else {                                                          //Otherwise we print the name

    
      tft.setCursor((300/2) - ((nameLength * 10)) / 2, 22);         //This centers the text before I made the centering text function which works in the same way.
      tft.print(settings.profileName);                              //Print the name

    }

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);                 //Draw the dividing line and print all the info
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

    if (hours > 99) {                                                //Same trick but this time to align the number of hours and minutes with the : 

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

  if (infoType == 2) {                                   // Time remaining greyed out option.  Same as above but written in a dark grey to show that the Time Remaining isn't relevant to this screen 
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
    tft.fillRect(0, 0, 320, 179, ILI9341_BLACK);              // This is for the manual screens where you don't have a profile name to write
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

    if (infoType == 4) {                                   // This is special for the timer pages so it un-greys out the Time Remaining block when the timer starts running
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

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                   TEXT CENTERING FUNCTION SINCE THE DISPLAY LIBRARY DOESN'T HAVE ONE
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void centerText(String textValue, byte textSize, int yPos) {

  int nameLength = textValue.length();                                    //Get the length of the text
  tft.setTextSize(textSize);                                              //Set the text size to write according to the variable passed to this function
  tft.setCursor(160 - ((nameLength * (textSize * 6))) / 2, yPos);         //Calculate where to start the text to make it centered.  Half the screen width, minus the length of the string times the text size (1, 2 or 3) times 6 (5 pixels in a character plus one blank pixel for a space) divided by two to only subtract half the string length, the other half goes on past the centerling so we don't care about that
  tft.print(textValue);                                                   //Then print the text.  Easy peasy!

} 
