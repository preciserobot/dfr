
/*
╔═════════════════════════════════════════════════════════════════════════════════╗
║ DFR - DFAM resetter                                                             ║
╠═════════════════════════════════════════════════════════════════════════════════╣
║ Simple GPIO based pulse counter and generator for DFAM                          ║
║                                                                                 ║
║ 1. Auto-init                                                                    ║
║       * When powered on sends pulses to ADV/CLK to set DFAM to start position   ║
║       * Delays startup to ignore spikes from DFAM power-on                      ║
║ 2. Auto-reset                                                                   ║
║       * Counts TRIGGER pulses and waits for a timeout to stop                   ║
║       * Wait for pulses to stop with a given timeout                            ║
║       * Generates a calculated number of reset pulses to move to step 8         ║
║ 3. Step limiter (future)                                                        ║
║       * Limits the number of steps to less than 8 via button press              ║
║       * Sends extra pulses at cycle end to reach last step                      ║
╚═════════════════════════════════════════════════════════════════════════════════╝
*/

// the gate/trig input pin
const int inputPin = 15;

// adv/clk output pin
const int outputPin = 2;

// output gate time in milliseconds
int gate = 100;
//
// default steps
int steps = 8;
//
// time to call auto-reset
int autoResetTime = 1000;

// startup delay (ignore voltage signals on DFAM power-on)
int startupDelay = 1000;

// pulse count/detection variables
volatile int pulseCount = 0;

// time in ms of last received pulse
unsigned long lastPulseTime = 0;
bool pulseDetected = false;

//
// pulse counter
void IRAM_ATTR onPulse() {
    pulseCount++;
    lastPulseTime = millis();
    pulseDetected = true;
}

// decrease steps by 1 and wrap back around to 8
void IRAM_ATTR changeSteps() {
    steps--;
    if (steps < 1) {
        steps = 8;
    }
}

// sets to last step on startup
void IRAM_ATTR initSteps(int steps) {
    for (int i = 1; i < steps; i++) {
        digitalWrite(outputPin, HIGH);
        delay(gate);
        digitalWrite(outputPin, LOW);
        delay(gate);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(inputPin, INPUT_PULLUP);    // Use pull-up if switch is grounding
    pinMode(outputPin, OUTPUT);
    // wait a second before initialisation
    delay(startupDelay);
    // set to last steps on startup
    initSteps(steps);
    // attach pulse counter callback
    attachInterrupt(digitalPinToInterrupt(inputPin), onPulse, FALLING);    // Adjust for your signal edge
}


void loop() {
    // Only check timing if a pulse has been detected
    if (pulseDetected && (millis() - lastPulseTime > autoResetTime)) {
        int outputPulses = steps - (pulseCount % steps);
        if (outputPulses > 0) {
            Serial.printf("Timeout reached. Input pulses: %d. Sending %d output pulses.\n", pulseCount, outputPulses);
            for (int i = 0; i < outputPulses; i++) {
                digitalWrite(outputPin, HIGH);
                delay(gate);
                digitalWrite(outputPin, LOW);
                delay(gate);
            }
        } else {
            Serial.printf("Timeout reached. Input pulses: %d. No output pulses needed.\n", pulseCount);
        }

        pulseCount = 0;
        pulseDetected = false;
    }
}

