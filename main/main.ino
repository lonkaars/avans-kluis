#define SERIAL_BAUD 115200
#define MAX_CODE_LEN 1024

#define BUZZ_HAPPY 1000
#define BUZZ_SAD   700

#define PINOUT_KBM7   4
#define PINOUT_KBM6   5
#define PINOUT_KBM5   6
#define PINOUT_KBM4   7
#define PINOUT_KBM3   12
#define PINOUT_KBM2   8
#define PINOUT_KBM1   9
#define PINOUT_LED_Y  A5
#define PINOUT_LED_R  A4
#define PINOUT_LED_G  A3
#define PINOUT_BUZZ   A2
#define PINOUT_SET    A1
#define PINOUT_RELAIS A0

#define KB_ROW_1 PINOUT_KBM2
#define KB_ROW_2 PINOUT_KBM3
#define KB_ROW_3 PINOUT_KBM5
#define KB_ROW_4 PINOUT_KBM7
#define KB_COL_1 PINOUT_KBM1
#define KB_COL_2 PINOUT_KBM4
#define KB_COL_3 PINOUT_KBM6

#define KB_KEY_1 0
#define KB_KEY_2 1
#define KB_KEY_3 2
#define KB_KEY_4 3
#define KB_KEY_5 4
#define KB_KEY_6 5
#define KB_KEY_7 6
#define KB_KEY_8 7
#define KB_KEY_9 8
#define KB_KEY_C 9
#define KB_KEY_0 10
#define KB_KEY_E 11

int kb_rows[] = { KB_ROW_1, KB_ROW_2, KB_ROW_3, KB_ROW_4 };
int kb_cols[] = { KB_COL_1, KB_COL_2, KB_COL_3 };

unsigned char kb[12], kb_old[12];

unsigned char correct_code[] = { 0, 0, 0, 0 };
unsigned char code[MAX_CODE_LEN];
unsigned int codei = 0;

typedef struct {
	unsigned char key;
	bool down;
	unsigned long timestamp;
} kb_event;

void setup() {
	// Start serial (vooral voor debugging)
	Serial.begin(SERIAL_BAUD);

	// keyboard kolommen als input
	pinMode(KB_COL_1, INPUT_PULLUP);
	pinMode(KB_COL_2, INPUT_PULLUP);
	pinMode(KB_COL_3, INPUT_PULLUP);

	// keyboard rijen als output
	pinMode(KB_ROW_1, OUTPUT);
	pinMode(KB_ROW_2, OUTPUT);
	pinMode(KB_ROW_3, OUTPUT);
	pinMode(KB_ROW_4, OUTPUT);

	// Overige outputs
	pinMode(PINOUT_LED_G, OUTPUT);
	pinMode(PINOUT_LED_R, OUTPUT);
	pinMode(PINOUT_LED_Y, OUTPUT);
	pinMode(PINOUT_RELAIS, OUTPUT);
	pinMode(PINOUT_BUZZ, OUTPUT);
}

void kb_scan() {
	// kopieer kb naar kb_old
	memcpy(&kb_old, kb, sizeof(kb_old));

	// stel alle indices van kb_old in op 0
	memset(&kb, 0, sizeof(kb));

	for (int row = 0; row < 4; row++) {
		Serial.print("Pin ");
		Serial.print(kb_rows[row], DEC);
		Serial.print(" is now high\n");
		digitalWrite(kb_rows[row], HIGH);

		for (int col = 0; col < 3; col++) {
			// Serial.print("checking pin ");
			// Serial.print(kb_cols[col], DEC);
			// Serial.print(" -> ");
			// Serial.print(kb_rows[row], DEC);
			// Serial.print(" ...\n");

			// Serial.print("kb[");
			// Serial.print(3 * row + col, DEC);
			// Serial.print("] (c");
			// Serial.print(kb_cols[col], DEC);
			// Serial.print(", r");
			// Serial.print(kb_rows[row], DEC);
			// Serial.print(")\n");
			kb[3 * row + col] = !digitalRead(kb_cols[col]);

			Serial.print("Pin ");
			Serial.print(kb_cols[col], DEC);
			Serial.print(" is now ");
			Serial.print(digitalRead(kb_cols[col]), DEC);
			Serial.print("\n");
		}

		digitalWrite(kb_rows[row], LOW);
		Serial.print("Pin ");
		Serial.print(kb_rows[row], DEC);
		Serial.print(" is now low\n");
		delay(10);
	}
}

void debug_print_arr(unsigned char x[12]) {
	Serial.print("[");
	for(int i = 0; i < 12; i++) {
		Serial.print(x[i], DEC);
		if (i != 11) Serial.print(", ");
	}
	Serial.print("]\n");
}

void wrong_code_routine() {
	tone(PINOUT_BUZZ, BUZZ_SAD);
	digitalWrite(PINOUT_LED_R, HIGH);

	delay(1e3); // shorthand voor 1000

	digitalWrite(PINOUT_LED_R, LOW);
	noTone(PINOUT_BUZZ);
}

void unlock_routine() {
	tone(PINOUT_BUZZ, BUZZ_HAPPY);
	digitalWrite(PINOUT_RELAIS, HIGH);
	digitalWrite(PINOUT_LED_G, HIGH);

	delay(2e3);

	digitalWrite(PINOUT_LED_G, LOW);
	digitalWrite(PINOUT_RELAIS, LOW);
	noTone(PINOUT_BUZZ);
}

void kb_onevent(kb_event ev) {
	if (!ev.down) return;

	if (ev.key != KB_KEY_E) tone(PINOUT_BUZZ, BUZZ_HAPPY, 200);

  Serial.println(ev.key, DEC);

	switch (ev.key) {
		case KB_KEY_C: {
			// TODO: kijk of sizeof() werkt voor arrays
			memset(&code, 0, sizeof(code));
			codei = 0;

			break;
		}

		case KB_KEY_E: {
			//TODO: array length
			int correct_code_len = 3;
			bool correct = true;

			if (codei != correct_code_len) break;
			for (int i = 0; i < correct_code_len; i++) {
				if (correct_code[i] == code[i]) continue;

				// dit voert alleen uit als een van
				// de getallen in code[] niet gelijk is aan correct_code[]
				correct = false;
				break;
			}
			
			// debug
			Serial.print(correct, DEC);

			// kijk mama geen haakjes
			if (correct)
				unlock_routine();
			else
				wrong_code_routine();

			break;
		}

		// Overige toetsen (nummers)
		default: {
			//TODO: NON BLOCKING LED VOOR ALLE LEDS

			// bereken getal op toets
			// (key + 1 voor alle toetsen behalve 0)
			int num = ev.key == 10 ? 0 : ev.key + 1;

			// voeg getal toe aan het einde van de code
			code[codei] = num;
			codei++;
		}
	}
}

void kb_event_gen() {
	for(int i = 0; i < 12; i++) {
		// ga door naar de volgende iteratie
		// als de toetsen niet veranderd zijn
		if (kb[i] == kb_old[i]) continue;

		kb_event event = {
			.key = (unsigned char) i,
			.down = kb[i],
			.timestamp = millis()
		};

		kb_onevent(event);
	}
}

void loop() {
	// unlock_routine();
	// delay(3e3);
	// wrong_code_routine();
	// delay(10e3);
	kb_scan();
	kb_event_gen();
	// debug_print_arr(kb);
	// Serial.print("time ");
  // Serial.print(millis(), DEC);
  // Serial.print("\n");
	// delay(1e3);
}
