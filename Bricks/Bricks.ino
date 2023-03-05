// A simple Bricks game based on the 30DLIS kit
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

// Game states
#define gameReady 3
#define gameStart 0
#define gameEnd 1
#define gamePlaying 2

volatile int gameStatus = gameReady;

// Other Game attributes
#define batwidth 20
#define batthick  3
#define balldia   2
#define screenwidth 128
#define screenheight 64
#define minbatspeed 3
#define maxbatspeed 8
#define rows 5
#define cols 8

// various variables
int bat_position = 0;
int ball_x = 0;
int ball_y = 58;
int xchange = 5;
int ychange = -5;
int currentStateCLK;
int lastStateCLK;
int MyScore = 0;
int bricks [cols][rows];
int lives = 3;

// Display the score on the 7seg display
void ShowScore () {
  int score=lives *100+MyScore;
  OurDisplay.showNumberDecEx(score,0b01000000);
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
      if (bat_position < (screenwidth - batwidth))
        bat_position += 5;
    } else {
      // Encoder is rotating CCW so DECREASE counter by 1
      if (bat_position > 0)
        bat_position -= 5;
    }
  }
  if (gameStatus==gameStart) move_ball();
  // Remember last CLK state to use on next interrupt...
  lastStateCLK = currentStateCLK;
}

/**
 * Interrupt handler for button
 **/
void StartStopGame () {
  if (gameStatus == gameReady) { // Ready state - showing welcome
    gameStatus = gameStart;
    resetGame();
  }
  else if (gameStatus== gameStart) gameStatus= gamePlaying;
  else if (gameStatus==gameEnd) gameStatus=gameReady;
}

/**
 * Initialize the game and board
 */
void resetGame(){
  //initialize the array
  for (int c=0; c<cols; c++)
    for (int r=0; r<rows; r++)
      bricks [c][r]=1;
  MyScore=0;
  lives=3;
}

void setup() {
  OurDisplay.setBrightness(7);
  OurDisplay.clear();
  ShowScore ();
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SW), StartStopGame, FALLING);
}

/**
 * main ball moving routine - most of the physics needs
 * to be here
 **/
void move_ball() {
  if (gameStatus == gameStart){ // start mode - just follow center of bat
    ball_x = bat_position+(batwidth/2);
    ball_y = screenheight-batthick-balldia;
  }
  else if (gameStatus == gamePlaying){
    // increase the state
    ball_x += xchange;
    ball_y += ychange;
  
    if ( ball_x < balldia ) { // Ball off the left of screen
      xchange = 5;
      tone(buzzer, wallSound, 200);
    }
    if ( ball_x >= (screenwidth - balldia) ) { // ball off the right
      xchange = -5;
      tone(buzzer, wallSound, 200);
    }
  
    if ( ball_y  < balldia ) { // Ball at the top of screen
        tone(buzzer, wallSound, 200);
      ball_y = balldia;
      ychange = 5;
    }
    if ( ball_y >= (screenheight - balldia) ) { // ball at the bottom
      // if we hit the bat then reflect
      if ( ball_x >= bat_position && ball_x <= bat_position + batwidth) {
        ychange = -5;
        tone(buzzer, hitSound, 200);
      }
      else { // Otherwise we lose a life
        tone(buzzer, missSound, 300);
        lives--;
        if (lives==0)gameStatus=gameEnd;
        else gameStatus=gameStart;
      }
    }
    checkCollision();
    ShowScore();
   if (MyScore>=40 ) gameStatus = gameEnd;
  }
}

/**
 * Check if the ball hit any of the bricks
 * Could improve accuracy a bit here.
 **/
void checkCollision() {
   for (int c=0; c<cols; c++)
    for (int r=0; r<rows; r++){
      int bx = c*16;
      int by = r*5;
      if (ball_x>= bx && ball_x<=bx+15 && ball_y>=by && ball_y<=by+5){
        if (bricks [c][r]==1){
          bricks [c][r]=0;
          MyScore++;
          tone(buzzer, missSound, 100);
          ychange=ychange*-1;
          return;
        }
      }
    }

}
// ************************************************
void loop(void) {
  // picture loop
  My_u8g_Panel.firstPage();
  do {
    draw();
  } while ( My_u8g_Panel.nextPage() );

  move_ball();

}

// ************************************************

void draw(void) {
  u8g_prepare();
  if (gameStatus == gamePlaying || gameStatus==gameStart) {
    u8g_bricks();
    u8g_bat();
    u8g_ball();
  }
  else if (gameStatus == gameReady) {
    My_u8g_Panel.drawStr( 0, 10, "Welcome to");
    My_u8g_Panel.drawStr( 10, 30, "BRICKS!");
    My_u8g_Panel.drawStr( 0, 50, "Push to begin");
    MyScore = 0;
    ShowScore();
  }
  else {
    My_u8g_Panel.drawStr( 0, 10, "** GAME OVER **");
    My_u8g_Panel.drawStr( 0, 50, "Push to start");
  }
}

void u8g_prepare(void) {
  My_u8g_Panel.setFont(u8g_font_6x10);
  My_u8g_Panel.setFontRefHeightExtendedText();
  My_u8g_Panel.setDefaultForegroundColor();
  My_u8g_Panel.setFontPosTop();
}

void u8g_bat() {
  My_u8g_Panel.drawBox(bat_position, (screenheight-batthick), batwidth, batthick);
}

void u8g_bricks() {
  for (int c=0; c<cols; c++)
    for (int r=0; r<rows; r++)
      if (bricks [c][r]==1)
        My_u8g_Panel.drawBox(16*c, 5*r, 15, 4);
}

void u8g_ball() {
  My_u8g_Panel.drawDisc(ball_x, ball_y, balldia);
}

