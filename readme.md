# avans-kluis

(In Dutch)

## Dingen die nog gedaan moeten worden

- Code veranderen door de SET knop aan de binnenkant van de kluis in te drukken
- Index out of bounds crash wanneer er meer dan 1024 cijfers ingetoetst worden
- Heel misschien een grappige easter egg ofzo

## Coole functies

- Event-gebaseerde keyboard lezer
- Niet-blokkerende LED functie
- Overmatig veel gebruik van `#define` voor pinouts en alles
- Debounce correctie
- Code's die tot wel 1024 getallen lang kunnen zijn
- De enige blokkerende routine's zijn om de kluis te openen en te schreeuwen
	tegen de gebruiker dat de code fout is
- Ongeveer zoveel commentaar als daadwerkelijke code
- Veel te ingewikkeld voor het 1e project

De veiligheid van deze software is niet gegarandeerd, want als de Arduino
crasht of uitvalt wordt de code terug gezet op 0000. Dat maakt niet veel uit
want je kunt de kluis toch openen door hem een lichte ram te geven vanaf de
onderkant terwijl je hem open draait.
