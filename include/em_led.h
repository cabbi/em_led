#ifndef EM_LED_H
#define EM_LED_H

#include <Arduino.h>

#include "em_defs.h"
#include "em_timeout.h"


// Abstract led state class 
class EmLedState {
public:
    virtual bool IsOn() = 0;
    virtual void Reset() = 0;
};

// Led ON fixed state 
class EmLedOnState: public EmLedState {
public:
    virtual bool IsOn() override { return true; }
    virtual void Reset() override {};
};

// Led OFF fixed state 
class EmLedOffState: public EmLedState {
public:
    virtual bool IsOn() override { return false; }
    virtual void Reset() override {};
};


// Simple led blinker
class EmLedSimpleBlinker: public EmLedState {
public:
    EmLedSimpleBlinker(uint32_t blinkDurationMs, bool startAsOn=true)
     : m_BlinkingTimeout(blinkDurationMs, false),
       m_StartAsOn(startAsOn),
       m_IsOn(startAsOn) {}
    
    virtual bool IsOn() override;
    virtual void Reset() override;

    void SetDuration(uint32_t millis);

protected:
    bool _isElapsed();
    
    EmTimeout m_BlinkingTimeout;
    bool m_StartAsOn;
    bool m_IsOn;
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
    EmLedSequenceBlinker(uint32_t sequenceMillis[], 
                         uint8_t sequenceLen,
                         bool startAsOn=true)
     : EmLedSimpleBlinker(sequenceMillis[0], startAsOn),
       m_SequenceMillis(sequenceMillis),
       m_SequenceLen(sequenceLen),
       m_CurrentIndex(0) {}
    
    virtual bool IsOn() override;
    virtual void Reset() override;

protected:
    void _incSequence();

private:
    uint32_t* m_SequenceMillis;
    uint8_t m_SequenceLen;
    uint8_t m_CurrentIndex;
};


// The abstract led class.
//
// The real implementation of a led should override the 'Update' method 
// by getting the 'IsOn()' state method.
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
     : m_State(initialState) {}

    // Will get "lto1.exe: internal compiler error: Segmentation fault"
    // if not declaring this virtual abstract method :(
    virtual void Update() = 0;  

    // Sets the led state and calls the 'Update' method if requested.
    void SetState(StateEnum state, bool callUpdate=true) {
        m_State = state;
        ledStates[static_cast<size_t>(m_State)]->Reset();
        if (callUpdate) {
            Update();
        }
    }

    // Gets the led state.
    StateEnum GetState() const {
        return m_State;
    }

protected:
    StateEnum m_State;
};

// The hardware led using a GPIO pin as output.
template <class StateEnum, EmLedState* ledStates[]>
class EmGpioLed: public EmLed<StateEnum, ledStates> {
public:
    EmGpioLed(uint8_t ioPin, StateEnum initialState)
     : EmLed<StateEnum, ledStates>(initialState),
       m_IoPin(ioPin) {
        pinMode(m_IoPin, OUTPUT);
    }

    // Updates the led output port.
    //
    // This method might be called regularly within the main loop in case of
    // defined blinking states
    virtual void Update() override {
        digitalWrite(m_IoPin, ledStates[static_cast<uint8_t>(this->m_State)]->IsOn());
    }

protected:
    uint8_t m_IoPin;
};

#endif