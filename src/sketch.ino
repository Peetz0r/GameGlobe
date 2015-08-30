#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Bounce2.h>

Adafruit_PCD8544 d = Adafruit_PCD8544(5, 0, 3);
Bounce b_u = Bounce();
Bounce b_d = Bounce();
Bounce b_l = Bounce();
Bounce b_r = Bounce();
Bounce b_s = Bounce();

// these pin numbers are for the Pro Mini breadboard version
// the Nano PCB version has different numbers
#define PIN_L 8
#define PIN_D 7
#define PIN_R 6
#define PIN_U 4
#define PIN_S 9

int selected = 2;

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

	b_u.interval(50);
	b_d.interval(50);
	b_l.interval(50);
	b_r.interval(50);
	b_s.interval(50);

	d.begin();
	d.setContrast(55);
}


void loop()
{
	b_u.update();
	b_d.update();
	b_l.update();
	b_r.update();
	b_s.update();

	if(b_u.rose()) selected--;
	if(b_d.rose()) selected++;
	if(selected > 4) selected = 0;
	if(selected < 0) selected = 4;

	d.clearDisplay();
	d.drawRoundRect(0,0, 84,48, 10, 1);

	d.drawRoundRect(4,selected*9, 76,11, 5, 1);
	d.drawLine(5,selected*9+5, 10,selected*9+5, 1);
	d.fillTriangle(10,selected*9+3, 10,selected*9+7, 12,selected*9+5, 1);

	d.setCursor(16, 2);
	d.print("fqgjABC 1");
	d.setCursor(16, 11);
	d.print("fqgjABC 2");
	d.setCursor(16, 20);
	d.print("fqgjABC 3");
	d.setCursor(16, 29);
	d.print("fqgjABC 4");
	d.setCursor(16, 38);
	d.print("fqgjABC 5");

	d.display();
}
