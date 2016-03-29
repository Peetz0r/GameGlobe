#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Bounce2.h>
#include <toneAC.h>
#include <EEPROM.h>
#include "pitches.h"
#include "naam.h"

#define PIN_U 2
#define PIN_S 3
#define PIN_L 4
#define PIN_R 5
#define PIN_D 6

#define PIN_LCD_DC  7
#define PIN_LCD_RST 8

#define EEPROM_VOLUME 10
#define EEPROM_SNAKE 110
#define EEPROM_TETRIS 120
#define EEPROM_FLAPPY 130

Adafruit_PCD8544 d = Adafruit_PCD8544(PIN_LCD_DC, -1, PIN_LCD_RST);

Bounce b_u = Bounce();
Bounce b_d = Bounce();
Bounce b_l = Bounce();
Bounce b_r = Bounce();
Bounce b_s = Bounce();

enum game {
	MENU,
	SNAKE,
	TETRIS,
	FLAPPY,
	HIGHSCORES,
	GAME_OVER,
};

game current_game = MENU;
game previous_game;

int8_t selected = 0;
int8_t volume = 5;
int8_t need_write_volume = 0;
uint8_t score;

uint8_t snake_length;
uint8_t snake_pushed;
uint8_t snake_pos[255];
int8_t snake_food_x;
int8_t snake_food_y;
long snake_delay;
long snake_last_frame = 0;
enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};
direction snake_direction = LEFT;
direction snake_direction_old = LEFT;

uint16_t tetris_state[10];
enum tetris_type {
	I, J, L, O, S, T, Z
};
tetris_type tetris_current_type;
tetris_type tetris_next_type;

uint8_t flappy_x;
float flappy_y;
float flappy_y_speed;
int8_t flappy_pipe;
long flappy_delay;

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
	d.print("FlappySmile");
	d.setCursor(11, 29);
	d.print("Highscores");
	d.setCursor(11, 38);

	d.print("Volume");
	d.drawRect(52,37,         24,9, 1);
	d.fillRect(54,39, (volume*2),5, 1);

	if(b_s.fell()) {
		toneAC(NOTE_A6, volume, 25);
		score = 0;
		if(selected == 0) {
			snake_length = 5;
			snake_pushed = 0;
			snake_pos[0] = 115;
			snake_pos[1] = 116;
			snake_pos[2] = 117;
			snake_pos[3] = 118;
			snake_pos[4] = 119;
			snake_delay = 350;
			snake_pushed = 0;
			snake_last_frame = 0;
			snake_direction = LEFT;
			snake_place_food();

			current_game = SNAKE;
		} else if(selected == 1) {
			tetris_state[0] = 0;
			tetris_state[1] = 0;
			tetris_state[2] = 0;
			tetris_state[3] = 0;
			tetris_state[4] = 0;
			tetris_state[5] = 0;
			tetris_state[6] = 0;
			tetris_state[7] = 0;
			tetris_state[8] = 0;
			tetris_state[9] = 0;

			current_game = TETRIS;
		} else if(selected == 2) {
			flappy_x = 0;
			flappy_y = 24;
			flappy_y_speed = 0;
			flappy_pipe = 12;
			flappy_delay = 50;

			current_game = FLAPPY;
		} else if(selected == 3) {
			current_game = HIGHSCORES;
		}
	}

	if(selected == 4) {
		if(b_l.fell()) {
			if(volume > 0) {
				volume--;
				need_write_volume = 1;
			}
			toneAC(NOTE_A6, volume, 25);
		}
		if(b_r.fell()) {
			if(volume < 10) {
				volume++;
				need_write_volume = 1;
			}
			toneAC(NOTE_A6, volume, 25);
		}
	} else if(need_write_volume == 1) {
		EEPROM.write(EEPROM_VOLUME, volume);
		need_write_volume = 0;
	}

	d.display();
	delay(25);
}

void snake_loop () {
	if(b_u.fell() && snake_direction_old != DOWN ) snake_direction = UP;
	if(b_d.fell() && snake_direction_old != UP   ) snake_direction = DOWN;
	if(b_l.fell() && snake_direction_old != RIGHT) snake_direction = LEFT;
	if(b_r.fell() && snake_direction_old != LEFT ) snake_direction = RIGHT;

	if((millis() - snake_last_frame) > snake_delay) {
		snake_direction_old = snake_direction;
		d.clearDisplay();

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


		snake_pos[0] = tmp_snake_y*21 + tmp_snake_x;

		if(tmp_snake_x == snake_food_x && tmp_snake_y == snake_food_y) {
			snake_length++;
			snake_place_food();
			snake_delay *= 0.95;

			toneAC(NOTE_G5, volume, 40); delay(40);
			toneAC(NOTE_G6, volume, 40); delay(40);
		} else {
			toneAC(NOTE_A3, volume, 5);
		}

		for(uint8_t i = 0; i < snake_length; i++) {
			d.fillRoundRect((snake_pos[i]%21)*4, (snake_pos[i]/21)*4, 4, 4, 1, 1);
			if(i > 0 && tmp_snake_x == snake_pos[i]%21 && tmp_snake_y == snake_pos[i]/21) {
				score = snake_length;
				previous_game = SNAKE;
				game_over();
			}
		}
		d.drawRoundRect(snake_food_x*4, snake_food_y*4, 4, 4, 2, 1);
		d.display();

		snake_last_frame = millis();
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

void tetris_loop () {
	d.clearDisplay();
	d.drawRect(26, -1, 31, 49, 1);
	for(int x = 0; x < 10; x++) {
		for(int y = 0; y < 16; y++) {
			if(tetris_state[x] & (1 << y)) {
				d.fillRect(27+x*3, y*3, 2, 2, 1);
			}
		}
	}
	d.display();
}

void flappy_loop () {
	flappy_y += flappy_y_speed;

	// generate new pipes once in a while
	if(flappy_x > 120) {
		flappy_x = 0;
		flappy_pipe = random(24);
	}

	// change vertical acceleration over time
	if(flappy_y_speed < 0) {
		// when moving up, decelerate quickly
		flappy_y_speed += 0.2;
	} else {
		// when falling down, accelerate less quickly
		flappy_y_speed += 0.075;
	}

	// (re)set vertical acceleration on button push
	if(b_u.fell()) {
		flappy_y_speed = -2;
	}

	// collision checking
	if(flappy_y > 47 ||
	  ((flappy_x > 52 && flappy_x < 68) &&
	  (flappy_y < flappy_pipe+8 || flappy_y > flappy_pipe+20))) {
		previous_game = FLAPPY;
		game_over();
	}

	if(flappy_x == 70) {
		score++;
		flappy_delay *= 0.975;
	}

	flappy_x++;

	d.clearDisplay();

	// pipes, narrow part
	d.fillRect(100-flappy_x, 0, 10, flappy_pipe, 1); // upper
	d.fillRect(100-flappy_x, flappy_pipe+28, 10, 100, 1); // lower

	// pipes, wide part (edge)
	d.fillRect(100-flappy_x-2, flappy_pipe-2, 14, 4, 1); // upper
	d.fillRect(100-flappy_x-2, flappy_pipe+26, 14, 4, 1); // lower

	d.drawCircle(42, flappy_y-1, 5, 1); // face
	d.fillRect(39, flappy_y-3, 2, 2, 1); // left eye
	d.fillRect(44, flappy_y-3, 2, 2, 1); // right eye
	d.drawLine(39, flappy_y, 41, flappy_y+2, 1); // left part of smile
	d.drawPixel(42, flappy_y+2, 1); // center part of smile
	d.drawLine(43, flappy_y+2, 45, flappy_y, 1); // right part of smile
	d.display();

	delay(flappy_delay);
}

void game_over() {
	current_game = GAME_OVER;

	toneAC(NOTE_G6, volume, 50);
	toneAC(NOTE_F6, volume, 50);
	toneAC(NOTE_E6, volume, 50);
	toneAC(NOTE_D6, volume, 50);
	toneAC(NOTE_C6, volume, 50);
	toneAC(NOTE_B6, volume, 50);
	toneAC(NOTE_A6, volume, 50);
	toneAC(NOTE_G5, volume, 50);
	toneAC(NOTE_F5, volume, 50);
	toneAC(NOTE_E5, volume, 50);
	toneAC(NOTE_D5, volume, 50);
	toneAC(NOTE_C5, volume, 50);
	toneAC(NOTE_B5, volume, 50);
	toneAC(NOTE_A5, volume, 50);
	toneAC(NOTE_G4, volume, 50);
	toneAC(NOTE_F4, volume, 50);
	toneAC(NOTE_E4, volume, 50);
	toneAC(NOTE_D4, volume, 50);
	toneAC(NOTE_C4, volume, 50);
	toneAC(NOTE_B4, volume, 50);
	toneAC(NOTE_A4, volume, 50);
	toneAC(NOTE_G3, volume, 50);
	toneAC(NOTE_F3, volume, 50);
	toneAC(NOTE_E3, volume, 50);
	toneAC(NOTE_D3, volume, 50);
	toneAC(NOTE_C3, volume, 50);
	toneAC(NOTE_B3, volume, 50);
	toneAC(NOTE_A3, volume, 50);
	toneAC(NOTE_G2, volume, 50);
	toneAC(NOTE_F2, volume, 50);
	toneAC(NOTE_E2, volume, 50);
	toneAC(NOTE_D2, volume, 50);
	toneAC(NOTE_C2, volume, 50);
	toneAC(NOTE_B2, volume, 50);
	toneAC(NOTE_A2, volume, 50);
	toneAC(NOTE_G1, volume, 50);
	toneAC(NOTE_F1, volume, 50);
	toneAC(NOTE_E1, volume, 50);
	toneAC(NOTE_D1, volume, 50);
	toneAC(NOTE_C1, volume, 50);
	toneAC(NOTE_B1, volume, 50);
	toneAC(NOTE_A1, volume, 50);
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
		case SNAKE:       address = EEPROM_SNAKE;       break;
		case TETRIS:      address = EEPROM_TETRIS;      break;
		case FLAPPY: address = EEPROM_FLAPPY; break;
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

	uint8_t highscore;

	d.setCursor(0, 0);
	d.print("High scores:");

	d.setCursor(0, 16);
	d.print("Snake");
	highscore = EEPROM.read(EEPROM_SNAKE);
	if(highscore < 10) {
		d.setCursor(78, 16);
		d.print(highscore);
	} else if(highscore < 100) {
		d.setCursor(72, 16);
		d.print(highscore);
	} else if(highscore < 255) {
		d.setCursor(66, 16);
		d.print(highscore);
	}

	d.setCursor(0, 26);
	d.print("Tetris");
	highscore = EEPROM.read(EEPROM_TETRIS);
	if(highscore < 10) {
		d.setCursor(78, 26);
		d.print(highscore);
	} else if(highscore < 100) {
		d.setCursor(72, 26);
		d.print(highscore);
	} else if(highscore < 255) {
		d.setCursor(66, 26);
		d.print(highscore);
	}

	d.setCursor(0, 36);
	d.print("Flappy");
	highscore = EEPROM.read(EEPROM_FLAPPY);
	if(highscore < 10) {
		d.setCursor(78, 36);
		d.print(highscore);
	} else if(highscore < 100) {
		d.setCursor(72, 36);
		d.print(highscore);
	} else if(highscore < 255) {
		d.setCursor(66, 36);
		d.print(highscore);
	}

	d.display();

	if(b_s.fell()) current_game = MENU;

	delay(25);
}

void setup() {
	randomSeed(analogRead(A0));

	volume = EEPROM.read(EEPROM_VOLUME);
	if(volume >10) {
		volume = 5;
	}

	toneAC(NOTE_A4, volume, 50);

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
	d.setCursor(30, 2);
	d.print("Deze");
	d.setCursor(15, 10);
	d.print("GameGlobe");
	d.setCursor(21, 18);
	d.print("is van:");
	if(sizeof(NAAM) <= 8) {
		d.setTextSize(2);
		d.setCursor(int(42-(sizeof(NAAM)-1)*6), 32);
		d.print(NAAM);
		d.setTextSize(1);
	} else {
		d.setCursor(int(42-(sizeof(NAAM)-1)*3), 36);
		d.print(NAAM);
	}
	d.display();

	toneAC(NOTE_A5, volume, 100);
	delay(300);

	toneAC(NOTE_A5, volume, 50);
	delay(100);

	toneAC(NOTE_A5, volume, 200);
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
	} else if(current_game == TETRIS) {
		tetris_loop();
	} else if(current_game == FLAPPY) {
		flappy_loop();
	} else if(current_game == HIGHSCORES) {
		highscores_loop();
	} else if(current_game == GAME_OVER) {
		game_over_loop();
	}
}
