#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Bounce2.h>
#include "splash_logo.h"
#include <EEPROM.h>

// these pin numbers are for the Pro Mini breadboard version
// the Nano PCB version has different numbers
#define PIN_L 8
#define PIN_D 7
#define PIN_R 6
#define PIN_U 4
#define PIN_S 9

#define EEPROM_SNAKE 0
#define EEPROM_TETRIS 1
#define EEPROM_FLAPPY_BIRD 2

Adafruit_PCD8544 d = Adafruit_PCD8544(5, 0, 3);
Bounce b_u = Bounce();
Bounce b_d = Bounce();
Bounce b_l = Bounce();
Bounce b_r = Bounce();
Bounce b_s = Bounce();

enum game {
	MENU,
	SNAKE,
	TETRIS,
	FLAPPY_BIRD,
	HIGHSCORES,
	GAME_OVER,
};

game current_game = MENU;
game previous_game;

int8_t selected = 0;

uint8_t snake_length = 5;
uint8_t snake_pos[255] = {115,116,117,118,119};
long snake_delay = 500;
long snake_last_frame = 0;
enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};
direction snake_direction = LEFT;

byte score;

int8_t snake_food_x;
int8_t snake_food_y;

void menu_loop() {
	d.clearDisplay();
	if(b_u.fell()) selected--;
	if(b_d.fell()) selected++;
	if(selected > 4) selected = 0;
	if(selected < 0) selected = 4;

	d.drawRoundRect(0,selected*9, 80,11, 5, 1);
	d.drawLine(1,selected*9+5, 6,selected*9+5, 1);
	d.fillTriangle(6,selected*9+3, 6,selected*9+7, 8,selected*9+5, 1);

	d.setCursor(11, 2);
	d.print("Snake");
	d.setCursor(11, 11);
	d.print("Tetris");
	d.setCursor(11, 20);
	d.print("Flappy Bird");
	d.setCursor(11, 29);
	d.print("Highscores");
	d.setCursor(11, 38);
	d.print("...");

	if(b_s.fell()) {
		if(selected == 0) {
			snake_length = 5;
			snake_pos[0] = 115;
			snake_pos[1] = 116;
			snake_pos[2] = 117;
			snake_pos[3] = 118;
			snake_pos[4] = 119;
			snake_delay = 500;
			snake_last_frame = 0;
			snake_direction = LEFT;

			current_game = SNAKE;
		} else if(selected == 1) {
			current_game = TETRIS;
		} else if(selected == 2) {
			current_game = FLAPPY_BIRD;
		} else if(selected == 3) {
			current_game = HIGHSCORES;
		}
	}
	d.display();
	delay(25);
}

void snake_loop () {
	if(b_u.fell() && snake_direction != DOWN ) snake_direction = UP;
	if(b_d.fell() && snake_direction != UP   ) snake_direction = DOWN;
	if(b_l.fell() && snake_direction != RIGHT) snake_direction = LEFT;
	if(b_r.fell() && snake_direction != LEFT ) snake_direction = RIGHT;

	if((millis() - snake_last_frame) > snake_delay) {
		d.clearDisplay();
		snake_last_frame = millis();

		for(uint8_t i = snake_length+1; i > 0; i--) {
			snake_pos[i] = snake_pos[i-1];
		}

		int8_t tmp_snake_x;
		int8_t tmp_snake_y;

		switch(snake_direction) {
			case UP:
				tmp_snake_x = (snake_pos[1]%21);
				tmp_snake_y = (snake_pos[1]/21)-1;
				break;
			case DOWN:
				tmp_snake_x = (snake_pos[1]%21);
				tmp_snake_y = (snake_pos[1]/21)+1;
				break;
			case LEFT:
				tmp_snake_x = (snake_pos[1]%21)-1;
				tmp_snake_y = (snake_pos[1]/21);
				break;
			case RIGHT:
				tmp_snake_x = (snake_pos[1]%21)+1;
				tmp_snake_y = (snake_pos[1]/21);
				break;
		}

		if(tmp_snake_x <  0) tmp_snake_x = 20;
		if(tmp_snake_x > 20) tmp_snake_x =  0;
		if(tmp_snake_y <  0) tmp_snake_y = 11;
		if(tmp_snake_y > 11) tmp_snake_y =  0;

		if(tmp_snake_x == snake_food_x && tmp_snake_y == snake_food_y) {
			snake_length++;
			snake_place_food();
			snake_delay *= 0.98;
		}

		snake_pos[0] = tmp_snake_y*21 + tmp_snake_x;

		for(uint8_t i = 0; i < snake_length; i++) {
			d.fillRoundRect((snake_pos[i]%21)*4, (snake_pos[i]/21)*4, 4, 4, 1, 1);
			if(i > 0 && tmp_snake_x == snake_pos[i]%21 && tmp_snake_y == snake_pos[i]/21) {
				score = snake_length;
				previous_game = SNAKE;
				current_game = GAME_OVER;
			}
		}
		d.drawRoundRect(snake_food_x*4, snake_food_y*4, 4, 4, 2, 1);

		d.display();
	}
}

void snake_place_food() {
	boolean done = false;
	while(!done) {
		snake_food_x = random(20);
		snake_food_y = random(11);
		done = true;
		for(uint8_t i = 0; i < snake_length; i++) {
			if(snake_food_x == snake_pos[i]%21 && snake_food_y == snake_pos[i]/21) {
				done = false;
			}
		}
	}
}

void game_over_loop() {
	d.clearDisplay();
	d.setCursor(0, 0);
	d.setTextSize(2);
	d.print("GAME");
	d.setCursor(36, 12);
	d.print("OVER");

	d.setTextSize(1);
	d.setCursor(0, 30);
	d.print("Your score");
	d.setCursor(66, 30);
	d.print(score);

	int address;

	switch(previous_game) {
		case SNAKE: address = EEPROM_SNAKE; break;
	}

	byte highscore = EEPROM.read(address);
	if(score > highscore || highscore == 255) {
		highscore = score;
		EEPROM.write(address, highscore);
	}

	d.setCursor(0, 40);
	d.print("High score");
	d.setCursor(66, 40);
	d.print(highscore);

	d.display();

	if(b_s.fell()) current_game = MENU;

	delay(25);
}

void highscores_loop() {
	d.clearDisplay();

	byte highscore;

	d.setCursor(0, 0);
	d.print("High scores:");

	d.setCursor(0, 16);
	d.print("Snake");

	highscore = EEPROM.read(EEPROM_SNAKE);
	if(highscore != 255) {
		d.setCursor(66, 16);
		d.print(highscore);
	}

	d.setCursor(0, 26);
	d.print("Tetris");
	highscore = EEPROM.read(EEPROM_TETRIS);
	if(highscore != 255) {
		d.setCursor(66, 26);
		d.print(highscore);
	}

	d.setCursor(0, 36);
	d.print("Flappy");
	highscore = EEPROM.read(EEPROM_FLAPPY_BIRD);
	if(highscore != 255) {
		d.setCursor(66, 36);
		d.print(highscore);
	}

	d.display();

	if(b_s.fell()) current_game = MENU;

	delay(25);
}

void setup() {
	randomSeed(analogRead(A0));
	snake_place_food();

	pinMode(PIN_L, INPUT_PULLUP);
	pinMode(PIN_R, INPUT_PULLUP);
	pinMode(PIN_D, INPUT_PULLUP);
	pinMode(PIN_U, INPUT_PULLUP);
	pinMode(PIN_S, INPUT_PULLUP);

	b_u.attach(PIN_U);
	b_d.attach(PIN_D);
	b_l.attach(PIN_L);
	b_r.attach(PIN_R);
	b_s.attach(PIN_S);

	b_u.interval(20);
	b_d.interval(20);
	b_l.interval(20);
	b_r.interval(20);
	b_s.interval(20);

	d.begin();
	d.setContrast(55);

	d.clearDisplay();
	d.drawXBitmap(0,0, splash_logo, 84,48, 1);
	d.display();
	delay(2000);
}

void loop() {
	b_u.update();
	b_d.update();
	b_l.update();
	b_r.update();
	b_s.update();

	if(current_game == MENU) {
		menu_loop();
	} else if(current_game == SNAKE) {
		snake_loop();
	} else if(current_game == HIGHSCORES) {
		highscores_loop();
	} else if(current_game == GAME_OVER) {
		game_over_loop();
	}
}
