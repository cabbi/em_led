#include "em_led.h"


bool EmLedSimpleBlinker::isOn() 
{
    if (isElapsed_()) {
        m_isOn = !m_isOn;
    }
    return m_isOn; 
}

void EmLedSimpleBlinker::reset() 
{
    m_isOn = m_startAsOn;
    m_blinkingTimeout.restart();
}

void EmLedSimpleBlinker::setDuration(uint32_t millis) 
{
    m_blinkingTimeout.setTimeout(millis, true);
}

bool EmLedSimpleBlinker::isElapsed_()
{
    return m_blinkingTimeout.isElapsed(true);
}

bool EmLedSequenceBlinker::isOn() 
{
    if (isElapsed_()) {
        m_isOn = !m_isOn;
        incSequence_();
    }
    return m_isOn; 
}

void EmLedSequenceBlinker::reset() 
{
    m_currentIndex = m_sequenceLen;
    incSequence_();
    EmLedSimpleBlinker::reset();
}

void EmLedSequenceBlinker::incSequence_() 
{
    if (m_currentIndex < m_sequenceLen-1) {
        m_currentIndex++;
    } else {
        m_currentIndex = 0;
    }
    setDuration(m_sequenceMillis[m_currentIndex]);
}