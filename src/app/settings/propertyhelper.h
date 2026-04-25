#pragma once

#define MERGEQT_DEFINE_VALUE(type, valueName, defaultValue) \
    Q_PROPERTY(type valueName READ valueName WRITE set_##valueName NOTIFY valueName##Changed) \
public: \
    type valueName() const { return m_##valueName; } \
    void set_##valueName(type value) \
    { \
        if (m_##valueName == value) \
            return; \
        m_##valueName = value; \
        emit valueName##Changed(); \
    } \
Q_SIGNALS: \
    void valueName##Changed(); \
private: \
    type m_##valueName = defaultValue;
