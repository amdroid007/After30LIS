// A simple Snake game based on the 30DLIS kit
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

#define left 0
#define right 1
#define up 2
#define down 3
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


volatile int gameStatus = gameStart;

// Other Game attributes

// various variables
int currentStateCLK;
int lastStateCLK;
int MyScore = 0;

class Snake {
  private:
    int direction = right;
    int head = -1;
    int length = 0;
    int x[128], y[128];

  public:
    Snake() {

    }

    void reset() {
      direction = -1;
      length = 1;
      head = 0;
      x[head] = random (0, 16);
      y[head] = random(0, 8);
    }

    void turn(int d) {
      if (d == right) {
        // Serial.println("bob turning right" );
        if (direction == right) direction = down;
        else if (direction == down) direction = right;
        else if (direction == left) direction = up;
        else if (direction == up) direction = right;
        else direction = right;
      }
      else if (d == left) {
        // Serial.println("bob turning left" );
        if (direction == right) direction = up;
        else if (direction == down) direction = left;
        else if (direction == left) direction = down;
        else if (direction == up) direction = left;
        else direction = left;
      }

    }

    void move () {
      if (direction<0) return;
      int headx = x[head];
      int heady = y[head];

      head = (head + 1) % 128;
      // Serial.print("Head is "); Serial.println(head);
      switch (direction) {
        case right: 
          x[head] = headx + 1;
          y[head] = heady;
          break;
        case left: 
          x[head] = headx - 1;
          y[head] = heady;
          break;
        case up: 
          x[head] = headx;
          y[head] = heady - 1;
          break;
        case down: 
          x[head] = headx;
          y[head] = heady + 1;
          break;
      }
      tone(buzzer, missSound, 50);
      checkCrash();
    }

    int onBody(int cx, int cy, int ignoreHead){
      for(int pos=0;pos<length;pos++){
       if (ignoreHead && pos==0) continue;
        int x = getx(pos);
        int y = gety(pos);
        if (x==cx && y==cy) return 1;
      }
      return 0;      
    }
    void checkCrash() {
      if (x[head] > 15) x[head] = 0;
      if (x[head] < 0) x[head] = 15;
      if (y[head] > 7) y[head] = 0;
      if (y[head] < 0) y[head] = 7;
      if (onBody (x[head],y[head],1)){
        gameStatus = gameEnd;
        tone (buzzer, 125, 100);
        delay(150);
        tone (buzzer, 125, 100);        
      }
    }

    void grow(){
      length++;
    }
    int getLength() {
      return length;
    }
    int getDirection() {
      return direction;
    }

    int getx(int pos) {
      int p=head-pos;
      if (p<0)
        p=p+128;
      return x[p];
    }
    int gety(int pos) {
      int p=head-pos;
      if (p<0)
        p=p+128;
      return y[p];
    }
};

class Fruit{
  public: 
    int x = 0;
    int y = 0;

    void create(Snake *bob){
      do {
        x = random (0, 16);
        y = random(0, 8);
      }while (bob->onBody(x,y,0));      
    }
};


Snake *bob = new Snake();
Fruit *apple = new Fruit();

// Display the score on the 7seg display
void ShowScore () {
  OurDisplay.showNumberDecEx(MyScore);
}

void playTone(uint16_t tone1, uint16_t duration) {
  if (tone1 < 50 || tone1 > 15000) return; // these do not play on a piezo
  for (long i = 0; i < duration * 1000L; i += tone1 * 2) {
    digitalWrite(buzzer, HIGH);
    delayMicroseconds(tone1);
    digitalWrite(buzzer, LOW);
    delayMicroseconds(tone1);
  }
}

void chirp() {  // Bird chirp
  for (uint8_t i = 200; i > 180; i--)
    playTone(i, 9);
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
    // Serial.println("right");
    // If the DT state is different than the CLK state then
    // the encoder is rotating CW so INCREASE counter by 1
    if (millis() - last_interrupt > DBOUNCE) {
      if (digitalRead(DT) != currentStateCLK) {
          bob->turn(right);

        } else {
          // Encoder is rotating CCW so DECREASE counter by 1
          // Serial.println("left");
          bob->turn(left);
        }
      last_interrupt = millis(); //note the last time the ISR was called
    }
  }

  // Remember last CLK state to use on next interrupt...
  lastStateCLK = currentStateCLK;
}


void StartStopGame () {
  static unsigned long last_interrupt = 0;
  if (millis() - last_interrupt > DBOUNCE) {
    if (gameStatus == gamePlaying) {
      bob->move();
    }
    else if (gameStatus == gameStart) gameStatus = gamePlaying;
    else gameStatus = gameStart;
  }
  last_interrupt = millis(); //note the last time the ISR was called

}

void resetGame () {
  MyScore = 0;
  bob->reset();
  apple->create(bob);  
}

void setup() {
  Serial.begin(9600);
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
    ShowScore ();
    bob->move();
    if (bob->getx(0)==apple->x && bob->gety(0)==apple->y){
      MyScore++;
      chirp();
      bob->grow();
      apple->create(bob);
    }
  }
  delay(200);
}

// ************************************************

void draw(void) {
  u8g_prepare();
  if (gameStatus == gamePlaying) {
    drawSnake();
    drawFruit();
  }
  else if (gameStatus == gameStart) {
    My_u8g_Panel.drawStr( 0, 10, "Welcome to");
    My_u8g_Panel.drawStr( 10, 30, "Snake!!");
    My_u8g_Panel.drawStr( 0, 50, "Push to begin");
    resetGame();
    ShowScore();
  }
  else {
    drawSnake();
    My_u8g_Panel.drawXBMP( 14, 12, 100, 15, gameOver);
  }
}
void u8g_prepare(void) {
  My_u8g_Panel.setFont(u8g_font_6x10);
  My_u8g_Panel.setFontRefHeightExtendedText();
  My_u8g_Panel.setDefaultForegroundColor();
  My_u8g_Panel.setFontPosTop();
}

void drawSnake() {
  int x = bob->getx(0);
  int y = bob->gety(0);
  //My_u8g_Panel.drawBox(x*8, y*8, 8,8);
  switch (bob->getDirection()){
    case right: My_u8g_Panel.drawStr( x * 8, y * 8, ">");break;
    case left: My_u8g_Panel.drawStr( x * 8, y * 8, "<");break;
    case up: My_u8g_Panel.drawStr( x * 8, y * 8, "^");break;
    case down: My_u8g_Panel.drawStr( x * 8, y * 8, "v");break;
    default: My_u8g_Panel.drawStr( x * 8, y * 8, "o");break;
  }

  for(int pos=1;pos<bob->getLength();pos++){
    int x = bob->getx(pos);
    int y = bob->gety(pos);
    My_u8g_Panel.drawStr( x * 8, y * 8, "O");
  }
}

void drawFruit(){
   My_u8g_Panel.drawStr( apple->x * 8, apple->y * 8, "*"); 
}
