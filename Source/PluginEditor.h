#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Parameters.h"
#include "RotaryKnob.h"
#include "LookAndFeel.h"

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

    // this connects the UI sliders to the backend parameters in the apvts
    RotaryKnob gainKnob{ "Gain", audioProcessor.apvts, gainParamID, true };
    RotaryKnob mixKnob{ "Mix", audioProcessor.apvts, mixParamID };
    RotaryKnob delayTimeKnob{ "Time", audioProcessor.apvts, delayTimeParamID };
    RotaryKnob feedbackKnob{ "Feedback", audioProcessor.apvts, feedbackParamID, true };
    RotaryKnob stereoKnob{ "Stereo", audioProcessor.apvts, stereoParamID, true };
    RotaryKnob lowCutKnob{ "Low Cut", audioProcessor.apvts, lowCutParamID };
    RotaryKnob highCutKnob{ "High Cut", audioProcessor.apvts, highCutParamID };
    RotaryKnob delayNoteKnob{ "Note", audioProcessor.apvts, delayNoteParamID };

    // UI group for the knobs
    juce::GroupComponent delayGroup, feedbackGroup, outputGroup;
    
    juce::TextButton tempoSyncButton;

    // attachs button to ID and the state of the parameter
    juce::AudioProcessorValueTreeState::ButtonAttachment tempoSyncAttachment{
        audioProcessor.apvts, tempoSyncParamID.getParamID(), tempoSyncButton
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayAudioProcessorEditor)
};
