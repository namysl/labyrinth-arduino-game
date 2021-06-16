#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

#define RESET_BUTTON 2
#define OLED_RESET 4
#define END_GAME_LEVEL 5

#define SCREEN_WIDTH 128 // szerokosc oleda w px
#define SCREEN_HEIGHT 64 // wysokosc  oleda w px

#define JOY_ANALOG_Y 0
#define JOY_ANALOG_X 1

// SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM krzyzyk [] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
static const unsigned char PROGMEM sciana [] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};
static const unsigned char PROGMEM gracz [] = {0x00, 0x3c, 0x5a, 0x7e, 0x5a, 0x66, 0x3c, 0x00};
static const unsigned char PROGMEM meta [] = {0x00, 0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18};

static const unsigned char PROGMEM strzalka [] = {0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x03, 0xc0, //strzalka przy wyborze levela
                                                  0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0,
                                                  0x03, 0xc0, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0,
                                                  0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00};

static const unsigned char PROGMEM powodzenia [] = {0xfe,0x00,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x0c,0x00,0x18,
                                                    0x63,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x18,
                                                    0x63,0x0f,0x1e,0x78,0xf0,0x36,0x7f,0x0e,0x2f,0x3c,0x3e,0x18,
                                                    0x63,0x19,0x8d,0xb1,0x98,0x6e,0x63,0x1b,0x19,0x8c,0x63,0x18,
                                                    0x63,0x30,0xcd,0xb3,0x0c,0xc6,0x66,0x31,0x99,0x8c,0x03,0x18,
                                                    0x7e,0x30,0xcd,0xb3,0x0c,0xc6,0x0c,0x3f,0x99,0x8c,0x3f,0x18,
                                                    0x60,0x30,0xcf,0xf3,0x0c,0xc6,0x18,0x30,0x19,0x8c,0x63,0x18,
                                                    0x60,0x30,0xc7,0xe3,0x0c,0xc6,0x33,0x30,0x19,0x8c,0x63,0x00,
                                                    0x60,0x19,0x86,0x61,0x98,0x6e,0x63,0x19,0x99,0x8c,0x67,0x18,
                                                    0xf8,0x0f,0x06,0x60,0xf0,0x37,0x7f,0x0f,0x3b,0xff,0x3b,0x98};

static const unsigned char PROGMEM duza_strzalka [] = {0x00,0x00,0x04,0x00,0x00,0x00,0x02,0x00,
                                                       0x00,0x00,0x05,0x00,0x00,0x00,0x02,0x80,
                                                       0x00,0x00,0x05,0x40,0xaa,0xaa,0xaa,0xa0,
                                                       0x55,0x55,0x55,0x50,0xaa,0xaa,0xaa,0xa8,
                                                       0x55,0x55,0x55,0x54,0xaa,0xaa,0xaa,0xaa,
                                                       0x55,0x55,0x55,0x54,0xaa,0xaa,0xaa,0xa8,
                                                       0x55,0x55,0x55,0x50,0xaa,0xaa,0xaa,0xa0,
                                                       0x00,0x00,0x05,0x40,0x00,0x00,0x02,0x80,
                                                       0x00,0x00,0x05,0x00,0x00,0x00,0x02,0x00,
                                                       0x00,0x00,0x04,0x00};
  
unsigned char level [] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0x81, 0x81, 
                          0x81, 0x81, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00};

struct X {
  byte a; //nieuzywane juz
  byte x; //na pozycji x
  byte y; //na y
};

struct LVL {
  byte px; //pozycja gracza na x
  byte py; //pozycja gracza na y
  byte l;  //mapa levela do wczytania
};

                                     //{x grzacz, y gracz, ktora mape zaladowac}
static const LVL levels [] PROGMEM = {{5, 1, 0},  //1
                                      {6, 1, 0},  //2
                                      {5, 6, 1},  //3
                                      {10, 6, 2}, //4
                                      {5, 6, 3}   //5
};

static const unsigned char level_maps [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00, //1 i 2 lvl
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xe1, 0x8d, 0x81, 0x81, 0xe1, 0xFF, 0x00, 0x00, 0x00, 0x00, //3 lvl
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0x8d, 0xa1, 0xa1, 0x8d, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00, //4 lvl
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xc9, 0x81, 0xb1, 0x81, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00  //5 lvl
  };
  
int piezoPin = 11; //buzzer
unsigned char x;
unsigned char y;
int i;
int joy_x = 0;
int joy_y = 0;
int pl_x = 8;
int pl_y = 8;
int pm_x = 0;
int pm_y = 0;
byte cur_level = 0;
int menu_pos = 0;
char menu_dir = 0;
byte menu_lev = 0;
bool in_menu = true;

int X_iter = 0;
int X_arr_x[64];
int X_arr_y[64];

int target_x[] = {10, 5, 7, 9, 10};  //meta x os do wczytania
int target_y[] = {1, 1, 1, 4, 4};    //meta y os

int moves_to_finish[] = {36, 36, 30, 30, 32};  //kroki do ukonczenia
int lvl_cntr = 0;

void setup() {
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  in_menu = true;
}


void load_level(byte l)  //wczytywanie levela
{
  in_menu = false;
  display.clearDisplay();
  display.drawBitmap(16, 15, powodzenia, 93, 10, 1);
  display.drawBitmap(47, 30, duza_strzalka, 31, 19, 1);
  display.display();

  tone(piezoPin, 523, 2000);
  delay(400);
  tone(piezoPin, 587, 2000);
  delay(400);
  tone(piezoPin, 523, 2000);
  delay(500);
  
  //load level: program memory -> ram
  LVL lvl;
  memcpy_P(&lvl, &levels[l], sizeof(LVL));
  
  //load level map: progmem -> ram
  memcpy_P(&level, &level_maps[lvl.l * 16], 16);

  pl_x = lvl.px * 8;  //pozycja gracza
  pl_y = lvl.py * 8;
  pm_x = pm_y = 0;
  delay(1000);
}

void loop() {
  if (in_menu) {
    menu_loop();
  } else {
    game_loop();
  }
}


void menu_loop()
{
  joy_x = (analogRead(JOY_ANALOG_X) - 512) / 100;

  if (joy_x > 0 && menu_pos == menu_lev * 48) {
    if (menu_lev < END_GAME_LEVEL - 1) menu_lev++;
  }
  if (joy_x < 0 && menu_pos == menu_lev * 48) {
    if (menu_lev > 0) menu_lev--;
  }

  if (menu_pos > menu_lev * 48) {
    menu_pos -= 8;
    if (menu_pos < menu_lev * 48) menu_pos = menu_lev * 48;
  }
  if (menu_pos < menu_lev * 48) {
    menu_pos += 8;
    if (menu_pos > menu_lev * 48) menu_pos = menu_lev * 48;
  }

  display.clearDisplay();

  display.fillRoundRect(0, 48, 128, 16, 30, WHITE);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(25, 52);
  display.print(String("wybierz poziom"));
  display.drawBitmap(55, 2, strzalka, 16, 16, 1);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  
  //1 lvl -> 5 lvl
  for (i = 0; i < END_GAME_LEVEL; i++) {  
    int x = 64 - (i+1 < 10 ? 7 : 16) - menu_pos + i * 48;
    if (x < - 24 || x > 152) continue;
    display.setCursor(x, 30);
    display.print(String(i+1));
  }

  display.display();

  if (!digitalRead(RESET_BUTTON)) {
    cur_level = menu_lev;
    load_level(menu_lev);
  }
}

int cntr = 0;


void game_loop()
{
  if ( level_finished() ) {
    for (int i = 0; i < 64; i++){
      X_arr_x[i] = 0;
      X_arr_y[i] = 0;
    }
    
    X_iter = 0;
    cntr = 0;
    lvl_cntr++;
    game_complete();
    cur_level++;
    
    if (cur_level == END_GAME_LEVEL) {
      end_game();
    } else {
      load_level(cur_level);
    }
  }
  
  if (!digitalRead(RESET_BUTTON)) {
    for (int i = 0; i < 64; i++){
      X_arr_x[i] = 0;
      X_arr_y[i] = 0;
    }
    X_iter = 0;
    cntr = 0;
    load_level(cur_level);
  }

  if(cntr == 0)
  {
    X_arr_x[X_iter] = pl_x;
    X_arr_y[X_iter] = pl_y;
    X_iter++;
  }

  player_movement();  //aktualizuj

  display.clearDisplay();

  draw_level();
  draw_target();
  draw_player();
  draw_X();

  display.display();
  cntr++;
  
  if(cntr-1 > 0)
  {
    game_failed();
  }
}


void game_failed()
{
  bool end = false;
  for (int i = 0; i < X_iter-1; i++)
  {
    if (X_arr_x[i] == pl_x && X_arr_y[i] == pl_y)
    {
      end = true;
    }
  }

  if(end == true)
  {
    tone(piezoPin, 110, 300);
    delay(400);
    tone(piezoPin, 110, 300);
    delay(400);  
    
    display.fillRoundRect(0, 16, 128, 32, 30, WHITE);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(7, 28);
    display.println(String("poziom nieukonczony"));
    display.display();
    delay(1000);
  
    for (int i = 0; i < 64; i++){
      X_arr_x[i] = 0;
      X_arr_y[i] = 0;
    }
    X_iter = 0;
    cntr = 0;
    load_level( cur_level );
  }
}


void end_game()
{
  game_complete();
  cur_level = END_GAME_LEVEL - 1;
  menu_lev = END_GAME_LEVEL - 1;
  in_menu = true;
  lvl_cntr = 0;
  cntr = 0;

  tone(piezoPin, 523, 500);
  delay(400);
  tone(piezoPin, 587, 500);
  delay(400);
  tone(piezoPin, 659, 500);
  delay(400);
  tone(piezoPin, 698, 500);
  delay(400);
  tone(piezoPin, 784, 500);
  delay(400);
  tone(piezoPin, 880, 500);
  delay(400);
  tone(piezoPin, 988, 500);
  delay(400);
  tone(piezoPin, 1047, 1000);
  delay(400);
  
  display.fillRoundRect(0, 16, 128, 32, 30, WHITE);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(8, 25);
  display.println(String("GAME OVER"));
  display.display(); 
  
  while (digitalRead(RESET_BUTTON)) {
    delay(16);
  }
  delay(1000);
}


void game_complete()
{ 
  display.fillRoundRect(0, 16, 128, 32, 30, WHITE);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(47, 20);
  display.println(String("poziom"));
  display.setCursor(37, 35);
  display.println(String("ukonczony"));
  display.display();
  
  tone(piezoPin, 659, 300);
  delay(400);
  tone(piezoPin, 698, 300);
  delay(400);
  tone(piezoPin, 740, 300);
  delay(400);
  tone(piezoPin, 784, 500);
  delay(1500);
}


void player_movement()
{
  joy_x = (analogRead(JOY_ANALOG_X));
  joy_y = (analogRead(JOY_ANALOG_Y));

  if (pm_x == 0 && pm_y == 0) {
    if (joy_x > 1000) { pm_x = 1; } else 
    if (joy_x < 50) { pm_x = -1; } else
    if (joy_y > 1000) { pm_y = -1; } else 
    if (joy_y < 50) { pm_y = 1; }

    // sprawdz sciany
    if (!can_go(pl_x/8 + pm_x, pl_y/8 + pm_y)) {
      pm_x = 0;
      pm_y = 0;
    }
  }
}


void draw_player()
{
  if ( pm_x != 0 ) {
    pl_x += pm_x;
    if (pl_x % 8 == 0){ 
      X_arr_x[X_iter] = pl_x;
      X_arr_y[X_iter] = pl_y;
      X_iter++;
      pm_x = 0;
    }
  }
  if ( pm_y != 0 ) {
    pl_y += pm_y;
    if (pl_y % 8 == 0){
      X_arr_x[X_iter] = pl_x;
      X_arr_y[X_iter] = pl_y;
      X_iter++;
      pm_y = 0;
    }
  }
  display.drawBitmap(pl_x, pl_y, gracz, 8, 8, 1);
}

//pekniete plytki
void draw_X()
{
  for (int i = 0; i < X_iter; i++)
    {
      display.drawBitmap(X_arr_x[i], X_arr_y[i], krzyzyk, 8, 8, 1);
    }
}

//meta
void draw_target()
{
  display.drawBitmap(target_x[lvl_cntr] * 8, target_y[lvl_cntr] * 8, meta, 8, 8, 1);
}

//jesli wszystkie kroki wykonane i na pozycji mety
bool level_finished()
{
  return pl_y/8 == target_y[lvl_cntr] && pl_x/8 == target_x[lvl_cntr] && X_iter == moves_to_finish[lvl_cntr];
}

//blokuj jesli sciana
bool can_go(unsigned char x, unsigned char y)
{
  i = level[x] & (0x01 << y);
  return (i == 0);
}

//rysuj lvle z level_maps
void draw_level()
{
  for (x = 0; x < 16; x++) {
    for (y = 0; y < 8; y++) {
      i = level[x] & (0x01 << y);
      if ( i > 0 ) display.drawBitmap(x * 8, y * 8, sciana, 8, 8, 1);
    }
  }
}
