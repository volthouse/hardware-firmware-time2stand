#include <Arduino.h>

const int trigPin = 7;
const int echoPin = 6;

unsigned long sitTime = 0; // Zeit im Sitzen in Minuten
unsigned long standTime = 0; // Zeit im Stehen in Minuten
unsigned long lastTime = 0; // Letzte Zeitstempel
unsigned long currentMillis = 0; // Aktuelle Zeit in Millisekunden
int sitStandThreshold = 80; // Grenzwert für Sitzen/Stehen in cm

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  lastTime = millis(); // Initialisiere lastTime
}

void loop() {
  long duration, distance;
  int pos;

  // Trigger the sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2; // cm

  // Aktuelle Zeit in Millisekunden
  currentMillis = millis();

  // Überprüfen, ob die Person sitzt oder steht
  if (distance < sitStandThreshold) { // Beispielwert für Sitzen
    // Wenn die Person sitzt, inkrementiere die sitTime
    sitTime += (currentMillis - lastTime);
    pos = 0;
  } else {
    // Wenn die Person steht, inkrementiere die standTime
    standTime += (currentMillis - lastTime);
    pos = 1;
  }

  // Aktualisiere lastTime
  lastTime = currentMillis;

  // Ausgabe der Zeiten
  Serial.print("Sitzzeit: ");
  Serial.print(sitTime / 60000);
  Serial.print(" Min, Stehzeit: ");
  Serial.print(standTime / 60000);
  Serial.print(" Min, ");
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(", Threshold: ");
  Serial.print(sitStandThreshold);
  if(pos == 0) {
    Serial.println(", Sit");
  } else {
    Serial.println(", Stand");
  }

  // if (Serial.available() > 0) {
  //   String command = Serial.readStringUntil('\n'); // Lese bis zum Zeilenumbruch
  //   if (command.equals("reset")) { // Überprüfe, ob das Kommando "reset" empfangen wurde
  //     sitTime = 0; // Setze Sitzzeit zurück
  //     standTime = 0; // Setze Stehzeit zurück
  //     Serial.println("Sitz- und Stehzeit zurückgesetzt.");
  //   }
  // }

  // Überprüfen auf serielle Eingabe
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Lese bis zum Zeilenumbruch
    if (command.equals("reset")) { // Überprüfe, ob das Kommando "reset" empfangen wurde
      sitTime = 0; // Setze Sitzzeit zurück
      standTime = 0; // Setze Stehzeit zurück
      Serial.println("Sitz- und Stehzeit zurückgesetzt.");
    } else if (command.startsWith("st ")) { // Überprüfe, ob das Kommando "setThreshold" empfangen wurde
      int newThreshold = command.substring(3).toInt(); // Extrahiere den neuen Grenzwert
      if (newThreshold > 0) { // Überprüfe, ob der neue Grenzwert positiv ist
        sitStandThreshold = newThreshold; // Setze den neuen Grenzwert
        Serial.print("Grenzwert für Sitzen/Stehen auf ");
        Serial.print(sitStandThreshold);
        Serial.println(" cm gesetzt.");
      } else {
        Serial.println("Ungültiger Grenzwert. Bitte einen positiven Wert eingeben.");
      }
    }
  }

  delay(1000); // 1 Sekunde warten
}
