#include <stdio.h>
#include "em_defs.h"
#include "em_led.h"


// The led states
enum class MyLedState: uint8_t {
    off = 0,
    on = 1,
    bad_status_blink = 2,
    good_status_blink = 3,
};

// All possible led states
EmLedOffState ledOffState;
EmLedOnState ledOnState;
EmLedSimpleBlinker badStatusBlink(500);
// Long on + 3 quick blinks
uint32_t sequence[] = {1000, 200, 100, 200, 100, 200, 100, 200};
EmLedSequenceBlinker goodStatusBlink(sequence, SIZE_OF(sequence), true);

// State array respecting 'LedStates' values as array position/index
EmLedState* ledStates[] = {&ledOffState,      // 0 -> off
                           &ledOnState,       // 1 -> on
                           &badStatusBlink,   // 2 -> bad_status_blink
                           &goodStatusBlink}; // 3 -> good_status_bli

// GPIO leds
typedef EmGpioLed<MyLedState, ledStates> MyLed;
MyLed led1(3, MyLedState::off);
MyLed led2(4, MyLedState::off);
MyLed led3(5, MyLedState::off);
EmUpdatable* leds[] = {&led1, &led2, &led3};

EmUpdater<leds, SIZE_OF(leds)> updater;

void setup() {
    led1.SetState(MyLedState::on);
    led2.SetState(MyLedState::bad_status_blink);
    led3.SetState(MyLedState::good_status_blink);
}


void loop() {
    // Updates the led status (i.e. switched on or off)
    updater.Update();
}