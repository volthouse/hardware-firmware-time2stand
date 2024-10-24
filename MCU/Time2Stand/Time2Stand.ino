#include <Arduino.h>

const int trigPin = 7;
const int echoPin = 6;


// Definiere die Struktur
struct Data {
  uint16_t distance;
  uint16_t count;
};

static struct Data m_data;

// Funktion zur Berechnung der Prüfziffer für beliebige Strukturen
uint8_t calculateChecksum(const void* data, size_t size) {
    uint8_t checksum = 0;
    const uint8_t* byteData = static_cast<const uint8_t*>(data);

    for (size_t i = 0; i < size; ++i) {
        checksum += byteData[i];
    }

    return checksum;
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Starte die serielle Kommunikation mit 9600 Baud
  Serial.begin(9600);

  m_data.distance = 100;
  m_data.count = 0;
}

void loop() {
  long duration;
  // Überprüfe, ob Daten verfügbar sind
  if (Serial.available() > 0) {
    // Lese die eingehenden Daten
    String command = Serial.readStringUntil('\n'); // Lese bis zum Zeilenumbruch

    // Überprüfe, ob das Kommando "GETDATA" empfangen wurde
    if (command.equals("GETDATA")) {
      // Berechne die Prüfziffer

      // Trigger the sensor
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);

      // Read the echo
      duration = pulseIn(echoPin, HIGH);
      m_data.distance = (duration * 0.034) / 2; // cm

      uint8_t checksum = calculateChecksum(&m_data, sizeof(m_data));

      // Sende die Daten binär über die serielle Schnittstelle
      Serial.write((uint8_t*)&m_data, sizeof(m_data)); // Sende die Struktur
      Serial.write(checksum); // Sende die Prüfziffer

      m_data.count++;
    } else if (command.equals("RESET")) {
      memset(&m_data, 0, sizeof(m_data));
    } else if (command.equals("TEST_SIT")) {
      m_data.distance = 60;
    } else if (command.equals("TEST_STAND")) {
      m_data.distance = 117;
    }
  }
}
