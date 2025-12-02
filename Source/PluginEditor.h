#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Parameters.h"
#include "RotaryKnob.h"
#include"LookAndFeel.h"

class DelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DelayAudioProcessorEditor (DelayAudioProcessor&);
    ~DelayAudioProcessorEditor() override;
    
    void paint (juce::Graphics&) override; // draw UI elements
    void resized() override; // used to position and arrage the UI

private:
    DelayAudioProcessor& audioProcessor; // reference to processor object
    MainLookAndFeel mainLF;

    // this connects the UI slider to the 'gain' backend parameter
    RotaryKnob gainKnob{ "Gain", audioProcessor.apvts, gainParamID, true };
    RotaryKnob mixKnob{ "Mix", audioProcessor.apvts, mixParamID };
    RotaryKnob delayTimeKnob{ "Time", audioProcessor.apvts, delayTimeParamID };

    juce::GroupComponent delayGroup, feedbackGroup, outputGroup;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioProcessorEditor)
};
