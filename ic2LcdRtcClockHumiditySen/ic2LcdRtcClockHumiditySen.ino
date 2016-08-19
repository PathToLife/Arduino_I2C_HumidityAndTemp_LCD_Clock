/*
* ic2LcdRtcClockHumiditySen.ino
*
* Created: 6/18/2015 12:40:27 AM
* Author: Sean Z
*
* Edited for a bigger screen 19/08/2016
*	single Screen as opposed to 2 screens
*/

#include <stdio.h>
#include <DS1302.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>

// Remove this if you have a bread board on hand to power the RTC LOL 19/08/2016 In ECE labs #owned
#define _powerPin 13
#define _powerPin 7

// Set the appropriate digital I/O pin connections.
// Data sheet: http://datasheets.maximintegrated.com/en/ds/DS1302.pdf
#define rtcCePin 10  // RTC Chip Enable
#define rtcIoPin 11  // RTC Input/Output
#define rtcSclkPin 12  // RTC Serial Clock

// Create a DS1302 object.
DS1302 rtc(rtcCePin, rtcIoPin, rtcSclkPin);

// Create LCD object 16 x 2 with address
// A1 0 = 3D, A0 0 = 3E, 000 = 27, Gussing A2 0 = 3C
LiquidCrystal_I2C lcd1(0x3c, 20, 4);

// LCD Custom Char for RAM storage
uint8_t smileGoate[8] = { 0x0, 0xA, 0xA, 0x0, 0x11, 0x11, 0xE, 0x4 };
uint8_t smileHair[8] = { 0x1B, 0x11, 0x0, 0xA, 0xA, 0x0, 0x11, 0xE };
uint8_t tempSymbol[8] = { 0x4, 0xA, 0xA, 0xA, 0xE, 0x1F, 0x1F, 0xE };
uint8_t humidSymbol[8] = { 0x4, 0x4, 0xA, 0xA, 0x11, 0x11, 0x11, 0xE };
uint8_t leftBoarder[8] = { 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F };
uint8_t rightBoarder[8] = { 0x1F, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1F };

// Set DHT Digital Humidity Temperature
DHT dht(3, DHT11);

String dayAsString(const Time::Day day) {
	switch (day) {
	case Time::kSunday: return "Sun";
	case Time::kMonday: return "Mon";
	case Time::kTuesday: return "Tues";
	case Time::kWednesday: return "Wed";
	case Time::kThursday: return "Thurs";
	case Time::kFriday: return "Fri";
	case Time::kSaturday: return "Sat";
	}
	return "(unknown Time)";
}

void setup() {
	Serial.begin(9600);

	#if defined(_powerpin)
	pinMode(_powerPin, OUTPUT);
	digitalWrite(_powerPin, HIGH);
	#endif
#if defined(_powerpin2)
	pinMode(_powerPin2, OUTPUT);
	digitalWrite(_powerPin2, HIGH);
#endif

	// Initialize a new chip These methods needn't always be called.
	rtc.writeProtect(false);
	rtc.halt(false);

	// Start LCD functions
	initializeLCD();

	lcd1.createChar(0, smileGoate);
	lcd1.createChar(1, tempSymbol);
	lcd1.createChar(2, humidSymbol);
	lcd1.createChar(3, smileHair);

	lcd1.createChar(4, leftBoarder);
	lcd1.createChar(5, rightBoarder);

	// Start Temp Sensor
	dht.begin();

	// Set the date and time. t(yr, mth, day, hr, min, sec, day)
	// Time t(2015, 6, 17, 23, 40, 30, Time::kWednesday);  <---------------------------- WRITE TIME
	// rtc.time(t);
	Serial.println("Ready for Commands");
}


// Millis
unsigned long previousClockMillis = 0;
unsigned long previousSerialMillis = 0;
unsigned long previousTempMillis = 0;
// Other
char timeString[50];
Time t = rtc.time();
// Serial Loop T/F
boolean lcd_blink = false;
boolean clockON = true;
boolean serialClockON = false;
boolean timeColumnON = false;
boolean serialTempON = false;
// boolean firstRunClock = true;

// Temps
float humid, temp, farht, hIndx = 0;

uint8_t dispTime[4] = { 00, 00, 00, 00 }; // hours, min, sec, day


void initializeLCD() {
	// initialize LCD
	lcd1.begin();

	// Turn On backlight and print "Hello World:
	lcd1.backlight();
	lcd1.print("Hello, World");
	lcd1.setCursor(0, 1);
	lcd1.print("This is LCD 1");

	delay(500);

	for (int i = 3; i >= 0; i--) {
		lcd1.setCursor(20, 1);
		lcd1.print(i);
		delay(100);
	}

	if (lcd_blink) {
		lcd1.blink();
	}
}

// Loop and print the time every second, with error correction 'buffer' that is reset every time the second changes.
void loop() {

	unsigned long currentMillis = millis();

	// Clock Disp
	if (clockON && ((currentMillis - previousClockMillis) > 990)) {

		// Buffer. If FALSE: previousMillis is not written, thus will check again on next void loop() ~ 1ms;
		if (rtc.time().sec != t.sec) {

			printTime(); // Get time, store, serial print.
			int xCor = 4;
			if (dispTime[0] != t.hr) {
				if (t.hr < 10) {
					lcd1.setCursor(xCor, 1);
					lcd1.print("0");
					lcd1.setCursor(xCor + 1, 1);
				}
				else {
					lcd1.setCursor(xCor, 1);
				}
				lcd1.print(t.hr);
				dispTime[0] = t.hr;
			}
			if (dispTime[1] != t.min) {
				if (t.min < 10) {
					lcd1.setCursor(xCor + 3, 1);
					lcd1.print("0");
					lcd1.setCursor(xCor + 4, 1);
				}
				else {
					lcd1.setCursor(xCor + 3, 1);
				}
				lcd1.print(t.min);
				dispTime[1] = t.min;
			}
			if (dispTime[2] != t.sec) {
				if (t.sec < 10) {
					lcd1.setCursor(xCor + 6, 1);
					lcd1.print("0");
					lcd1.setCursor(xCor + 7, 1);
				}
				else {
					lcd1.setCursor(xCor + 6, 1);
				}

				lcd1.print(t.sec);

				if (timeColumnON) {
					lcd1.setCursor(xCor + 2, 1);
					lcd1.print(" ");
					lcd1.setCursor(xCor + 5, 1);
					lcd1.print(" ");
					timeColumnON = false;
				}
				else {
					lcd1.setCursor(xCor + 2, 1);
					lcd1.print(":");
					lcd1.setCursor(xCor + 5, 1);
					lcd1.print(":");
					timeColumnON = true;
				}
				dispTime[2] = t.sec;
			}

			if (dispTime[3] != t.date) {
				// Clear LCD
				lcd1.clear();
				for (int i = 0; i < 3; i++) {
					dispTime[i] = 0;
				}

				lcd1.setCursor(0, 0);
				lcd1.print(timeString);
				dispTime[3] = t.date;

				lcd1.setCursor(xCor - 1, 1);
				lcd1.write(0);
				lcd1.setCursor(xCor + 8, 1);
				lcd1.write(1);
			}



			previousClockMillis = currentMillis;
		}

	}

	/* Temp Disp
	if ((currentMillis - previousTempMillis) > 2000) {
		readTemp();

		int tempI, humidI;


		lcd2.clear();
		lcd2.setCursor(0, 0);
		lcd2.print("LCD 2 Working");
		lcd2.setCursor(14, 0);
		//lcd2.write(0);
		lcd2.write(3);

		lcd2.setCursor(0, 1);
		char tempChar[16];
		tempI = temp;
		humidI = humid;

		snprintf(tempChar, 16, "   %2d C    %2d%c", tempI, humidI, 37);
		lcd2.print(tempChar);

		lcd2.setCursor(1, 1);
		lcd2.write(1);
		lcd2.setCursor(9, 1);
		lcd2.write(2);
		lcd2.setCursor(5, 1);
		lcd2.print((char)223);


		previousTempMillis = currentMillis;
	}
	*/

	// Serial Check
	if ((currentMillis - previousSerialMillis) > 100) {
		serialEvent();
		previousSerialMillis = currentMillis;
	}

}

void printTime() {
	// Get the current time and date from the chip.
	t = rtc.time();

	// Name the day of the week.
	const String day = dayAsString(t.day);

	// Format the time and date and insert into the temporary buffer.
	snprintf(timeString, sizeof(timeString), "%-5s %02d-%02d-%04d %02d:%02d:%02d",
		day.c_str(),
		t.date, t.mon, t.yr,
		t.hr, t.min, t.sec);

	// Print the formatted string to serial so we can see the time.
	if (serialClockON) {
		Serial.println(timeString);
	}
}

void readTemp() {
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	humid = dht.readHumidity();
	// Read temperature as Celsius
	temp = dht.readTemperature();
	// Read temperature as Fahrenheit
	farht = dht.readTemperature(true);

	// Check if any reads failed and exit early (to try again).
	if (isnan(humid) || isnan(temp) || isnan(farht)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	// Compute heat index
	// Must send in temp in Fahrenheit!
	hIndx = dht.computeHeatIndex(farht, humid);
	hIndx = (hIndx - 32) / 1.8;

	if (serialTempON) {
		Serial.print("Humid: ");
		Serial.print(humid);
		Serial.print("% ");
		Serial.print("Temp: ");
		Serial.print(temp);
		Serial.print("*C ");
		Serial.print("HeatIdx: ");
		Serial.print(hIndx);
		Serial.println("*C");
	}
}

void serialEvent() {

	// Double Check SerialAvaliable
	if (!Serial.available()) {
		return; // Goes back if not
	}

	String serialString;
	serialString.reserve(20);
	char globalChar[30];

	delay(32);// Let Serial Data Finish Flowing;

			  // READS 1st Char, for detection of "#"
	char inChar = (char)Serial.read();

	// CHAR DATA INPUT FUNCTION "#123456HSFJEKFD"
	if (inChar == '#') {

		// Store Data in global array
		int i = 0;

		while (Serial.available()) {

			if (i <= 29) {
				inChar = (char)Serial.read();
				globalChar[i] = inChar;
				i++;
			}
			else {
				// ignores rest of data
				inChar = (char)Serial.read();
			}
		}

		globalChar[i] = '\0'; // Adds in ending

		Serial.print("Printing Received String[");
		Serial.print(i);
		Serial.print("]: ");
		Serial.println(globalChar);
		lcd1.print(globalChar);

		return; // Finish Loop
	}

	// CURSOR CHANGE FUNCTION
	if (inChar == '@') {

		int row, col;

		row = (char)Serial.read() - '0';
		col = (char)Serial.read() - '0';

		Serial.print("Cursor Change (X,Y): ");
		Serial.print(row);
		Serial.print(",");
		Serial.println(col);

		lcd1.setCursor(col, row); // (Y,X) INPUT j,i

								  // ignore rest of data
		while (Serial.available()) {
			inChar = (char)Serial.read();
		}
		return;
	}

	// CUSTOM CHARACTER FUNCTION
	if (inChar == '!') {

		int i;

		Serial.print("Custom Char #");
		Serial.println(i);

		lcd1.write(i);

		// ignore rest of data
		while (Serial.available()) {
			inChar = (char)Serial.read();
		}
		return;
	}

	// COMMAND RECOGNITION FUNCTION

	// Stores the 1st Char that was pre checked for "#"
	serialString += inChar;

	while (Serial.available()) {
		char inChar = (char)Serial.read();
		serialString += inChar;
	}
	Serial.print("String Received: ");
	Serial.println(serialString);

	//String decoder
	if (serialString == "clear" || serialString == "c") {
		Serial.println("Clearing");
		lcd1.clear();

	}
	else if (serialString == "line" || serialString == "n") {
		Serial.println("NewLine");
		lcd1.setCursor(0, 1);

	}
	else if (serialString == "off") {
		Serial.println("LCD OFF");
		lcd1.noDisplay();
		lcd1.noBacklight();
	}
	else if (serialString == "on") {
		Serial.println("LCD ON");
		lcd1.display();
		lcd1.backlight();
	}
	else if (serialString == "blink") {
		Serial.println("Toggle Blink");
		if (lcd_blink == true) {
			lcd_blink = false;
			lcd1.noBlink();
		}
		else {
			lcd_blink = true;
			lcd1.blink();
		}
	}
	else if (serialString == "clock") {
		clockON = !clockON;
		dispTime[3] = NULL;
		Serial.print("Toggle Clock: ");
		Serial.println(clockON);
	}
	else if (serialString == "sclock") {
		Serial.print("Toggle Serial Clock: ");
		Serial.println(clockON);
		serialClockON = !serialClockON;
	}
	else if (serialString == "stemp") {
		Serial.print("Toggle Serial Tmp: ");
		Serial.println(serialTempON);
		serialTempON = !serialTempON;
	}
	else {
		Serial.print("ERROR COMMAND UNKNOWN: ");
		Serial.println(serialString);
	}
	serialString = "";

	return;

}