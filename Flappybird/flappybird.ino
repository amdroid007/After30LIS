// A simple Flappybird game based on the 30DLIS kit
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
#define jumpSound 700
#define blahSound 125
#define speedSound 1000
#define DBOUNCE 50

// Game states
#define gameStart 0
#define gameEnd 1
#define gamePlaying 2

volatile int gameStatus = gameStart;

// PRE-SAVED BITMAP CONSTANTS
//20 x 14
static unsigned char flappyStraight [] U8G_PROGMEM= {
  0x00, 0x1F, 0x00, 0x80, 0x3F, 0x00, 0xF0, 0xCF, 0x00, 0xF8, 0x0F, 0x01, 
  0xFE, 0x4F, 0x01, 0xC1, 0x4F, 0x01, 0x81, 0x1F, 0x01, 0x81, 0xFF, 0x03, 
  0xC2, 0x1F, 0x04, 0xFC, 0xEF, 0x07, 0xE0, 0x0F, 0x02, 0xC0, 0xFF, 0x03, 
  0x00, 0x1F, 0x00, 0x00, 0x0E, 0x00, };

//20 x 15  
static unsigned char flappyUp [] U8G_PROGMEM= {
  0x00, 0x07, 0x00, 0x80, 0x7D, 0x00, 0xE0, 0x41, 0x00, 0xF0, 0xD1, 0x00, 
  0xF0, 0x91, 0x07, 0xFC, 0xC3, 0x00, 0xFC, 0x3F, 0x06, 0xFE, 0x9F, 0x05, 
  0xFE, 0x5F, 0x0C, 0x83, 0x3F, 0x03, 0x81, 0xBF, 0x01, 0x81, 0x7F, 0x00, 
  0x83, 0x7F, 0x00, 0xE6, 0x3F, 0x00, 0x38, 0x0A, 0x00, };

//14 x 20
static unsigned char flappyDown [] U8G_PROGMEM= {
  0xE0, 0x01, 0x30, 0x00, 0x18, 0x00, 0x08, 0x04, 0x18, 0x0E, 0x1C, 0x1F, 
  0xFE, 0x1F, 0xFE, 0x3F, 0xFF, 0x3F, 0xFF, 0x3F, 0xFF, 0x3F, 0xFF, 0x3F, 
  0xE7, 0x30, 0x56, 0x30, 0x56, 0x1F, 0x54, 0x0C, 0xD4, 0x0F, 0xD4, 0x03, 
  0x5C, 0x00, 0x20, 0x00, };

static unsigned char gameOver [] U8G_PROGMEM= {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xF8, 0x01, 0x02, 0x0C, 0x0C, 0x3E, 0x00, 0x78, 0x60, 
  0x30, 0x7C, 0xF0, 0x01, 0x0C, 0x01, 0x07, 0x14, 0x0A, 0x02, 0x00, 0x84, 
  0x40, 0x10, 0x04, 0x10, 0x02, 0x04, 0x00, 0x05, 0x14, 0x0A, 0x02, 0x00, 
  0x02, 0x41, 0x10, 0x04, 0x10, 0x02, 0x04, 0x00, 0x05, 0x14, 0x0A, 0x02, 
  0x00, 0x02, 0xC1, 0x18, 0x04, 0x10, 0x02, 0xC4, 0x81, 0x0D, 0x34, 0x0B, 
  0x3E, 0x00, 0x02, 0x81, 0x08, 0x7C, 0xF0, 0x01, 0x04, 0x81, 0x08, 0x24, 
  0x09, 0x02, 0x00, 0x02, 0x81, 0x0D, 0x04, 0x10, 0x01, 0x04, 0x81, 0x0F, 
  0x64, 0x09, 0x02, 0x00, 0x02, 0x01, 0x05, 0x04, 0x10, 0x02, 0x0C, 0xC1, 
  0x18, 0xC4, 0x08, 0x02, 0x00, 0x84, 0x00, 0x05, 0x04, 0x10, 0x02, 0xF8, 
  0x41, 0x10, 0xC4, 0x08, 0x3E, 0x00, 0x78, 0x00, 0x07, 0x7C, 0x10, 0x02, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, };


// Other Game attributes

// various variables
int currentStateCLK;
int lastStateCLK;
int MyScore = 0;
int flappyPos = 32;
volatile int jumping = 0;
volatile long pushTime = 0;
int pipex [2] = {128,200};
int pipel [2]={5,5};
int pipeg [2]={40,40};
int fly = 0;

int speed = 5;

// Display the score on the 7seg display
void ShowScore () {
  if (gameStatus == gamePlaying) {
    OurDisplay.showNumberDecEx(MyScore);
  }
}

// Interrupt handler for the encoder
// Get the data from the encoder
void updateEncoder() {
  // Read the current state of encoder CLK
  currentStateCLK = digitalRead(CLK);
  // If last and current state of CLK are different, then pulse occurred
  // React to only 0->1 state change to avoid double counting
  static unsigned long last_interrupt = 0;
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
    // If the DT state is different than the CLK state then
    // the encoder is rotating CW so INCREASE counter by 1
    if (jumping !=0)jumping = 3;
    if (digitalRead(DT) != currentStateCLK) {
     flappyPos = flappyPos+3;
    } else {
      // Encoder is rotating CCW so DECREASE counter by 1
      flappyPos = flappyPos -3;
    }
    pushTime = millis();
  }

  last_interrupt = millis(); //note the last time the ISR was called
  // Remember last CLK state to use on next interrupt...
  lastStateCLK = currentStateCLK;
}

void StartStopGame () {
   static unsigned long last_interrupt = 0;
   if(millis()-last_interrupt > DBOUNCE){
    if (digitalRead(SW) == 0) {
      if (gameStatus == gamePlaying) {
          jumping = 1;
          tone (buzzer, jumpSound, 100);
          pushTime = millis();
      }
      else if (gameStatus == gameStart) gameStatus = gamePlaying;
      else gameStatus = gameStart;    
    }
  }
  last_interrupt = millis(); //note the last time the ISR was called  
}

void resetGame (){
  MyScore = 0;
  pipex[0] = 128;
  pipex [1]= 200;
  flappyPos = 32;
  jumping = 0;
  pushTime = 0;
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
  attachInterrupt(digitalPinToInterrupt(SW), StartStopGame, CHANGE);
}

// ************************************************
void loop(void) {
  ShowScore();
  My_u8g_Panel.firstPage();
  do {
    draw();
  } while ( My_u8g_Panel.nextPage() );
  if (gameStatus == gamePlaying) {
    moveFlappy();
    movePipes();
  }
}

void moveFlappy (){
  if(jumping ==0) fly = ( fly+1) %2;
  else if (jumping == 1) {    
    if (flappyPos>-15) flappyPos = flappyPos-6;
    if (millis()-pushTime>200 ) jumping = 2;
  }else if (jumping == 2){
    if (flappyPos<55) flappyPos = flappyPos +4;
  }else{
    fly = ( fly+1) %2;
    if (millis()-pushTime>500 ) jumping = 2;
  }
  checkCollision ();
}

void movePipes(){
  if (jumping == 0) return;
    int obx=pipex [0];
    obx = obx- speed;
    if (obx < -20){
      MyScore++;
      pipex[0]= pipex[1];
      pipex[1] =pipex[0]+random(80,125);
      pipel[0]=pipel[1];
      pipeg[0]=pipeg[1];
      pipel[1]=random (2,20);
      pipeg[1]=random (40,50);
    }
    else {
      pipex[0]=obx;
      pipex[1]-=speed;
    }
 
}
// ************************************************

void draw(void) {
  u8g_prepare();
  if (gameStatus == gamePlaying) {
    drawFlappy ();
    drawPipes ();
  }
  else if (gameStatus == gameStart) {
    My_u8g_Panel.drawStr( 0, 10, "Welcome to");
    My_u8g_Panel.drawStr( 10, 30, "Flappybird!!");
    My_u8g_Panel.drawStr( 0, 50, "Push to begin");
   resetGame();
    ShowScore();
  }
  else {
    My_u8g_Panel.drawXBMP( 14, 12, 100, 15, gameOver);
    drawFlappy ();
    drawPipes ();
  }
    
}


void drawFlappy (void) {
  if (gameStatus == gameEnd){
    My_u8g_Panel.drawXBMP( 0, flappyPos - 10, 14, 20, flappyDown);
    return;
  }
  switch (jumping) {
    case 0:
    case 3:if (fly) My_u8g_Panel.drawXBMP( 0, flappyPos - 7, 20, 14, flappyStraight); 
      else My_u8g_Panel.drawXBMP( 0, flappyPos - 7, 20, 15, flappyUp);
      break;
    case 1:My_u8g_Panel.drawXBMP( 0, flappyPos - 7, 20, 15, flappyUp); break;
    case 2:My_u8g_Panel.drawXBMP( 0, flappyPos - 10, 14, 20, flappyDown); break;
    default:My_u8g_Panel.drawXBMP( 0, flappyPos - 10, 14, 20, flappyDown); break;
  }
}

int crashed(){
  int obx = pipex [0];
  int obl = pipel[0];
  int obg = pipeg[0];
  if (flappyPos>55) return 1;
  if (obx >18) return 0;
  
  if (flappyPos>obl+7 && flappyPos < obl+obg-7) return 0;
  return 1;
}

void checkCollision (){
  if (crashed()) {
    gameStatus = gameEnd;
    tone (buzzer, 125, 100);
    delay(150);
    tone (buzzer, 125, 100);
  }
}

void drawPipes (){
  My_u8g_Panel.drawBox(pipex[0],0,20,pipel[0]);
  if (pipel[0]+pipeg[0]<64) 
    My_u8g_Panel.drawBox(pipex[0],pipel[0]+pipeg[0],20,64-pipel[0]-pipeg[0]);
  
  My_u8g_Panel.drawBox(pipex[1],0,20,pipel[1]);
  if (pipel[1]+pipeg[1]<64) 
    My_u8g_Panel.drawBox(pipex[1],pipel[1]+pipeg[1],20,64-pipel[1]-pipeg[1]);
}

void u8g_prepare(void) {
  My_u8g_Panel.setFont(u8g_font_6x10);
  My_u8g_Panel.setFontRefHeightExtendedText();
  My_u8g_Panel.setDefaultForegroundColor();
  My_u8g_Panel.setFontPosTop();
}
