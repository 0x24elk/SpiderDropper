# SpiderDropper

Arduino program to control a small Fischertechnik assembly to suddenly drop (and wind back up) a rubber spider for a little Halloween scare.

## Hardware / BOM

* Arduino Uno
* [MotorShield v3](https://store.arduino.cc/products/arduino-motor-shield-rev3)
* [Fischertechnik Motor XM](https://www.fischertechnik.de/de-de/produkte/spielzeug/zubehoer/505282-motor-set-xm)
* Servo, e.g., the [SM-S2309S](https://servodatabase.com/servo/springrc/sm-s2309s) that came with the [Arduino Starter Kit](https://store.arduino.cc/products/arduino-starter-kit-multi-language)
* Fischertechnik Bricks, Gears, etc. to build the mechanical assembly. I used the parts from the [Creative Box Mechanics](https://www.fischertechnik.de/de-de/produkte/spielzeug/zubehoer/554196-creative-box-mechanics)
* 433 MHz Receiver. e.g., this [purecrea](https://www.bastelgarage.ch/433mhz-rf-funkmodul-set-rx470-4-wl102-341) model.
* 433 MHz Garage Remote, e.g. this [Sonoff Model](https://www.digitec.ch/de/s1/product/sonoff-4-key-433-remote-infrarot-fernbedienung-15992009)
* 9V Power Supply

I want to let the spider free-fall when the remote is pressed, and then use the motor
to wind it back up for the next scare. The Fischertechnik motor does the winding (by driving
an axis with a spool via a 1:1 gearing with the 20-tooth gears), and we use the servo
to slide the axis and disengage the gear(s) for the free-fall. The motor shield is needed because both the motor (9V / 3 W) and the servo can draw more current than the Arduino can supply,
running the risk of damaging the unit otherwise.

## Connections

I connected the _Motor Shield_ to a 9V power supply and let the shield power the Arduino,
which lets the shield draw a fair lot of current without needing to run that throught the
Arduino. The rest of the components got connected like this (I used a breadboard for the
433 MHz receiver)

| Motor Shield | Part                            |
+--------------+---------------------------------+
| GND          | GND of the 433 MHz receiver     |
| 5V           | 5V of the 433 MHz receiver      |
| Pin 2        | Data pin of the 433 MHz receiver|
| Motor A      | Fischertechnik Motor            |
| PWM Out 5    | Servo (3-pin connector)         |

## Sketch

The sketch is straightfoward, with the core loop being:

```C
void loop() {
  while(!waitForRemote(10 * 1000UL)) {
    // Wait ...
  }
  dropSpider();
  resetRemote();
  windUpSpider();
}
```

The remote seems to use interrupts, so we use a timer-based waiting
approach. And I had to futz around a bit to work around issues like
the 433 MHz remote reporting each button press twice (so the sketch
explicitly resets the receiver after each code), and with things like
the small spool sometimes getting stuck for the free-fall, so we're
giving it a nudge with the motor before disengaging.

**Things to adjust** 

You will probably need to adjust these constants at the top of the file
to tune things for your specific setup:

* The **servo angles** for the engaged/disengaged positions of the sliding axis (`CLUTCH_*_ANGLE`).
* The **winding time** depending on the length of your string (`SPIDER_WIND_UP_TIME_MS`)

Other things you can play with:

* The `*_PIN` constants, if you didn't use the same pinout as above.
* The `MOTOR_DIRECTION` (if you need to wind things in a particular way)
* The `SPIDER_BOTTOM_TIME_MS` if you want to adjust the wait before the spider moves back up.

