const byte PIN_OPTO = 2;
const byte PIN_RELAY = 7;
const byte LED_STATUS = 5;
const byte LED_RELAY = 6;

const unsigned long OUTAGE_THRESHOLD_MS = 5000UL;
const unsigned long BLINK_INTERVAL_MS = 250UL;
const unsigned long AC_PULSE_INTERVAL = 30UL;  // more than a full cycle
const unsigned long RELAY_PULSE_INTERVAL = 250UL;

enum State {
  MONITORING,
  OUTAGE_DETECTED,
  TRIGGER_RELAY,
  WAIT,
} currentState;

volatile bool acPresent = false;
volatile unsigned long lastAcPulse_ms = 0ul;

unsigned long outageStart_ms = 0ul;
unsigned long lastBlink_ms = 0ul;
bool blinkState = false;

void onAcPulse() {
  lastAcPulse_ms = millis();
  acPresent = true;
}

void setup() {
  pinMode(PIN_OPTO, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(LED_RELAY, OUTPUT);

  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(LED_STATUS, HIGH);
  digitalWrite(LED_RELAY, LOW);

  attachInterrupt(digitalPinToInterrupt(PIN_OPTO), onAcPulse, RISING);
}

void loop() {
  const unsigned long now = millis();

  if (now - lastAcPulse_ms > AC_PULSE_INTERVAL) {
    acPresent = false;
  }

  switch (currentState) {
    case MONITORING:
      digitalWrite(PIN_RELAY, LOW);
      digitalWrite(LED_STATUS, HIGH);
      digitalWrite(LED_RELAY, LOW);

      if (!acPresent) {
        outageStart_ms = now;
        currentState = OUTAGE_DETECTED;
      }
      break;

    case OUTAGE_DETECTED:
      if (now - lastBlink_ms >= BLINK_INTERVAL_MS) {
        lastBlink_ms = now;
        blinkState = !blinkState;
        digitalWrite(LED_STATUS, blinkState ? HIGH : LOW);
      }

      if (acPresent) {
        currentState = MONITORING;
      } else if (now - outageStart_ms >= OUTAGE_THRESHOLD_MS) {
        currentState = TRIGGER_RELAY;
      }
      break;

    case TRIGGER_RELAY:
      digitalWrite(LED_STATUS, LOW);
      digitalWrite(LED_RELAY, HIGH);

      digitalWrite(PIN_RELAY, HIGH);
      delay(RELAY_PULSE_INTERVAL);
      digitalWrite(PIN_RELAY, LOW);
      currentState = WAIT;
      break;

    case WAIT:
      while (true);
      break;
  }
}
