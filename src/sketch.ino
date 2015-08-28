#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(5, 0, 3);

// these pin numbers are for the Pro Mini breadboard version
// the Nano PCB version has different numbers
#define PIN_L 8
#define PIN_D 7
#define PIN_R 6
#define PIN_U 4
#define PIN_S 9

void setup()
{
	pinMode(DPAD_L, INPUT_PULLUP);
	pinMode(DPAD_R, INPUT_PULLUP);
	pinMode(DPAD_D, INPUT_PULLUP);
	pinMode(DPAD_U, INPUT_PULLUP);
	pinMode(DPAD_S, INPUT_PULLUP);

	display.begin();
	display.setContrast(55);
	display.clearDisplay();
}


void loop()
{

}
