#pragma once

#include <JuceHeader.h>

#include "Parameters.h"

enum channel {left, right};

class DelayAudioProcessor  : public juce::AudioProcessor
{
public:
    DelayAudioProcessor();
    ~DelayAudioProcessor() override;

    // ====== functions ======
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    #ifndef JucePlugin_PreferredChannelConfigurations
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    #endif

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    void changeProgramName(int index, const juce::String& newName) override;
    double getTailLengthSeconds() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    // ====== getters/setters ======
    void getStateInformation(juce::MemoryBlock& destData) override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    const juce::String getProgramName(int index) override;
    const juce::String getName() const override;

    void setStateInformation(const void* data, int sizeInBytes) override;
    void setCurrentProgram(int index) override;
    
    // parameter helper
    juce::AudioProcessorValueTreeState apvts{ 
        *this, nullptr, "Parameters", Parameters::createParameterLayout() 
    };

private:
    Parameters params;

    // note -- dsp object have state, reset them when needed
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;

    // StateVariableTPTFilter can be configured to high, low, or band pass filter
    juce::dsp::StateVariableTPTFilter<float> lowCutFilter; 
    juce::dsp::StateVariableTPTFilter<float> highCutFilter;

    float feedbackL;
    float feedbackR;

    float lastLowCut;
    float lastHighCut;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessor)
};
