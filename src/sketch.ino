#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Bounce2.h>
#include "splash_logo.h"

// these pin numbers are for the Pro Mini breadboard version
// the Nano PCB version has different numbers
#define PIN_L 8
#define PIN_D 7
#define PIN_R 6
#define PIN_U 4
#define PIN_S 9

Adafruit_PCD8544 d = Adafruit_PCD8544(5, 0, 3);
Bounce b_u = Bounce();
Bounce b_d = Bounce();
Bounce b_l = Bounce();
Bounce b_r = Bounce();
Bounce b_s = Bounce();

enum screens {
	MENU,
	SNAKE,
	TETRIS,
	FLAPPY_BIRD,
	HIGHSCORES
};

screens current_screen = MENU;

int8_t selected = 0;

int8_t snake_length = 5;
int8_t snake_x[255] = {10,11,12,13,14};
int8_t snake_y[255] = {5,5,5,5,5};
long snake_delay = 500;
long snake_last_frame = 0;
enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};
direction snake_direction = LEFT;


int8_t snake_fruit_x;
int8_t snake_fruit_y;

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

	if(b_s.rose()) {
		if(selected == 0) {
			current_screen = SNAKE;
		} else if(selected == 1) {
			current_screen = TETRIS;
		} else if(selected == 2) {
			current_screen = FLAPPY_BIRD;
		} else if(selected == 3) {
			current_screen = HIGHSCORES;
		}
	}
	d.display();
}

void snake_loop () {
	if(b_u.rose() && snake_direction != DOWN ) snake_direction = UP;
	if(b_d.rose() && snake_direction != UP   ) snake_direction = DOWN;
	if(b_l.rose() && snake_direction != RIGHT) snake_direction = LEFT;
	if(b_r.rose() && snake_direction != LEFT ) snake_direction = RIGHT;

	if(millis() - snake_last_frame > snake_delay) {
		d.clearDisplay();
		snake_last_frame = millis();

		for(int8_t i = snake_length; i >= 0; i--) {
			snake_x[i+1] = snake_x[i];
			snake_y[i+1] = snake_y[i];
		}

		switch(snake_direction) {
			case UP:
				snake_x[0] = snake_x[1];
				snake_y[0] = snake_y[1]-1;
				break;
			case DOWN:
				snake_x[0] = snake_x[1];
				snake_y[0] = snake_y[1]+1;
				break;
			case LEFT:
				snake_x[0] = snake_x[1]-1;
				snake_y[0] = snake_y[1];
				break;
			case RIGHT:
				snake_x[0] = snake_x[1]+1;
				snake_y[0] = snake_y[1];
				break;
		}

		if(snake_x[0] <  0) snake_x[0] = 20;
		if(snake_x[0] > 20) snake_x[0] =  0;
		if(snake_y[0] <  0) snake_y[0] = 11;
		if(snake_y[0] > 11) snake_y[0] =  0;

		if(snake_x[0] == snake_fruit_x && snake_y[0] == snake_fruit_y) {
			snake_length++;
			snake_plaats_fruit();
			snake_delay *= 0.95;
			snake_delay = 250;
		}

		// fixme: game freezes after eating a number of fruits
		// most likely out-of-memory issue because snake_x/y[] are huge
		d.setCursor(0,0);
		d.print(snake_length);
		d.setCursor(20,0);
		d.print(snake_delay);

		for(int8_t i = 0; i < snake_length; i++) {
			d.fillRoundRect(snake_x[i]*4, snake_y[i]*4, 4, 4, 1, 1);
			if(i > 0 && snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
				// todo, write proper game-over code.
				asm volatile ("  jmp 0");
			}
		}

		d.fillRect(snake_fruit_x*4, snake_fruit_y*4, 4, 4, 1);
		d.display();
	}
}

void snake_plaats_fruit() {
	boolean done = false;
	while(!done) {
		snake_fruit_x = random(20);
		snake_fruit_y = random(11);
		done = true;
		for(int8_t i = 0; i < snake_length; i++) {
			if(snake_fruit_x == snake_x[i]) done = false;
			if(snake_fruit_y == snake_y[i]) done = false;
		}
	}
}

void setup() {
	randomSeed(analogRead(A0));
	snake_plaats_fruit();

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
	delay(25);

	b_u.update();
	b_d.update();
	b_l.update();
	b_r.update();
	b_s.update();

	if(current_screen == MENU) {
		menu_loop();
	} else if(current_screen == SNAKE) {
		snake_loop();
	}
}
