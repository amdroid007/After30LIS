// A simple Frogger game based on the 30DLIS kit
// Developed by Ishani and Arijit Sengupta
// Required libraries
#include <U8glib.h>
#include <TM1637Display.h>

// HW Pins required
#define SW 3 // RotaryEncoder SW Pin
#define DT 4 // RotaryEncoder DT Pin
#define CLK 2 // RotaryEncoder CLK Pin

// Define the display connections pins:
#define DCLK 6
#define DIO 5

// pin 10 drives the buzzer
#define buzzer  10

// In Addition, this sketch uses the I2C Pins for the U8G Panel
// A1+A2

// Create display object of type TM1637Display
// The display shows the current score
TM1637Display OurDisplay = TM1637Display(DCLK, DIO);

// Create array that turns all segments off:
const uint8_t blank[] = {0x00, 0x00, 0x00, 0x00}; // 0xff is a hexidecimal number whose binary
// representation is all zeros

// Create the oled display object - this shows gameplay
// and welcome/win/loss messages

U8GLIB_SH1106_128X64  My_u8g_Panel(U8G_I2C_OPT_NONE); // I2C / TWI

/*
  For documentation on u8glib functions:
  https://github.com/olikraus/u8glib/wiki/userreference
*/

// Sounds we use for the hit effects
#define hitSound 440
#define wallSound 247
#define missSound 147
#define DBOUNCE 180

// Game states
#define gameStart 0
#define gameEnd 1
#define gamePlaying 2

volatile int gameStatus = gameStart;

// PRE-SAVED BITMAP CONSTANTS
static unsigned char froggy [] U8G_PROGMEM= {
  0x02, 0x04, 0x7B, 0x0C, 0xBA, 0x05, 0xBE, 0x07, 0x06, 0x06, 0x00, 0x00, 
  0x10, 0x00, 0x1E, 0x06, 0x3E, 0x07, 0x9A, 0x05, 0x03, 0x0C, 0x02, 0x04
  };

static unsigned char car [] U8G_PROGMEM= {
  0xC0, 0x7F, 0x00, 0x20, 0x88, 0x00, 0x10, 0x08, 0x01, 0x08, 0x0C, 0x03, 
  0x03, 0x00, 0x0C, 0x01, 0x00, 0x08, 0x11, 0x80, 0x08, 0xCF, 0x3F, 0x0F, 
  0x4C, 0x20, 0x01, 0x38, 0xC0, 0x00, };


// Other Game attributes

// various variables
int currentStateCLK;
int lastStateCLK;
int MyScore = 0;
int frogx = 56;
int frogy = 0;
int frogRoad = 4;
int cars [4][2] = { {0,0},{0,0},{0,0},{0,0}};
int speeds [4] = {-3,5,-4,3};
// Display the score on the 7seg display
void ShowScore () {
  OurDisplay.showNumberDecEx(MyScore);
}

void playTone(uint16_t tone1, uint16_t duration) {
  if(tone1 < 50 || tone1 > 15000) return;  // these do not play on a piezo
  for (long i = 0; i < duration * 1000L; i += tone1 * 2) {
     digitalWrite(buzzer, HIGH);
     delayMicroseconds(tone1);
     digitalWrite(buzzer, LOW);
     delayMicroseconds(tone1);
  }     
}


void chirp() {  // Bird chirp
  for(uint8_t i=200; i>180; i--)
     playTone(i,9);
}

void createCars (){
   for (int i=0; i<4; i++){
    cars [i][0]=random (10,44);
    cars [i][1]=random (cars [i][0]+44,100);
  } 
}

void drawCars (){
  for (int i=0; i<4; i++){
    int carx=cars [i][0];
    int cary = i*13+1;
    My_u8g_Panel.drawXBMP( carx, cary, 20, 10, car); 
    carx=cars[i][1];
    My_u8g_Panel.drawXBMP( carx, cary, 20, 10, car); 

  }
}

void movecars(){
  for (int i=0; i<4; i++){
    if (speeds [i] <0) { //cars moving left
      int carx=cars [i][0];
      carx = carx+ speeds [i];
      if (carx < -20){
        cars[i][0]= cars [i][1];
        cars [i][1] =125;
      }
      else {
        cars [i] [0]=carx;
        cars[i][1]-=2;
      }
    } else {
      int carx=cars [i][1];
      carx = carx+ speeds [i];
      if (carx > 148){
        cars[i][1]= cars [i][0];
        cars [i][0] =-20;
      }
      else {
        cars [i] [1]=carx;
        cars[i][0]=cars [i][0]+speeds[i];
      }
    }
  }
}

// Interrupt handler for the encoder
// Get the data from the encoder
void updateEncoder() {
  // Read the current state of encoder CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 0->1 state change to avoid double counting
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CW so INCREASE counter by 1
    if (digitalRead(DT) != currentStateCLK) {
      if (frogx <= 112)
        frogx += 4;

    } else {
      if (frogx >= 4)
        frogx -= 4;
      // Encoder is rotating CCW so DECREASE counter by 1
    }
  }

  // Remember last CLK state to use on next interrupt...
  lastStateCLK = currentStateCLK;
}


void StartStopGame () {
   static unsigned long last_interrupt = 0;
    if(millis()-last_interrupt > DBOUNCE){
      if (gameStatus == gamePlaying) {
        if (frogRoad > 0){
          frogRoad --;
          MyScore ++;
        } else {
          frogRoad = 4;
          createCars();
        }
        chirp();
      }
      else if (gameStatus == gameStart) gameStatus = gamePlaying;
      else gameStatus = gameStart;
  }
  last_interrupt = millis(); //note the last time the ISR was called
  
}

void resetGame (){
  createCars ();
  MyScore = 0;
  frogx = 56;
  frogRoad = 4;
}
void setup() {
  OurDisplay.setBrightness(7);
  OurDisplay.clear();
  resetGame();
  ShowScore ();
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW), StartStopGame, FALLING);
}

// ************************************************
void loop(void) {
  My_u8g_Panel.firstPage();
  do {
    draw();
  } while ( My_u8g_Panel.nextPage() );
  
  if (gameStatus == gamePlaying) {
  
    movecars();
    if (frogRoad <4){
      int r = frogRoad;
      if (((frogx >= cars [r][0]-10)&&(frogx<= cars [r][0]+18)) ||
            ((frogx >= cars [r][1]-10)&&(frogx<= cars [r][1]+18))) {
              tone(buzzer, missSound, 500);
              gameStatus=gameEnd;
            }
    }
    ShowScore ();
  }
}

// ************************************************

void draw(void) {
  u8g_prepare();
  if (gameStatus == gamePlaying) {
    drawFrogger ();
    drawRoad ();
    drawCars ();
  }
  else if (gameStatus == gameStart) {
    My_u8g_Panel.drawStr( 0, 10, "Welcome to");
    My_u8g_Panel.drawStr( 10, 30, "Frogger!!");
    My_u8g_Panel.drawStr( 0, 50, "Push to begin");
   resetGame();
    ShowScore();
  }
  else {
    My_u8g_Panel.drawStr( 0, 10, "** GAME OVER **");
    char message[100] = "";
    sprintf(message, "You score: %d",MyScore);

    My_u8g_Panel.drawStr( 20, 30, message);
    My_u8g_Panel.drawStr( 0, 50, "Push to start");
  }
}


void drawFrogger (void) {
  frogy = frogRoad * 13;
  My_u8g_Panel.drawXBMP( frogx, frogy, 12, 12, froggy); 
 
}

void drawRoad (void) {
  My_u8g_Panel.drawHLine(0,12,128);
  My_u8g_Panel.drawHLine(0,25,128);
  My_u8g_Panel.drawHLine(0,38,128);
  My_u8g_Panel.drawHLine(0,51,128);
}
void u8g_prepare(void) {
  My_u8g_Panel.setFont(u8g_font_6x10);
  My_u8g_Panel.setFontRefHeightExtendedText();
  My_u8g_Panel.setDefaultForegroundColor();
  My_u8g_Panel.setFontPosTop();
}
