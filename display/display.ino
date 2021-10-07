void setup() {
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
}

int teller = 0;

void loop() {
	teller = (teller + 1) % 10;

	digitalWrite(2, teller & 0b1000);
	digitalWrite(3, teller & 0b0100);
	digitalWrite(4, teller & 0b0010);
	digitalWrite(5, teller & 0b0001);

	delay(1000);
}
