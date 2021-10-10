int drukKnop = 12; //Hier initialiseer je de waarden
int led = A0;
int StaatdrukKnop = LOW; //Staat van de druktoets instellen

void setup() {
	Serial.begin(9600);
	pinMode(led, OUTPUT); //LED instellen als uitvoer
	pinMode(drukKnop, INPUT); //Druktoets instellen als invoer
}

void loop() {
	StaatdrukKnop = digitalRead(drukKnop); //Staat van de druktoets de waarde geven van het uitlezen van de druktoets
	Serial.println(StaatdrukKnop);

	if (StaatdrukKnop == HIGH){
		digitalWrite(led, HIGH);
	} else {
		digitalWrite(led, LOW);
	}
}
