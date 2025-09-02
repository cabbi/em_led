#ifndef EM_LED_H
#define EM_LED_H

#include <Arduino.h>

#include "em_defs.h"
#include "em_timeout.h"


// Abstract led state class 
class EmLedState {
public:
    virtual bool isOn() = 0;
    virtual void reset() = 0;
};

// Led ON fixed state 
class EmLedOnState: public EmLedState {
public:
    virtual bool isOn() override { return true; }
    virtual void reset() override {};
};

// Led OFF fixed state 
class EmLedOffState: public EmLedState {
public:
    virtual bool isOn() override { return false; }
    virtual void reset() override {};
};


// Simple led blinker
class EmLedSimpleBlinker: public EmLedState {
public:
    EmLedSimpleBlinker(uint32_t blinkDurationMs, bool startAsOn=true)
     : m_blinkingTimeout(blinkDurationMs, false),
       m_startAsOn(startAsOn),
       m_isOn(startAsOn) {}
    
    virtual bool isOn() override;
    virtual void reset() override;

    void setDuration(uint32_t millis);

protected:
    bool isElapsed_();
    
    EmTimeout m_blinkingTimeout;
    bool m_startAsOn;
    bool m_isOn;
};

// Sequence led blinker
// 
// You set the alternate state timeouts and the initial sequence state.
// Example:
//    // Long on + 3 quick blinks
//    uint32_t sequence[] = {1000, 200, 100, 200, 100, 200, 100, 200};
//    EmLedSequenceBlinker goodStatusBlink(sequence, SIZE_OF(sequence), true);
class EmLedSequenceBlinker: public EmLedSimpleBlinker {
public:
    EmLedSequenceBlinker(const uint32_t sequenceMillis[], 
                         uint8_t sequenceLen,
                         bool startAsOn=true)
     : EmLedSimpleBlinker(sequenceMillis[0], startAsOn),
       m_sequenceMillis(sequenceMillis),
       m_sequenceLen(sequenceLen),
       m_currentIndex(0) {}
    
    virtual bool isOn() override;
    virtual void reset() override;

protected:
    void incSequence_();

private:
    const uint32_t* m_sequenceMillis;
    uint8_t m_sequenceLen;
    uint8_t m_currentIndex;
};


// The abstract led class.
//
// The real implementation of a led should override the 'update' method 
// by getting the 'isOn()' state method.
//
// The StateEnum values should be the index within the 'ledStates' array.
// Led class implemented as template to reduce the RAM footprint.
//
// Example:
//    // The led states
//    enum class LedStates: uint8_t {
//        off = 0,
//        on = 1,
//        bad_status_blink = 2,
//        good_status_blink = 3,
//    };
// 
//    // All possible led states
//    EmLedOffState ledOffState;
//    EmLedOnState ledOnState;
//    EmLedSimpleBlinker badStatusBlink(500);
//    // Long on + 3 quick blinks
//    uint32_t sequence[] = {1000, 200, 100, 200, 100, 200, 100, 200};
//    EmLedSequenceBlinker goodStatusBlink(sequence, SIZE_OF(sequence), true);
//
//    // State array respecting 'LedStates' values as array position/index
//    EmLedState* ledStates[] = {&ledOffState,      // 0 -> off
//                               &ledOnState,       // 1 -> on
//                               &badStatusBlink,   // 2 -> bad_status_blink
//                               &goodStatusBlink}; // 3 -> good_status_blink
// 
//    // Module leds
//    EmGpioLed<LedStates, ledStates> led(5, LedStates::off);
//
template <class StateEnum, EmLedState* ledStates[]>
class EmLed: public EmUpdatable {
public:
    EmLed(StateEnum initialState)
     : m_state(initialState) {}

    // Will get "lto1.exe: internal compiler error: Segmentation fault"
    // if not declaring this virtual abstract method :(
    virtual void update() = 0;  

    // Sets the led state and calls the 'Update' method if requested.
    void setState(StateEnum state, bool callUpdate=true) {
        m_state = state;
        ledStates[static_cast<size_t>(m_state)]->reset();
        if (callUpdate) {
            update();
        }
    }

    // Gets the led state.
    StateEnum getState() const {
        return m_state;
    }

protected:
    StateEnum m_state;
};

// The hardware led using a GPIO pin as output.
template <class StateEnum, EmLedState* ledStates[]>
class EmGpioLed: public EmLed<StateEnum, ledStates> {
public:
    EmGpioLed(uint8_t ioPin, StateEnum initialState)
     : EmLed<StateEnum, ledStates>(initialState),
       m_ioPin(ioPin) {
        pinMode(m_ioPin, OUTPUT);
    }

    // Updates the led output port.
    //
    // This method might be called regularly within the main loop in case of
    // defined blinking states
    virtual void update() override {
        digitalWrite(m_ioPin, ledStates[static_cast<uint8_t>(this->m_state)]->isOn());
    }

protected:
    uint8_t m_ioPin;
};

#endif