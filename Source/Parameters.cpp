#include "Parameters.h"

#include "Utilities.h"

template<typename T>
static void Parameters::castParameter(juce::AudioProcessorValueTreeState& apvts, 
                            const juce::ParameterID& id, T& destination) {
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts) {
    castParameter(apvts, gainParamID, gainParam);
    castParameter(apvts, delayTimeParamID, delayTimeParam);
    castParameter(apvts, mixParamID, mixParam);
    castParameter(apvts, feedbackParamID, feedbackParam);
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        gainParamID,
        "Output Gain",
        juce::NormalisableRange<float> { minOutputGain, maxOutputGain },
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction(stringFromDecibels)
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
        juce::NormalisableRange<float> { 0.0f, 100.0f, 1.0f },
        50.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction(stringFromPercent)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        feedbackParamID,
        "Feedback",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction(stringFromPercent)
    ));

    return layout;
}

void Parameters::prepareToPlay(double sampleRate) noexcept {
    double duration = 0.02;
    feedbackSmoother.reset(sampleRate, duration);
    
    coeff = 1.0f - std::exp(-1.0f / (0.2f * float(sampleRate))); // exponential smoothing -- 200 ms

    gainSmoother.reset(sampleRate, duration);
    mixSmoother.reset(sampleRate, duration);
    
}

void Parameters::reset() noexcept {
    gain = 0.0f;
    delayTime = 0.0f;
    mix = 1.0f;
    feedback = 0.0f;

    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    mixSmoother.setCurrentAndTargetValue(mixParam->get() * 0.01f); // converts 100% to 1 and 0% to 0.0 for example
    feedbackSmoother.setCurrentAndTargetValue(feedbackParam->get() * 0.01f);
}

void Parameters::update() noexcept {
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));

    targetDelayTime = delayTimeParam->get();
    if (delayTime == 0.0f)
        delayTime = targetDelayTime;

    mixSmoother.setTargetValue(mixParam->get() * 0.01f);
    feedbackSmoother.setTargetValue(mixParam->get() * 0.01f);
}

void Parameters::smoothen() noexcept {
    gain = gainSmoother.getNextValue();

    delayTime += (targetDelayTime - delayTime) * coeff;

    mix = mixSmoother.getNextValue();
    feedback = feedbackSmoother.getNextValue();
}