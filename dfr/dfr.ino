
/*
╔═════════════════════════════════════════════════════════════════════════════╗
║ DFR - DFAM resetter                                                         ║
╠═════════════════════════════════════════════════════════════════════════════╣
║ Simple GPIO based pulse counter and generator for DFAM                      ║
║                                                                             ║
║ 1. Auto-init                                                                ║
║       * Poweron sends pulses to ADV/CLK to set DFAM to start position       ║
║       * Delays startup to ignore spikes from DFAM power-on                  ║
║       * Blinks internal LED the number of steps to run                      ║
║ 2. Auto-reset                                                               ║
║       * Counts TRIGGER pulses and waits for a timeout to stop               ║
║       * Wait for pulses to stop with a given timeout                        ║
║       * Generates a calculated number of reset pulses to move to step 8     ║
║ 3. Step limiter/shifter                                                     ║
║       * Limits the number of steps to less than 8 via button press          ║
║       * Shift starting step on button press                                 ║
║       * Sends extra pulses at cycle end to reach last step                  ║
╚═════════════════════════════════════════════════════════════════════════════╝
*/

/// FUTURE enhancements
/// * Manual reset button (also toggles auto-reset when not running)
/// * get run signal to handle sequencer pause (clock still running)

// pin config (safe pins)
// https://www.espboards.dev/esp32/esp32-c3-super-mini
const int inputPin = 0;   // the gate/trig input pin
const int outputPin = 1;  // adv/clk output pin
const int minusPin = 3;   // button to reduce steps
const int shiftPin = 10;  // button to shift start position
const int ledPin = 8;

// output gate time in milliseconds
int gate = 100;

// default steps
const int STEPCOUNT = 8;
const int STEPSTART = 0;
int steps = STEPCOUNT;
int start = STEPSTART;

// time to call auto-reset
int autoResetTime = 1000;

// startup delay (ignore voltage signals on DFAM power-on)
int startupDelay = 1000;

// debounce buttonPresses
int debounce = 100;

// pulse count/detection variables
volatile int pulseCount = 0;
volatile int signalChange = LOW;  // last signal change


// time in ms of last received pulse
unsigned long lastPulseTime = 0;
bool pulseDetected = false;

// button actions
bool minusStep = false;  // reduces by one Step
bool shiftStep = false;  // shifts start to the right

/******************
 * BUTTON ACTIONS *
 ******************/

// pulse counter
void IRAM_ATTR onPulse() {
  signalChange = digitalRead(inputPin);
  pulseCount += 1;
  pulseDetected = true;
}

void IRAM_ATTR buttonMinus() {
  minusStep = true;
  delay(debounce);
}

void IRAM_ATTR buttonShift() {
  // TODO check if can be shifted
  if (start + steps < STEPCOUNT) {
    shiftStep = true;
  }
  delay(debounce);
}

/***************
 * ARITHMETRIC *
 ***************/

// decrease steps by 1 and wrap back around to 8
void IRAM_ATTR changeSteps() {
  steps--;
  if (steps == STEPSTART) {
    steps = 8;
  }
}

// changes start value (wraps around to 0)
void IRAM_ATTR changeStart() {
  if (++start == STEPCOUNT - 1) {
    start = STEPSTART;
  }
}

// is running
bool IRAM_ATTR isRunning() {
  unsigned long now = millis();
  bool running = now - lastPulseTime < autoResetTime;
  Serial.printf("LAST: %d.\n", lastPulseTime);
  Serial.printf("THIS: %d.\n", now);
  Serial.printf("RUN?: %d.\n", running);
  return running;
}

// send pulses
void IRAM_ATTR sendPulses(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(outputPin, HIGH);
    digitalWrite(ledPin, HIGH);
    delay(gate);
    digitalWrite(outputPin, LOW);
    digitalWrite(ledPin, LOW);
    delay(gate);
  }
}

// blink the internal LED
void IRAM_ATTR blink(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(gate);
    digitalWrite(ledPin, LOW);
    delay(gate);
  }
}

// setup function
// initializes serial, pins and interrupts
// sends initial pulses to set DFAM to start position
// blinks the internal LED the number of steps
void setup() {
  // init serial
  Serial.begin(115200);

  // setup pins and wait for startup
  pinMode(inputPin, INPUT_PULLUP);  // Use pull-up if switch is grounding
  // pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  delay(startupDelay);

  // print pin configuration
  Serial.printf("Input Pin: %d\n", inputPin);
  Serial.printf("Output Pin: %d\n", outputPin);
  Serial.printf("LED Pin: %d\n", ledPin);

  // set to last steps on startup
  sendPulses(steps);
  blink(steps);

  // attach pulse counter callback (adjust to signal edge
  // -> Adjust for your signal edge
  attachInterrupt(digitalPinToInterrupt(inputPin), onPulse, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(shiftPin), buttonShift, FALLING);
  // attachInterrupt(digitalPinToInterrupt(minusPin), buttonMinus, FALLING);
}

void loop() {
  // Only check timing if a pulse has been detected
  if (pulseDetected) {
    pulseDetected = false;  // reset pulse detection flag

    Serial.print("Interrupt! Signal is now: ");
    Serial.println(signalState == HIGH ? "HIGH" : "LOW");

    // reset logic if pulses were stopped for a while
    if (!isRunning()) {
      // reset if stopped for more than auto-reset time
      int currentStep = pulseCount % STEPCOUNT;
      int resetPulses = STEPCOUNT - currentStep + start;
      Serial.printf("Timeout: Input pulses: %d.\n", pulseCount);
      Serial.printf("Timeout: Sending %d reset pulses.\n", resetPulses);
      sendPulses(resetPulses);
      pulseCount = 1;
    } else {
      sendPulses(1);
      Serial.printf("Pulse count: %d.\n", pulseCount);
    }
    lastPulseTime = millis();
  }
  // buttons
  if (minusStep) {
    changeSteps();
    Serial.printf("Button pressed. Steps: %d\n", steps);
    minusStep = false;
  }
  if (shiftStep) {
    changeStart();
    Serial.printf("Button pressed. Start: %d\n", start);
    shiftStep = false;
  }
}
