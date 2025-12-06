#include "Parameters.h"
#include "Utilities.h"
#include "DSP.h"

template<typename T>
static void Parameters::castParameter(juce::AudioProcessorValueTreeState& apvts, 
                            const juce::ParameterID& id, T& destination) 
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination); // parameter does not exist or wrong type
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts) {
    // initialize variables
    gain = 0.0f;
    delayTime = 0.0f;
    mix = 1.0f; // 0-1 values only
    feedback = 0.0f;
    panL = 0.0f;
    panR = 1.0f;
    lowCut = 20.0f;
    highCut = 20000.0f;

    targetDelayTime = 0.0f;
    coeff = 0.0f;

    // grabbing parameter from apvts
    castParameter(apvts, gainParamID, gainParam);
    castParameter(apvts, delayTimeParamID, delayTimeParam);
    castParameter(apvts, mixParamID, mixParam);
    castParameter(apvts, feedbackParamID, feedbackParam);
    castParameter(apvts, stereoParamID, stereoParam);
    castParameter(apvts, lowCutParamID, lowCutParam);
    castParameter(apvts, highCutParamID, highCutParam);
}

// adds the given parameter to the juce framework
juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        gainParamID,
        "Output Gain",
        juce::NormalisableRange<float> { -12.0f, 12.0f },
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        delayTimeParamID,
        "Delay Time",
        juce::NormalisableRange<float> { minDelayTime, maxDelayTime, 0.001f, 0.25f },
        100.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(stringFromMilliseconds)
        .withValueFromStringFunction(millisecondsFromString)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        mixParamID,
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        feedbackParamID,
        "Feedback",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        stereoParamID,
        "Stereo",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        lowCutParamID,
        "Low Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
        20.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(stringFromHz)
        .withValueFromStringFunction(hzFromString)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        highCutParamID,
        "High Cut",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),
        20000.0f,
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction(stringFromHz)
        .withValueFromStringFunction(hzFromString)
    ));

    return layout;
}

void Parameters::prepareToPlay(double sampleRate) noexcept {
    double duration = 0.02;
    feedbackSmoother.reset(sampleRate, duration);
    
    coeff = 1.0f - std::exp(-1.0f / (0.2f * float(sampleRate))); // exponential smoothing -- 200 ms

    gainSmoother.reset(sampleRate, duration);
    mixSmoother.reset(sampleRate, duration);
    stereoSmoother.reset(sampleRate, duration);
    lowCutSmoother.reset(sampleRate, duration);
    highCutSmoother.reset(sampleRate, duration);
}

void Parameters::reset() noexcept {
    gain = 0.0f;
    delayTime = 0.0f;
    mix = 1.0f;
    feedback = 0.0f;
    panL = 0.0f;
    panR = 1.0f;
    lowCut = 20.0f;
    highCut = 20000.0f;

    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    mixSmoother.setCurrentAndTargetValue(mixParam->get() * 0.01f); // converts 100% to 1 and 0% to 0.0 for example
    feedbackSmoother.setCurrentAndTargetValue(feedbackParam->get() * 0.01f);
    stereoSmoother.setCurrentAndTargetValue(stereoParam->get() * 0.01f);
    lowCutSmoother.setCurrentAndTargetValue(stereoParam->get());
    highCutSmoother.setCurrentAndTargetValue(stereoParam->get());
}

void Parameters::update() noexcept {
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));

    targetDelayTime = delayTimeParam->get();
    if (delayTime == 0.0f)
        delayTime = targetDelayTime;

    mixSmoother.setTargetValue(mixParam->get() * 0.01f);
    feedbackSmoother.setTargetValue(mixParam->get() * 0.01f);
    stereoSmoother.setTargetValue(stereoParam->get() * 0.01f);
    lowCutSmoother.setTargetValue(stereoParam->get());
    highCutSmoother.setTargetValue(stereoParam->get());
}

void Parameters::smoothen() noexcept {
    gain = gainSmoother.getNextValue();

    delayTime += (targetDelayTime - delayTime) * coeff;

    mix = mixSmoother.getNextValue();
    feedback = feedbackSmoother.getNextValue();

    panningEqualPower(stereoSmoother.getNextValue(), panL, panR);

    lowCut = lowCutSmoother.getNextValue();
    highCut = highCutSmoother.getNextValue();
}