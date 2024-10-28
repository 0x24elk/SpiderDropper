#include <RCSwitch.h>
#include <Servo.h>

// A "spider dropper" for Halloween.
//
// Controls a small Fischertechnik assembly to suddenly drop and
// wind back up a rubber spider for a little Halloween scare.
// The assembly uses a motor shield with a DC motor and a servo.
// The servo acts as a "clutch", shifting an axle to (dis-)engage
// gears in the drivetrain. This way we can let the spider free-fall
// and then enagage the DC motor to wind it back up.
// 
// We're controlling the spider drop with a 433MHz garage door remote.

// Pins

// Pin the clutch servo's PWM input is connected to.
const int CLUTCH_SERVO_PIN = 5; 
// Pin controlling DC motor direction on the shield.
const int MOTOR_DIRECTION_PIN = 12;
// Pin controlling DC motor speed (PWM) on the shield.
const int MOTOR_SPEED_PIN = 3;
// Pin controlling DC motor brakes on the shield.
const int MOTOR_BRAKE_PIN = 9;
// Pin the data line of the 433MHz Receiver is connected to.
// This must be an interrupt-capable pin (2, 3) on the UNO.
const int REMOTE_CONTROL_PIN = 2;

// Servo

// Servo angles for the open and closed and neutral clutch positions.
// The open clutch disconnects the motor from the drivetrain.
// The closed clutch connects the motor to wind up the spider.
// The neutral clutch is in between the two and used when we
// drop the spider to minimize friction with the servo.
const int CLUTCH_CLOSED_ANGLE = 80;
const int CLUTCH_NEUTRAL_ANGLE = 115;
const int CLUTCH_OPEN_ANGLE = 130;

// Motor

// Motor direction. Doesn't really matter, we're just winding up a string
// on a winch, but let's be consistent and use the same direction always.
const int MOTOR_DIRECTION = LOW;

// Timings

// The time we run the motor when winding up the spider in ms.
// You'll most likely need to change this value to adjust
// for motor speed, winch diameter and string length.
const unsigned long SPIDER_WIND_UP_TIME_MS = 60 * 1000UL;
// The time we wait before winding the spider back up after
// letting it drop.
const unsigned long SPIDER_BOTTOM_TIME_MS = 2 * 1000UL;

Servo clutch_servo;
RCSwitch remote_control = RCSwitch();

// A simple busy wait loop that can be used instead
// of delay, so interrupt handling, etc. can still
// take place.
void busyWait(unsigned long timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout) {
    // Wait ...
  }
}

// Waits for the spider drop signal, up to timeout ms.
// Returns true if a drop signal was received, false otherwise.
bool waitForRemote(unsigned long timeout) {
  unsigned long start = millis();
  while(millis() - start < timeout) {
    if (remote_control.available()) {
      int value = remote_control.getReceivedValue();
      Serial.print("Remote control received: ");
      Serial.println(value);
      // Only react to known codes, so other remotes being used
      // nearby don't accidentally drop the spider.
      if (value == -15038 || value == -15032) {
        return true;
      }
    }
  }
  return false;
}

// RCSwitch seemed to yield each code twice with a little
// delay for the Sonoff remote I am using. To avoid
// mis-triggers, this function allows to perform an explicit
// reset of the remote later in the program.
void resetRemote() {
  remote_control.resetAvailable();
}


// Returns the opposite MOTOR_DIRECTION.
int oppositeDirection() {
  return MOTOR_DIRECTION == LOW ? HIGH : LOW;
}

void dropSpider() {
  Serial.println("Dropping spider :)");

  // Stop the motor, in case we were asked to drop while winding.
  analogWrite(MOTOR_SPEED_PIN, 0);
  busyWait(500);

  // Turn a little bit in the opposite direction to the wind-up
  // to ensure we overcome initial friction. In my case the
  // winch diamter was very slow for a pretty light spider
  // so we need a little nudge first.
  analogWrite(MOTOR_DIRECTION_PIN, oppositeDirection());
  analogWrite(MOTOR_SPEED_PIN, 100);
  busyWait(1000);
  analogWrite(MOTOR_SPEED_PIN, 0);

  // Open the clutch to let the spider free-fall.
  // After the initial nudge to disengage the axle
  // put the servo in neutral to minimize friction.
  clutch_servo.write(CLUTCH_OPEN_ANGLE);
  busyWait(500);
  clutch_servo.write(CLUTCH_NEUTRAL_ANGLE);

  // Wait for the spider to fall and people to be excited :-)
  busyWait(SPIDER_BOTTOM_TIME_MS);
}

void windUpSpider() {
  Serial.println("Winding up spider");

  // Disengage brakes.
  digitalWrite(MOTOR_BRAKE_PIN, LOW);

  // Start the motor before engaging the clutch to
  // ensure the gears are moving and the servo won't get
  // stuck trying to push together gears out of position.
  analogWrite(MOTOR_SPEED_PIN, 100);
  busyWait(500);

  // Engage the clutch to connect the motor to the winch.
  clutch_servo.write(CLUTCH_CLOSED_ANGLE);

  // Keep winding up the spider for the defined time.
  // Use waitForRemote here to allow dropping the spider
  // while it's still winding up.
  Serial.println("Winding...");
  bool triggered = waitForRemote(SPIDER_WIND_UP_TIME_MS);
  if (triggered) {
    Serial.println("Winding aborted, drop requested.");
  } else {
    Serial.println("Done winding, spider is up.");
  }

  // Switch off the motor and engage brake.
  // The gearing of the motor I have holds the spider in
  // place when switched off.
  analogWrite(MOTOR_SPEED_PIN, 0);
  digitalWrite(MOTOR_BRAKE_PIN, HIGH);
}

void setup() {
  Serial.begin(9600);

  // Initialize the 433Mhz receiver.
  Serial.print("Remote control is on pin ");
  Serial.print(REMOTE_CONTROL_PIN);
  Serial.print(", interrupt ");
  Serial.println(digitalPinToInterrupt(REMOTE_CONTROL_PIN));
  pinMode(REMOTE_CONTROL_PIN, INPUT);
  remote_control.enableReceive(digitalPinToInterrupt(REMOTE_CONTROL_PIN));

  // Initialize the motor.
  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
  pinMode(MOTOR_SPEED_PIN, OUTPUT);
  digitalWrite(MOTOR_DIRECTION_PIN, MOTOR_DIRECTION);

  // Initialize the servo.
  clutch_servo.attach(CLUTCH_SERVO_PIN);
}

void loop() {
  while(!waitForRemote(10 * 1000UL)) {
    // Wait ...
  }
  dropSpider();
  resetRemote();
  windUpSpider();
}
