#define SERIAL_BAUD 115200
#define MAX_CODE_LEN 1024

#define PINOUT_KBM7   4
#define PINOUT_KBM6   5
#define PINOUT_KBM5   6
#define PINOUT_KBM4   7
#define PINOUT_KBM3   12
#define PINOUT_KBM2   8
#define PINOUT_KBM1   9
#define PINOUT_LED_Y  19
#define PINOUT_LED_R  18
#define PINOUT_LED_G  17
#define PINOUT_BUZZ   16
#define PINOUT_SET    15
#define PINOUT_RELAIS 14

#define KB_ROW_1 PINOUT_KBM2
#define KB_ROW_2 PINOUT_KBM3
#define KB_ROW_3 PINOUT_KBM5
#define KB_ROW_4 PINOUT_KBM7
#define KB_COL_1 PINOUT_KBM1
#define KB_COL_2 PINOUT_KBM4
#define KB_COL_3 PINOUT_KBM6

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

	// keyboard kolommen als output
	pinMode(KB_COL_1, OUTPUT);
	pinMode(KB_COL_2, OUTPUT);
	pinMode(KB_COL_3, OUTPUT);

	// keyboard rijen als input
	pinMode(KB_ROW_1, INPUT_PULLUP);
	pinMode(KB_ROW_2, INPUT_PULLUP);
	pinMode(KB_ROW_3, INPUT_PULLUP);
	pinMode(KB_ROW_4, INPUT_PULLUP);
}

void kb_scan() {
	// kopieer kb naar kb_old
	memcpy(&kb_old, kb, sizeof(kb_old));

	// stel alle indices van kb_old in op 0
	memset(&kb, 0, sizeof(kb));

	for (int col = 0; col < 3; col++) {
		digitalWrite(kb_cols[col], HIGH);
		delay(10);

		for (int row = 0; row < 4; row++) {
			Serial.print("checking pin ");
			Serial.print(kb_cols[col], DEC);
			Serial.print(" -> ");
			Serial.print(kb_rows[row], DEC);
			Serial.print(" ...\n");

			kb[3 * col + row] = digitalRead(kb_rows[row]);
			delay(10);
		}

		digitalWrite(kb_cols[col], LOW);
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
	//TODO: buzz;
	digitalWrite(PINOUT_LED_R, HIGH);

	delay(1e3); // shorthand voor 1000

	digitalWrite(PINOUT_LED_R, LOW);
}

void unlock_routine() {
	//TODO: open_solenoid();
	//TODO: buzz;
	digitalWrite(PINOUT_LED_G, HIGH);

	delay(2e3);

	digitalWrite(PINOUT_LED_G, LOW);
}

void kb_onevent(kb_event ev) {
	if (!ev.down) return;

	switch (ev.key) {
		// C toets (clear)
		case 9: {
			// TODO: kijk of sizeof() werkt voor arrays
			memset(&code, 0, sizeof(code));
			codei = 0;

			break;
		}

		// E toets (enter)
		case 11: {
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
			// bereken getal op toets
			// (key + 1 voor alle toetsen behalve 0)
			int num = ev.key == 10 ? 0 : ev.key + 1;

			// voeg getal toe aan het einde van de code
			code[codei] = num;
			codei++;

			//TODO: beep
		}
	}
}

void kb_event_gen() {
	for(int i = 0; i < 12; i++) {
		// ga door naar de volgende iteratie
		// als de toetsen niet veranderd zijn
		if (kb[i] == kb_old[i]) continue;

		kb_event event = {
			.key = i,
			.down = kb[i],
			.timestamp = millis()
		};

		kb_onevent(event);
	}
}

void loop() {
	kb_scan();
	kb_event_gen();
	debug_print_arr(kb);
	Serial.print("time ");
  Serial.print(millis(), DEC);
  Serial.print("\n");
	delay(1e3);
}
