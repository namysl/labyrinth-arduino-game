#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>

#define JOYSTICK_X 1
#define JOYSTICK_Y 0
#define JOYSTICK_BUTTON 2

#define END_GAME_LEVEL 5

#define OLED_RESET 4
#define SCREEN_WIDTH 128 // pixels
#define SCREEN_HEIGHT 64

// SSD1306 display I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int buzzer_pin = 7; // piezo buzzer

// melody[0] -> duration
int melody_goodluck[] = {523, 587, 523};
int melody_failed[] = {110, 110};
int melody_completed[] = {659, 698, 740, 784};
int melody_gameover[] = {523, 587, 659, 698, 784, 880, 988, 1047};


bool in_menu = true;

int target_x[] = {10, 5, 7, 9, 10};  // marked tile/goal's x axis position on levels
int target_y[] = {1, 1, 1, 4, 4};    // marked tile/goal's y axis position on levels

int moves_to_finish[] = {36, 36, 30, 30, 32}; // on each level from 1 to 5

int i;

// i should clean this mess below

int lvl_cntr = 0;
int cntr = 0;

unsigned char x;
unsigned char y;

int joy_x = 0;
int joy_y = 0;
int pl_x = 8;
int pl_y = 8;
int pm_x = 0;
int pm_y = 0;
byte current_level = 0;
int menu_pos = 0;
char menu_dir = 0;
byte menu_lev = 0;

int X_iter = 0;
int X_arr_x[64];
int X_arr_y[64];

// i should clean this mess above...

unsigned char levelx[] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0x81, 0x81, 
                          0x81, 0x81, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00};

struct LVL {
  byte px; // player's position on x
  byte py; // player's position on y
  byte l;  // level to load
};

static const LVL levels [] PROGMEM = {{5, 1, 0},  // 1 lvl
                                      {6, 1, 0},  // 2
                                      {5, 6, 1},  // 3
                                      {10, 6, 2}, // 4
                                      {5, 6, 3}   // 5
};

static const unsigned char level_maps_layout [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00, // 1 and 2 lvl
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xe1, 0x8d, 0x81, 0x81, 0xe1, 0xFF, 0x00, 0x00, 0x00, 0x00, // 3
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0x8d, 0xa1, 0xa1, 0x8d, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00, // 4
  0x00, 0x00, 0x00, 0x00, 0xFF, 0x81, 0xc9, 0x81, 0xb1, 0x81, 0x81, 0xFF, 0x00, 0x00, 0x00, 0x00  // 5
};

// graphics in hex
static const unsigned char PROGMEM cross[] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};
static const unsigned char PROGMEM wall[] = {0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa};
static const unsigned char PROGMEM player[] = {0x00, 0x3c, 0x5a, 0x7e, 0x5a, 0x66, 0x3c, 0x00};
static const unsigned char PROGMEM target[] = {0x00, 0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18};
static const unsigned char PROGMEM arrow[] = {0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x03, 0xc0,
                                              0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0,
                                              0x03, 0xc0, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0,
                                              0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00};


void setup() {
  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop() {
  if (in_menu) {
    menu_loop();
  } 
  else {
    game_loop();
  }
}


void menu_loop() {
  joy_x = (analogRead(JOYSTICK_X) - 512) / 100;

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
  display.setCursor(27, 52);
  display.print(String("select level"));
  display.drawBitmap(55, 2, arrow, 16, 16, 1);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  
  // 1 lvl -> 5 lvl
  for (i = 0; i < END_GAME_LEVEL; i++) {  
    int x = 64 - (i+1 < 10 ? 7 : 16) - menu_pos + i * 48;
    if (x < - 24 || x > 152) continue;
    display.setCursor(x, 30);
    display.print(String(i+1));
  }

  display.display();

  if (!digitalRead(JOYSTICK_BUTTON)) {
    current_level = menu_lev;
    load_level(menu_lev);
  }
}


void load_level(byte l) {
  in_menu = false;

  display.clearDisplay();
  display.fillRoundRect(0, 20, 128, 32, 30, WHITE);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(11, 28);
  display.print(String("good luck"));
  display.display();

  buzzer_alert(melody_goodluck, 1500, 2);
  delay(1000);
  
  // load level: program memory -> ram
  LVL lvl;
  memcpy_P(&lvl, &levels[l], sizeof(LVL));
  
  // load level map: progmem -> ram
  memcpy_P(&levelx, &level_maps_layout[lvl.l * 16], 16);

  pl_x = lvl.px * 8;  // player's position
  pl_y = lvl.py * 8;
  pm_x = pm_y = 0;
  delay(1000);
}


void game_loop() {

  if (level_finished()) {
    for (int i = 0; i < 64; i++){
      X_arr_x[i] = 0;
      X_arr_y[i] = 0;
    }
    
    X_iter = 0;
    cntr = 0;
    lvl_cntr++;
    game_complete();
    current_level++;
    
    if (current_level == END_GAME_LEVEL) {
      end_game();
    } 
    else {
      load_level(current_level);
    }
  }
  
  if (!digitalRead(JOYSTICK_BUTTON)) {
    for (int i = 0; i < 64; i++){
      X_arr_x[i] = 0;
      X_arr_y[i] = 0;
    }
    X_iter = 0;
    cntr = 0;
    load_level(current_level);
  }

  if(cntr == 0) {
    X_arr_x[X_iter] = pl_x;
    X_arr_y[X_iter] = pl_y;
    X_iter++;
  }

  player_movement();  // update the moves

  display.clearDisplay();

  draw_level();
  draw_target();
  draw_player();
  draw_X();

  display.display();
  cntr++;
  
  if(cntr-1 > 0) {
    game_failed();
  }
}


void game_failed() {

  bool end = false;
  for (int i = 0; i < X_iter-1; i++) {
    if (X_arr_x[i] == pl_x && X_arr_y[i] == pl_y)
    {
      end = true;
    }
  }

  if (end == true) {
      buzzer_alert(melody_failed, 300, 1);

    display.fillRoundRect(0, 16, 128, 32, 30, WHITE);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.setCursor(28, 28);
    display.println(String("level failed"));
    display.display();
    delay(1000);
  
    for (int i = 0; i < 64; i++) {
      X_arr_x[i] = 0;
      X_arr_y[i] = 0;
    }

    X_iter = 0;
    cntr = 0;
    load_level(current_level);
  }
}


void game_complete() {   
  display.fillRoundRect(0, 16, 128, 32, 30, WHITE);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(47, 20);
  display.println(String("level"));
  display.setCursor(37, 35);
  display.println(String("completed"));
  display.display();
  
buzzer_alert(melody_completed, 300, (sizeof(melody_completed)/sizeof(melody_completed[0]))-1);
}


void end_game() {
  game_complete();
  current_level = END_GAME_LEVEL - 1;
  menu_lev = END_GAME_LEVEL - 1;
  in_menu = true;
  lvl_cntr = 0;
  cntr = 0;

buzzer_alert(melody_gameover, 500, (sizeof(melody_gameover)/sizeof(melody_gameover[0]))-1);
  
  display.fillRoundRect(0, 16, 128, 32, 30, WHITE);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(8, 25);
  display.println(String("GAME OVER"));
  display.display(); 
  
  while (digitalRead(JOYSTICK_BUTTON)) {
    delay(16);
    menu_loop();
  }
  delay(1000);
}


void player_movement() {
  joy_x = (analogRead(JOYSTICK_X));
  joy_y = (analogRead(JOYSTICK_Y));

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


void draw_player() {
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
  display.drawBitmap(pl_x, pl_y, player, 8, 8, 1);
}


void draw_X() {
  // broken tiles
  for (int i = 0; i < X_iter; i++)
    {
      display.drawBitmap(X_arr_x[i], X_arr_y[i], cross, 8, 8, 1);
    }
}


void draw_target() {
  // marked tile/goal
  display.drawBitmap(target_x[lvl_cntr] * 8, target_y[lvl_cntr] * 8, target, 8, 8, 1);
}


bool level_finished() {
  // checks if the player finished the level with the given number of steps and stands on the goal tile
  return pl_y/8 == target_y[lvl_cntr] && pl_x/8 == target_x[lvl_cntr] && X_iter == moves_to_finish[lvl_cntr];
}


bool can_go(unsigned char x, unsigned char y) {
  // block the way if there is a wall
  i = levelx[x] & (0x01 << y);
  return (i == 0);
}


void draw_level() {
  for (x = 0; x < 16; x++) {
    for (y = 0; y < 8; y++) {
      i = levelx[x] & (0x01 << y);
      if (i > 0) display.drawBitmap(x * 8, y * 8, wall, 8, 8, 1);
    }
  }
}


void buzzer_alert(int melody[], int duration, int len_arr) {
    for (int d = 0; d <= len_arr; d++) {
        tone(buzzer_pin, melody[d], duration);
        delay(400);
  }
  delay(500);
}
