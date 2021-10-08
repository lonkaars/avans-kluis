#include "constants.h"

/** KB -> arduino pin mapping
 * 0  (1) ->  8 & 9
 * 1  (2) ->  8 & 7
 * 2  (3) ->  8 & 5
 * 3  (4) -> 12 & 9
 * 4  (5) -> 12 & 7
 * 5  (6) -> 12 & 5
 * 6  (7) ->  6 & 9
 * 7  (8) ->  6 & 7
 * 8  (9) ->  6 & 5
 * 9  (C) ->  4 & 9
 * 10 (0) ->  4 & 7
 * 11 (E) ->  4 & 5
 */

const int kb_rows[] = { KB_ROW_1, KB_ROW_2, KB_ROW_3, KB_ROW_4 };
const int kb_cols[] = { KB_COL_1, KB_COL_2, KB_COL_3 };

#define LED_R 0
#define LED_G 1
#define LED_Y 2

const int led_pinouts[] = { PINOUT_LED_R, PINOUT_LED_G, PINOUT_LED_Y };
unsigned long led_timings[3];
bool led_status[3];

unsigned long kb_timings[12];
unsigned char kb[12], kb_old[12];

unsigned char correct_code[MAX_CODE_LEN];
unsigned int correct_code_len = 4;

unsigned char code[MAX_CODE_LEN];
unsigned int codei = 0;

/** @brief informatie over toets die `void kb_onevent()` mee krijgt */
typedef struct {
	/** @brief kb[12] index van de toets */
	unsigned char key;
	/** @brief true als de toets wordt ingedrukt */
	bool down;
	/** @brief tijd van toetsaanslag */
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

	// alle rijen moeten standaard HIGH zijn voor de scan
	digitalWrite(KB_ROW_1, HIGH);
	digitalWrite(KB_ROW_2, HIGH);
	digitalWrite(KB_ROW_3, HIGH);
	digitalWrite(KB_ROW_4, HIGH);

	// LED's
	pinMode(PINOUT_LED_G, OUTPUT);
	pinMode(PINOUT_LED_R, OUTPUT);
	pinMode(PINOUT_LED_Y, OUTPUT);

	// Overige outputs
	pinMode(PINOUT_RELAIS, OUTPUT);
	pinMode(PINOUT_BUZZ, OUTPUT);
}

/** @brief scan het toetsenbord en stop alles in `kb[12]` */
void kb_scan() {
	// kopieer kb naar kb_old
	memcpy(&kb_old, kb, sizeof(kb_old));

	// stel alle indices van kb_old in op 0
	memset(&kb, 0, sizeof(kb));

	for (int row = 0; row < 4; row++) {
		digitalWrite(kb_rows[row], LOW);

		for (int col = 0; col < 3; col++) {
			// index hack om 2d coordinaten in een 1d array op te slaan
			kb[3 * row + col] = !digitalRead(kb_cols[col]);
		}

		digitalWrite(kb_rows[row], HIGH);
	}
}

/** @brief debug functie die een getallen array als JSON print */
void debug_print_arr(unsigned char x[12]) {
	Serial.print("[");
	for(int i = 0; i < 12; i++) {
		Serial.print(x[i], DEC);
		if (i != 11) Serial.print(", ");
	}
	Serial.print("]\n");
}

/** @brief debug functie die de huidige ingevoerde code print */
void debug_print_code() {
	Serial.print("code: ");
	for(int i = 0; i < codei; i++) {
		Serial.print(code[i], DEC);
		if (i < codei - 1) Serial.print(",");
	}
	Serial.print("\n");
}

// wat zou deze functie toch doen
void clear_code() {
	memset(&code, 0, MAX_CODE_LEN);
	codei = 0;
}

/** @brief wordt uitgevoerd wanneer de verkeerde code is ingevoerd */
void wrong_code_routine() {
	Serial.println("verkeerde code");
	tone(PINOUT_BUZZ, BUZZ_SAD);
	digitalWrite(PINOUT_LED_R, HIGH);

	delay(1e3); // shorthand voor 1000

	digitalWrite(PINOUT_LED_R, LOW);
	noTone(PINOUT_BUZZ);

	clear_code();
}

/** @brief wordt uitgevoerd wanneer de juiste code is ingevoerd */
void unlock_routine() {
	Serial.println("juiste");
	tone(PINOUT_BUZZ, BUZZ_HAPPY);
	digitalWrite(PINOUT_RELAIS, HIGH);
	digitalWrite(PINOUT_LED_G, HIGH);

	delay(2e3);

	digitalWrite(PINOUT_LED_G, LOW);
	digitalWrite(PINOUT_RELAIS, LOW);
	noTone(PINOUT_BUZZ);

	clear_code();
}

/** @brief wordt uitgevoerd wanneer "E" wordt ingedrukt */
void check_code() {
	bool correct = true;

	// check lengte
	if (codei != correct_code_len) return wrong_code_routine();

	for (int i = 0; i < correct_code_len; i++) {
		if (correct_code[i] == code[i]) continue;

		// dit voert alleen uit als een van
		// de getallen in code[] niet gelijk is aan correct_code[]
		correct = false;
		break;
	}
	
	// kijk mama zonder haakjes!
	if (correct)
		unlock_routine();
	else
		wrong_code_routine();
}

/**
 * @brief event handler voor het toetsenbord
 * (voert elke keer uit wanneer de toestand van een toets verandert)
 */
void kb_onevent(kb_event ev) {
	if (!ev.down) return; // alleen uit naar aan events boeien (voor nu)

	// zorgt ervoor dat je een toets niet vaker dan een periode van
	// KB_DEBOUNCE_DELAY kunt indrukken
	bool bounce = kb_timings[ev.key] + KB_DEBOUNCE_DELAY > millis();
	kb_timings[ev.key] = ev.timestamp;
	if (bounce) return;

	// korte pieptoon als een nummertoets of *clear* wordt ingedrukt
	if (ev.key != KB_KEY_E) tone(PINOUT_BUZZ, BUZZ_HAPPY, 200);

	switch (ev.key) {
		case KB_KEY_C: {
			clear_code();
			break;
		}

		case KB_KEY_E: {
			check_code();
			break;
		}

		// Overige toetsen (nummers)
		default: {
			led_set_timeout(LED_Y, 200);

			// bereken getal op toets
			// (key + 1 voor alle toetsen behalve 0)
			int num = ev.key == 10 ? 0 : ev.key + 1;

			// voeg getal toe aan het einde van de code
			code[codei] = num;
			codei++; //TODO: fix array out of bounds crash
		}
	}

	debug_print_code();
}

/**
 * @brief voert `kb_onevent` uit
 * wanneer de toestand van een toets verandert
 */
void kb_event_gen() {
	for(int i = 0; i < 12; i++) {
		// ga door naar de volgende iteratie
		// als de toetsen niet veranderd zijn
		if (kb[i] == kb_old[i]) continue;

		// maak en vul struct met informatie
		kb_event event = {
			.key = (unsigned char) i,
			.down = kb[i],
			.timestamp = millis()
		};

		// voer on_event handler uit
		kb_onevent(event);
	}
}

/**
 * @brief zet een led aan voor `duration_millis` milliseconden (non-blocking)
 * @param led `LED_?` constante voor led kleur
 * @param duration_millis aantal milliseconden dat de led aan moet zijn
 */
void led_set_timeout(unsigned int led, unsigned long duration_millis) {
	led_timings[led] = millis() + duration_millis;
	led_status[led] = 1;
	digitalWrite(led_pinouts[led], HIGH);
}

/** @brief led update functie (voor event loop) */
void led_update() {
	unsigned long current_time = millis();

	for (int i = 0; i < 3; i++) {
		// negeer huidige led als die timer nog niet voorbij is
		if (led_timings[i] > current_time) continue;

		// negeer huidige led als die al uit is
		if (led_status[i] == 0) continue;

		// zet huidige led uit
		led_status[i] = 0;
		digitalWrite(led_pinouts[i], LOW);
	}
}

/** @brief arduino `loop` functie, wordt gebruikt als event loop */
void loop() {
	kb_scan();
	kb_event_gen();
	led_update();
}

