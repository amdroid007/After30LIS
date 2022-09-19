// A simple Pong game based on the 30DLIS kit
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
#define gameStart 0
#define gameEnd 1
#define gamePlaying 2

volatile int gameStatus = gameStart;

// Other Game attributes
#define batwidth 20
#define batthick  3
#define balldia   7
#define screenwidth 128
#define screenheight 64
#define maxscore 20
#define minbatspeed 3
#define maxbatspeed 8

// various variables
int bat_position = 0;
int bat2_position = 0;
int ball_x = 0;
int ball_y = 0;
int xchange = 5;
int ychange = 5;
int batChange = 8;
int currentStateCLK;
int lastStateCLK;
int MyScore = 0;
int YourScore = 0;

// Display the score on the 7seg display
void ShowScore () {
  int score = MyScore * 100 + YourScore;
  OurDisplay.showNumberDecEx(score, 0x40, 1);
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

  // Remember last CLK state to use on next interrupt...
  lastStateCLK = currentStateCLK;
}

void StartStopGame () {
  if (gameStatus == gameStart) gameStatus = gamePlaying;
  else gameStatus = gameStart;
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

void move_ball() {
  if (gameStatus != gamePlaying) return;
  // increase the state
  ball_x += xchange;
  ball_y += ychange;
  batChange = random(minbatspeed, maxbatspeed);

  if (bat2_position < 0) bat2_position+=batChange;
  else if (bat2_position > (screenwidth - batwidth)) bat2_position -= batChange;
  else {
    if (bat2_position < ball_x) bat2_position += batChange;
    else bat2_position-= batChange;
  }
  if ( ball_x < balldia ) {
    xchange = 5;
    tone(buzzer, wallSound, 200);
  }
  if ( ball_x >= (screenwidth - balldia) ) {
    xchange = -5;
    tone(buzzer, wallSound, 200);
  }

  if ( ball_y  < balldia ) {
    if ( ball_x >= bat2_position && ball_x <= bat2_position + batwidth) {
      tone(buzzer, hitSound, 200);
    }
    else {
      tone(buzzer, missSound, 300);
      YourScore ++;
    }
    ball_y = balldia;
    ychange = 5;
  }
  if ( ball_y >= (screenheight - balldia) ) {
    if ( ball_x >= bat_position && ball_x <= bat_position + batwidth) {
      ychange = -5;
      tone(buzzer, hitSound, 200);
    }
    else {
      ball_y = balldia;
      tone(buzzer, missSound, 300);
      MyScore ++;
    }
  }
  ShowScore();
  if (MyScore == maxscore || YourScore == maxscore) gameStatus = gameEnd;
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
  if (gameStatus == gamePlaying) {
    u8g_bat();
    u8g_ball();
  }
  else if (gameStatus == gameStart) {
    My_u8g_Panel.drawStr( 0, 10, "Welcome to");
    My_u8g_Panel.drawStr( 10, 30, "PONG!");
    My_u8g_Panel.drawStr( 0, 50, "Push to begin");
    MyScore = 0;
    YourScore = 0;
    ShowScore();
  }
  else {
    My_u8g_Panel.drawStr( 0, 10, "** GAME OVER **");
    if (YourScore == maxscore) {
      My_u8g_Panel.drawStr( 20, 30, "You Win!");
    } else {
      My_u8g_Panel.drawStr( 25, 30, "I win!");
    }
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
  My_u8g_Panel.drawBox(bat2_position, 0, batwidth, batthick);
  My_u8g_Panel.drawBox(bat_position, (screenheight-batthick), batwidth, batthick);
}

void u8g_ball() {
  My_u8g_Panel.drawDisc(ball_x, ball_y, balldia);
}
