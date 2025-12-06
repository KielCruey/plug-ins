#include <JuceHeader.h> // can use namespace juce::

#include "Utilities.h"
#include "Parameters.h"

juce::String stringFromMilliseconds(float value, int) {
    if (value < 10.0f)
        return juce::String(value, 2) + " ms";
    else if (value < 100.0f)
        return juce::String(value, 1) + " ms";
    else if (value < 1000.0f)
        return juce::String(int(value)) + " ms";
    else
        return juce::String(value * 0.001f, 2) + " s";
}

float millisecondsFromString(const juce::String& text) {
    float value = text.getFloatValue();

    if (!text.endsWithIgnoreCase("ms"))
        if (text.endsWithIgnoreCase("s") || value < Parameters::minDelayTime)
            return value * 1000.0f;

    return value;
}

juce::String stringFromDecibels(float value, int) {
    return juce::String(value, 1) + " dB";
}

juce::String stringFromPercent(float value, int){
    return juce::String(int(value)) + " %";
}

juce::String stringFromHz(float value, int) {
    if (value < 1000.0f)
        return juce::String(int(value)) + " Hz";
    else if (value < 10000.0f)
        return juce::String(value / 1000.0f, 2) + " k";
    else
        return juce::String(value / 1000.0f, 1) + " k";
}

float hzFromString(const juce::String& str) {
    float value = str.getFloatValue();

    if (value < 20.0f) 
        return value * 1000.0f;

    return value;
}