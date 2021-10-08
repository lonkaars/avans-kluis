void setup() {
	// put your setup code here, to run once:
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
}
void loop() {
	// put your main code here, to run repeatedly:

	//LED 1 aansturen
	digitalWrite(2, HIGH);
	delay(1000);
	digitalWrite(2, LOW);
	delay(1000);

	//LED 2 aansturen
	digitalWrite(3, HIGH);
	delay(1000);
	digitalWrite(3, LOW);
	delay(1000);

	//LED 3 aansturen
	digitalWrite(4, HIGH);
	delay(1000);
	digitalWrite(4, LOW);
	delay(1000);
}
