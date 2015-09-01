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
	menu,
	snake,
	tetris,
	flappy_bird,
	highscores
};

screens current_screen = menu;

int selected = 0;

void setup()
{
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


void loop()
{
	delay(50);

	d.clearDisplay();
	b_u.update();
	b_d.update();
	b_l.update();
	b_r.update();
	b_s.update();

	if(current_screen == menu) {

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

		d.display();

		if(b_s.rose()) {
			if(selected == 0) {
				current_screen = snake;
			} else if(selected == 1) {
				current_screen = tetris;
			} else if(selected == 2) {
				current_screen = flappy_bird;
			} else if(selected == 3) {
				current_screen = highscores;
			}
		}
	} else if(current_screen == snake) {

	}
}
