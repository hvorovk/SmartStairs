#include <Tlc5940.h>
#include <HCSR04.h>

// Pin setup
#define LIGHT 7             // Light sensor
#define UP_SONIC_TRIG 5     // Trigger pin for the lower ultrasonic
#define UP_SONIC_ECHO 6     // Echo pin for the lower ultrasonic
#define DOWN_SONIC_TRIG 2   // Trigger pin for the upper ultrasonic
#define DOWN_SONIC_ECHO 4   // Echo pin for the upper ultrasonic
// Turn on light
#define DELAY_BETWEEN_STAIRS 0      // Delay between light on every stair and light off every stair (microseconds)
#define SPEED_LIGHT_STAIRS 1000.0   // Speed light on on every stair (microseconds)
#define SPEED_PUT_OUT_LIGHT 1000.0  // Speed light off on every stair (microseconds)
#define DELAY_AFTER_ON 5000         // Delay after light on all stair to light off (microseconds)
// Light Control
#define ON_LIGHT  2047          // Full light 0..4095
#define OFF_LIGHT  0            // Off light 0..4095
#define START_END_LIGHT 300     // First and last stair light 0..4095
// SysSetting
#define STAIRS_NUMBER 15          // Stair amount
#define UP_OPENNING_WIDTH 60      // Width of upper oppening
#define DOWN_OPENNING_WIDTH 60    // Width of lower oppening
#define DELAY_UPDATE_SENSORS 50   // Delay update sensors
#define AFTER_ON_ALL_PUT_OUT true // If true after all stairs light on all stair light off together if false
                                  // they light off in the opposite direction in which they light up4

UltraSonicDistanceSensor ultrasonicUp(UP_SONIC_TRIG, UP_SONIC_ECHO);
UltraSonicDistanceSensor ultrasonicDown(DOWN_SONIC_TRIG, DOWN_SONIC_ECHO);

int direction = 0;

void setup()
{
  Serial.begin(9600);
  
  pinMode(LIGHT, INPUT); 
  Tlc.init(OFF_LIGHT);
  Tlc.set(0, START_END_LIGHT);
  Tlc.set(STAIRS_NUMBER-1, START_END_LIGHT);
  Tlc.update(); 
}

void clear_led() {
  Tlc.setAll(OFF_LIGHT);
  Tlc.set(0, START_END_LIGHT);
  Tlc.set(STAIRS_NUMBER-1, START_END_LIGHT);
  Tlc.update();
}

bool getLightStatus() {
  return digitalRead(LIGHT) == 1;
}

void turnOnStair(int channel) {
  for(double i = OFF_LIGHT; i <= ON_LIGHT; i += ON_LIGHT/ SPEED_LIGHT_STAIRS) {
    if ((channel == 0 || channel == STAIRS_NUMBER-1) && i <= START_END_LIGHT + ON_LIGHT/ SPEED_PUT_OUT_LIGHT) {
      Tlc.set(channel, START_END_LIGHT);
    } else {
      Tlc.set(channel, (int)i);
    }
    Tlc.update();
    delay(1);
  }
}

void turnOffStair(int channel = -1) {
  for(double i = ON_LIGHT; i >= OFF_LIGHT; i -= ON_LIGHT/ SPEED_PUT_OUT_LIGHT) {
    if (channel == -1) {
      Tlc.setAll(i);
      if ( i <= START_END_LIGHT + ON_LIGHT/ SPEED_PUT_OUT_LIGHT ) {
        Tlc.set(0, START_END_LIGHT);
        Tlc.set(STAIRS_NUMBER-1, START_END_LIGHT);
      }
      Tlc.update();
    } else {
      Tlc.set(channel, (int)i);
      Tlc.update();
      if ((channel == 0 || channel == STAIRS_NUMBER-1) && i <= START_END_LIGHT + ON_LIGHT/ SPEED_PUT_OUT_LIGHT) {
        Tlc.set(channel, START_END_LIGHT);
        Tlc.update();
      }
    }
    delay(1);
  }
}
 
void loop()
{
  if(direction == 1) {
    for (int channel = 0; channel < STAIRS_NUMBER; channel += 1) {
      turnOnStair(channel);
      delay(DELAY_BETWEEN_STAIRS);
    }
    delay(DELAY_AFTER_ON);
    if (AFTER_ON_ALL_PUT_OUT) {
      Tlc.update();
    } else {
      for (int channel = STAIRS_NUMBER-1; channel >= 0; channel -= 1) {
        turnOffStair(channel);
        delay(DELAY_BETWEEN_STAIRS);
      }
    }
    direction = 0;
  } else if (direction == -1) {
    for (int channel = STAIRS_NUMBER-1; channel >= 0; channel -= 1) {
      turnOnStair(channel);
      delay(DELAY_BETWEEN_STAIRS);
    }
    delay(DELAY_AFTER_ON);
    if (AFTER_ON_ALL_PUT_OUT) {
      turnOffStair();
    } else {
      for (int channel = 0; channel < STAIRS_NUMBER; channel += 1) {
        turnOffStair(channel);
        delay(DELAY_BETWEEN_STAIRS);
      }
    }
    direction = 0;
  } else {
    direction = 0;
    if (getLightStatus()) {
      if (ultrasonicUp.measureDistanceCm() < UP_OPENNING_WIDTH) {
        direction = -1;
      } else if (ultrasonicDown.measureDistanceCm() < DOWN_OPENNING_WIDTH) {
        direction = 1;
      } else {
        clear_led();
      }
    } else {
      clear_led();
    }
  }
  delay(DELAY_UPDATE_SENSORS);
}
