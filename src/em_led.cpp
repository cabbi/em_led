#include "em_led.h"


bool EmLedSimpleBlinker::IsOn() 
{
    if (_isElapsed()) {
        m_IsOn = !m_IsOn;
    }
    return m_IsOn; 
}

void EmLedSimpleBlinker::Reset() 
{
    m_IsOn = m_StartAsOn;
    m_BlinkingTimeout.Restart();
}

void EmLedSimpleBlinker::SetDuration(uint32_t millis) 
{
    m_BlinkingTimeout.SetTimeout(millis, true);
}

bool EmLedSimpleBlinker::_isElapsed()
{
    return m_BlinkingTimeout.IsElapsed(true);
}
    
bool EmLedSequenceBlinker::IsOn() 
{
    if (_isElapsed()) {
        m_IsOn = !m_IsOn;
        _incSequence();
    }
    return m_IsOn; 
}

void EmLedSequenceBlinker::Reset() 
{
    m_CurrentIndex = m_SequenceLen;
    _incSequence();
    EmLedSimpleBlinker::Reset();
}

void EmLedSequenceBlinker::_incSequence() 
{
    if (m_CurrentIndex < m_SequenceLen-1) {
        m_CurrentIndex++;
    } else {
        m_CurrentIndex = 0;
    }
    SetDuration(m_SequenceMillis[m_CurrentIndex]);
}