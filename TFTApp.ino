#include <ctime>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

//Define things for the TFT display. These have been set for the Seedunio xiao
#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 10
#define TFT_CLK 8
#define TFT_RST 5
#define TFT_MISO 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK,TFT_RST, TFT_MISO);

// Define the globals for the 4 front panel buttons and tone pin
#define TONEPIN 0
#define YELLOWPIN  1
#define BLUEPIN 2
#define GREENPIN 3
#define WHITEPIN 4

//prototype functions
void codeOscillator();
void sendGroups();
void dasBlinkenLights();

const char list[]="word1 word2 "
        "word3 ";

// ***********Global tables to define the code binary **************************
// In the lookup table, a zero means dit, one a dah.  Sending starts at low order
// and proceeds with a right shift until only the high order terminating one is left. 
  const uint8_t ltab[] = {
      0b110,              // A
      0b10001,            // B 
      0b10101,            // C
      0b1001,             // D
      0b10,               // E
      0b10100,            // F
      0b1011,             // G
      0b10000,            // H
      0b100,              // I
      0b11110,            // J
      0b1101,             // K
      0b10010,            // L
      0b111,              // M
      0b101,              // N
      0b1111,             // O
      0b10110,            // P
      0b11011,            // Q
      0b1010,             // R
      0b1000,             // S
      0b11,               // T
      0b1100,             // U
      0b11000,            // V
      0b1110,             // W
      0b11001,            // X
      0b11101,            // Y
      0b10011             // Z
  } ;
   
  const uint8_t ntab[] = {
      0b111111,           // 0
      0b111110,           // 1
      0b111100,           // 2
      0b111000,           // 3
      0b110000,           // 4
      0b100000,           // 5
      0b100001,           // 6
      0b100011,           // 7
      0b100111,           // 8
      0b101111            // 9
  } ;  
// class definitions
//***********************************************************************************
// Class for the CW sender
//***********************************************************************************

class CW {
  public:
    CW();         // constructor
    uint8_t getCode( uint8_t character);       // look up the binary for a character
    void sendCode(uint8_t code);            // send CW for character
    void sendSpace();                       // send space (word space)
    void setSpeed(int newSpeed);            // set the speed in words per minute
    int getSpeed();                         // get the speed in words per minute
    void setSpacing(int newSpacing);        // set the word spacing in units of dot periods
    int getSpacing();                       // get the word spacing
    void setTone(int newFreq);              // set the tone frequency
    int getTone()  ;                         // get the tone frequency
  private:
    int speed;                              // speed in words per minute
    int dotPeriod;                          // dot period in milliseconds
    int wordSpace;                        // word spacing in dot periods
    int toneFreq;                           // tone frequency in hertz
    //uint8_t ltab[];                         // letter to binary lookup table
    //uint8_t ntab[];                         // number to binary lookup table 
    

};

//******************** CW Constructor ***************************************************
CW::CW (){        
  speed = 25 ;
  dotPeriod = int( 1000./(speed/1.2) );
  wordSpace = 7; 
  toneFreq = 440;
       
}

//********** CW method to look up the binary code for a character ****************
uint8_t CW::getCode( uint8_t ch) {
    uint8_t charcode;         // binary code to be returned
    if (isalpha(ch)) {
        if (islower(ch)) ch = toupper(ch) ;
        charcode=(ltab[ch-'A']) ;
    } else if (isdigit(ch))
        charcode=(ntab[ch-'0']) ;
    else if (ch == ' ' || ch == '\r' || ch == '\n')
        sendSpace() ;
    else if (ch == '.')
        charcode=(0b1101010) ;
    else if (ch == ',')
        charcode=(0b1110011) ;
    else if (ch == '!')
        charcode=(0b1101011) ;
    else if (ch == '?')
        charcode=(0b1001100) ;
    else if (ch == '/')
        charcode=(0b101001) ;
    else if (ch == '+')
        charcode=(0b11010001) ; // use + for break code
    else if (ch == '-')
        charcode=(0b1100001) ;
    else if (ch == '=')         // this is the BT separator.
        charcode=(0b110001) ;
    else if (ch == '@')         // hardly anyone knows this!
        charcode=(0b1011010) ;

    else
        charcode=(0b00000001) ; // ignore anything else, this is null code
    return(charcode);
}
// *********** CW method to send a code character
void CW::sendCode(uint8_t charCode) {

  while (charCode != 1) {
    if ( (charCode & 1) == 1) {         //send a dah
      tone (TONEPIN,toneFreq);
      delay (3*dotPeriod);
      noTone(TONEPIN);
    }
    else {                              //send a dit
      tone (TONEPIN,toneFreq);
      delay (dotPeriod);
      noTone(TONEPIN);
    }
    delay(dotPeriod);                   // inter element delay
    charCode = charCode >> 1;            // shift to next element  
  }
  delay (2*dotPeriod);                  // inter character space = 3 dot periods
}

// ************* CW method to send inter word space
void CW::sendSpace() {

  delay((wordSpace-1) * dotPeriod);                   // makes a total dot periods counting end of character
  
}

//************** CW method to set the speed ***********************
void CW::setSpeed(int newSpeed) {

  speed = newSpeed;
  dotPeriod = int( 1000./(speed/1.2) );   
  return;
}

//*************** CW method to access current speed *****************
int CW::getSpeed() {
   return(speed);
}


//************** CW method to set the word spacing ***********************
void CW::setSpacing(int newSpacing) {

  wordSpace = newSpacing;
   return;
}

//*************** CW method to access current word spacing *****************
int CW::getSpacing() {
   return(wordSpace);
}
//*******************************************************************************************
// Class to implement the front panel buttons
//*******************************************************************************************
class Panel
{

  public:
    Panel();       // Constructor
    boolean readState(int button);
    void saveState(int button, boolean state);
    boolean didStateChange(int button, boolean state);
  private:    
    boolean prevYellow;    // previous state of the buttons
    boolean prevBlue;
    boolean prevGreen;
    boolean prevWhite;
};

// Panel constructor  - set up object and set initial state of the buttons

Panel::Panel()  {

// set up the input pins for the front buttons and key
// the key is connected to the yellow button. 
  pinMode(YELLOWPIN,INPUT);
  digitalWrite(YELLOWPIN,HIGH);
  prevYellow = true;  
  pinMode(BLUEPIN,INPUT);
  digitalWrite(BLUEPIN,HIGH);
  prevBlue = true;
  pinMode(GREENPIN,INPUT);
  digitalWrite(GREENPIN,HIGH);
  prevGreen = true;
  pinMode(WHITEPIN,INPUT);
  digitalWrite(WHITEPIN,HIGH);
  prevWhite = true; 
}

boolean Panel::readState(int button) {
  return(digitalRead(button));  
}

void Panel::saveState(int button, boolean state) {
  switch(button) {
    case YELLOWPIN :
      prevYellow = state;
      break;
    case BLUEPIN :
      prevBlue = state;
      break;
    case GREENPIN :
      prevGreen = state;
      break;
    case WHITEPIN :
      prevWhite = state;
      break;
    default :
      Serial.println("Invalid button in call to Panel.saveState"); 
  }
  return;
}
  
boolean Panel::didStateChange(int button, boolean state) {

  boolean returnValue = false;
  switch(button) {
    case YELLOWPIN :
      if (state != prevYellow) returnValue = true;
      break;
    case BLUEPIN :
      if (state != prevBlue) returnValue = true;
      break;
    case GREENPIN :
      if (state != prevGreen) returnValue = true;
      break;
    case WHITEPIN :
      if (state != prevWhite) returnValue = true;
      break;
    default :
      Serial.println("Invalid button in call to Panel.saveState"); 
  }    
  return (returnValue);
}



//*****************************************************************************
// Arduino setup, runs once
//*****************************************************************************
void setup() {
// put your setup code here, to run once:
  tft.begin();                        // initialize the display code


  
  Serial.begin(9600);
  Serial.println("  TFT Box Application");

  tft.setRotation(3);       // set TFT rotation to upright position
  
}

  Panel panel;        // Create panel object
  CW cw;              // Create CW sender object

//**********************************************************************************
// Arduino loop, runs forever (except fooled by code for initialization)
//**********************************************************************************

#define NUMAPPS 5
void loop() {
    
    int appNumber = 0;    // default to the first app

    writeMenu(appNumber);     // put out the menu with app highlighted    

    while(true) {   

 
  
      if (!panel.readState(YELLOWPIN)) {      // YELLOWPIN is the app select button
        switch(appNumber) {
          case 0:
             dasBlinkenLights();
            break;
          case 1:
             codeOscillator(); 
             break;
           case 2:
              sendGroups();
              break;
            case 3:
              sendCalls();
              break;
            case 4:
              settingsMenu();
          } // end of switch statement
          writeMenu(appNumber);
        } // end of yellow pin select

      if (!panel.readState(GREENPIN)) {         // GREENPIN is the increment (+) button
        delay(500);  // delay for button debounce
        appNumber = (++appNumber % NUMAPPS);
        showSelected(appNumber,NUMAPPS);
      }

      if (!panel.readState(BLUEPIN)) {         // BLUEPIN is the deccrement (-) button
        delay(500);  // delay for button debounce
        appNumber = (--appNumber % NUMAPPS);
        if (appNumber < 0)  appNumber = NUMAPPS - 1;  // wrap if under zero
        showSelected(appNumber,NUMAPPS);
      }
       
    }   // end of forever loop
    

} // end of mainline loop


void writeMenu(int selectedApp) {

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(50, 0); 
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Applications");
  tft.setTextSize(2);
  tft.println("Das Blinkenlights");
  tft.println("Code Oscillator");
  tft.println("Code Groups"); 
  tft.println("Call Signs");
  tft.println("Settings Menu"); 

  showSelected(selectedApp,NUMAPPS);       // show an arrow on selected app
}

// ***************** function to show arrow by selected item ****************
void showSelected(int selectedItem,int numberItems) {

#define ARROWX 210    // x location of the selection arrow
#define STARTY 25     // y location of the first item text
#define DELTAY 16     // width of the text in y direction

  for (int i=0 ;i < numberItems; i++) {     // run loop to clear previous arrow
    tft.fillRect(ARROWX,STARTY+(i*DELTAY),40,16,ILI9341_BLACK); // because blanks don't blank
  }

  tft.setCursor(ARROWX,STARTY+(selectedItem*DELTAY));  // Now draw select arrow
  tft.print("<--");

}

//************************************************************************************
// Code Oscillator Application
//************************************************************************************

void codeOscillator() {

// Announce our application
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(30, 0); tft.setRotation(3);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Code Oscillator");
  
  boolean finished = false;
  while(!finished) {
    boolean state = panel.readState(YELLOWPIN); 
    if (panel.didStateChange(YELLOWPIN,state) ){
   
        if (!state)  {        // put out tone on key down (false)
          tone(0,440);
        } else {
          noTone(0);
        }  
        panel.saveState(YELLOWPIN,state);
     }
     if (!panel.readState(WHITEPIN)) finished = true;
  } // end of while loop
  return;
}

//*****************************************************************************************
// Send Character Group Application
//*****************************************************************************************

void sendGroups() {

  
  boolean finished = false;
  while(!finished) {   // application while loop, runs until back button pressed
    
// Announce our application
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(30, 0); tft.setRotation(3);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Send Groups");
  tft.setTextSize(2);

  srand(time(NULL));    // randomize 

    for(int lineCount=0 ; lineCount<12 ; lineCount++) {  // output 12 lines of groups

      for (int groupCount=0 ; groupCount<4 ; groupCount++) { // output 4 groups on a line
        
        for (int letterCount=0 ; letterCount<5 ; letterCount++) {
          uint8_t letter = 'A' + (rand() % 26);
          uint8_t charCode = cw.getCode(letter);
          tft.printf("%c",letter);
          cw.sendCode(charCode); 
          if (!panel.readState(WHITEPIN)) return;  
        }
          tft.print(' ');
          cw.sendSpace();
      } // end of line loop
      tft.println();

    } // end of page after lineCount exhausted
    tft.println("White home, Yellow repeat");
    while (true) {
      if (!panel.readState(WHITEPIN) ) return;
      if (!panel.readState(YELLOWPIN) ) break;
        
    } // end of wait for next step loop

    
      
  } // end of application while loop
  return;
} // end of application

//*****************************************************************************************
// Send Call Signs Application
//*****************************************************************************************

void sendCalls() {

  
  boolean finished = false;
  while(!finished) {   // application while loop, runs until back button pressed
    
// Announce our application
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(30, 0); tft.setRotation(3);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Send Calls");
  tft.setTextSize(2);

  srand(time(NULL));    // randomize 

    for(int lineCount=0 ; lineCount<12 ; lineCount++) {  // output 12 lines of groups

      for (int callCount=0 ; callCount<4 ; callCount++) { // output 4 calls on a line 

        int callLength = 0;       // counter for number of letters in the call 
        int preMax = rand() % 2 + 1;  // Generate length of the prefix  
        callLength += preMax;    
        for (int letterCount=0 ; letterCount<preMax ; letterCount++) {
          uint8_t letter = 'A' + (rand() % 26);
          uint8_t charCode = cw.getCode(letter);
          tft.printf("%c",letter);
          cw.sendCode(charCode); 
          if (!panel.readState(WHITEPIN)) return;  
        } // end of prefix loop

        callLength += 1;
        uint8_t number= '0' + (rand() % 10);    // now send a number
        uint8_t charCode = cw.getCode(number);
        tft.printf("%c",number);
        cw.sendCode(charCode); 
        if (!panel.readState(WHITEPIN)) return;  

        int sufMax = rand() % 3 + 1;  // Generate length of suffix
        if( (callCount ==3) && (preMax >1) )sufMax = 2; // limit last call on line to 5 char (TFT size)
        callLength += sufMax;        
        for (int letterCount=0 ; letterCount<sufMax ; letterCount++) {
          uint8_t letter = 'A' + (rand() % 26);
          uint8_t charCode = cw.getCode(letter);
          tft.printf("%c",letter);
          cw.sendCode(charCode); 
          if (!panel.readState(WHITEPIN)) return;  
        } // end of suffix loop
        
          cw.sendSpace();


          if (callCount <3) {       // line up calls by columns 
            while(callLength < 7) {   
              tft.print(' ');
              callLength++;
            }
          } 
      } // end of line loop
      tft.println();

    } // end of page after lineCount exhausted
    tft.println("White home, Yellow repeat");
    while (true) {
      if (!panel.readState(WHITEPIN) ) return;
      if (!panel.readState(YELLOWPIN) ) break;
        
    } // end of wait for next step loop
      
  } // end of application while loop
  return;
} // end of application

//******************************************************************************************
// Das Blinkenlights Application
//*******************************************************************************************
void dasBlinkenLights() {
  boolean sound = false;         // whether to play sound or not
  boolean multiColor = false;    // whether or not to use colors other than red  
  int counter = 0;               // loop counter for sound player
  boolean finished = false;

// Announce our application
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 0); tft.setRotation(3);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Das Blinkenlights");
  tft.setTextSize(2);
  tone(TONEPIN,440);
  noTone(TONEPIN);
  
  while(!finished) {
    int xCell = rand() % 32;
    int yCell = rand() % 20;

    if  ((rand() % 2) == 1) {
      if (multiColor) {
        switch(rand() % 3) {
          case 0:
            tft.fillCircle(5+xCell*10,35+(yCell*10),2,ILI9341_RED);
            break;
          case 1:
            tft.fillCircle(5+xCell*10,35+(yCell*10),2,ILI9341_GREEN);
            break;
          case 2:
            tft.fillCircle(5+xCell*10,35+(yCell*10),2,ILI9341_WHITE);
        } // end of color selector switch 
      } else   {   // if multicolor not set
            tft.fillCircle(5+xCell*10,35+(yCell*10),2,ILI9341_RED); // single color use red        
      }
    }
    else tft.fillCircle(5+xCell*10,35+(yCell*10),2,ILI9341_BLACK);

    if (!panel.readState(GREENPIN) ) {      // green switch turns sound on
      delay(500);    // delay for debounced
      sound = true;
    }   

    if (!panel.readState(BLUEPIN) ) {       // blue switch turns sound and multicolor off
      delay(500);    // delay for debounced
      sound = false;
      noTone(TONEPIN);
      multiColor = false;
    }  


    if (!panel.readState(YELLOWPIN) ) {       // yellow switch turns multicolor on
      delay(500);    // delay for debounced
      multiColor = true;
    }       
    
    if (!panel.readState(WHITEPIN) ) {
      finished = true;
      break;
    }
    counter++;      // update sound loop counter
    if (sound && (counter%200 == 1) ) {     // if sound on, generate tone on each 200th loop execution
      switch (rand() % 6) {
        case 0 :
          tone(TONEPIN,262);  // middle C   // notes in middle Cmin pentatonic
          break;
        case 1 :
          tone(TONEPIN,311);  // middle Eb
          break;        
        case 2 :
          tone(TONEPIN,349);  // middle F
          break;
        case 3 :
          tone(TONEPIN,392);  // middle G
          break;  
        case 4 :
          tone(TONEPIN,466);  // middle Bb
          break; 
        case 5 :
          tone(TONEPIN,523);  // C again
          break;     
       }
    }
  }  // end of while loop  
  noTone(TONEPIN);
  return;
} // end of application
  
//******************************************************************************************
// Settings Menu
//*******************************************************************************************
void settingsMenu() {

  int numSettings = 2;      // number of items in the setting menu
  int settingNumber = 0;    // default to first option
  
  boolean finished = false;

  while(!finished) {
    // Announce our application
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 0); tft.setRotation(3);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.println("Settings Menu");
    tft.setTextSize(2); 
    tft.print("Code Speed    ");tft.println(cw.getSpeed());
    tft.print("Word Spacing  ");tft.println(cw.getSpacing());
    showSelected(settingNumber,numSettings);
  
    while (true) {          // check buttons and handle loop
      
      if (!panel.readState(WHITEPIN) ) {          // go back a level in the menu system
        finished = true;
        break;
      }
      if (!panel.readState(GREENPIN) ) {
          delay(500);   // delay for button debounce
          settingNumber = ++settingNumber % numSettings;   // increment the setting number
          showSelected(settingNumber,numSettings);
      }
      if (!panel.readState(BLUEPIN) ) {
          delay(500);   // delay for button debounce
          settingNumber = --settingNumber % numSettings;   // decrement the setting number 
          if(settingNumber < 0) settingNumber = numSettings - 1; // wrap if underflow         
          showSelected(settingNumber,numSettings);
      }      
      if (!panel.readState(YELLOWPIN) ) {
        delay(500);     // delay for button debounce
        switch(settingNumber) {
          case 0:
            changeSpeed();
            break;  
          case 1:
            changeSpacing();
            break;    
        } // end of switch statement
        
        break;      // break out of check buttons loop
        
       } // end of yellowpin check
    }
        
  } // end of settings menu loop
}// end of settings menu application 

// **************** Settings handler for code speed adjust *********************************
void changeSpeed() {  
  boolean finished = false;

    // Announce our handler
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 0); tft.setRotation(3);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.println("  Speed Adjust");
    tft.setTextSize(2); 
    tft.setCursor (0,100);
    tft.print("Code Speed    ");tft.println(cw.getSpeed());  

  while( !finished ) {

    if (!panel.readState(WHITEPIN) ) {   // exit handler on white pin
      finished = true;
      delay(500);      // delay for button debounce
      break;
    }
    if (!panel.readState(GREENPIN) ) {
      delay(500);     //  delay for button debounce
      cw.setSpeed( cw.getSpeed() + 1);
      tft.fillRect(0,100,239,15,ILI9341_BLACK);     // blank the line before updating
      tft.setCursor (0,100);          // write out the new speed
      tft.print("Code Speed    ");tft.println(cw.getSpeed());
    }
    if (!panel.readState(BLUEPIN) ) {
      delay(500);     //  delay for button debounce
      cw.setSpeed( cw.getSpeed() -1);
      if (cw.getSpeed() < 10) cw.setSpeed(10);      // don't go below 10
      tft.fillRect(0,100,239,15,ILI9341_BLACK);     // blank the line before updating
      tft.setCursor (0,100);          // write out the new speed
      tft.print("Code Speed    ");tft.println(cw.getSpeed());
    }
    
  } // end of button wait loop
  return;
} // end of change speed handler


// **************** Settings handler for code word spacing adjust *********************************
void changeSpacing() {  
  boolean finished = false;

    // Announce our handler
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(10, 0); tft.setRotation(3);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.println(" Spacing Adjust");
    tft.setTextSize(2); 
    tft.setCursor (0,100);
    tft.print("Word Spacing    ");tft.println(cw.getSpacing());  

  while( !finished ) {

    if (!panel.readState(WHITEPIN) ) {   // exit handler on white pin
      finished = true;
      delay(500);      // delay for button debounce
      break;
    }
    if (!panel.readState(GREENPIN) ) {
      delay(500);     //  delay for button debounce
      cw.setSpacing( cw.getSpacing() + 1);
      tft.fillRect(0,100,239,15,ILI9341_BLACK);     // blank the line before updating
      tft.setCursor (0,100);          // write out the new spacing
      tft.print("Word Spacing    ");tft.println(cw.getSpacing());
    }
    if (!panel.readState(BLUEPIN) ) {
      delay(500);     //  delay for button debounce
      cw.setSpacing( cw.getSpacing() -1);
      if (cw.getSpeed() < 7) cw.setSpeed(7);      // don't go below 7
      tft.fillRect(0,100,239,15,ILI9341_BLACK);     // blank the line before updating
      tft.setCursor (0,100);          // write out the new speed
      tft.print("Word Spacing    ");tft.println(cw.getSpacing());
    }
    
  } // end of button wait loop
  return;
} // end of change word spacing handler
