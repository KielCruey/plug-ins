#pragma once

#include <JuceHeader.h> // can use namespace juce::

const juce::ParameterID gainParamID{ "gain", 1 };
const juce::ParameterID delayTimeParamID{ "delayTime", 1 };
const juce::ParameterID mixParamID{ "mix", 1 };
const juce::ParameterID feedbackParamID{ "feedback", 1 };
const juce::ParameterID stereoParamID{ "stereo", 1 };
const juce::ParameterID lowCutParamID{ "lowCut", 1 };
const juce::ParameterID highCutParamID{ "highCut", 1 };

class Parameters
{
public:
	Parameters(juce::AudioProcessorValueTreeState& apvts);

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	
	void prepareToPlay(double sampleRate) noexcept;
	void reset() noexcept;
	void update() noexcept; // triggers on every block
	void smoothen() noexcept;

	// ======= helper functions =======
	template<typename T>
	static void castParameter(juce::AudioProcessorValueTreeState& apvts, const juce::ParameterID& id, T& destination);

	// ======= constants =======
	static constexpr float minDelayTime = 5.0f;
	static constexpr float maxDelayTime = 5000.0f;
	static constexpr float minOutputGain = -36.0f;
	static constexpr float maxOutputGain = 12.0f;

	// ======= variables =======
	float gain;
	float delayTime;
	float mix;
	float feedback;
	float panL;
	float panR;
	float lowCut;
	float highCut;

private:
	float targetDelayTime; // value that the one-pole filter is trying to reach
	float coeff; // one-pole smoothing -- how fast the smoothing happens

	juce::AudioParameterFloat* gainParam;
	juce::AudioParameterFloat* mixParam;
	juce::AudioParameterFloat* feedbackParam;
	juce::AudioParameterFloat* stereoParam;
	juce::AudioParameterFloat* lowCutParam;
	juce::AudioParameterFloat* highCutParam;

	juce::AudioParameterFloat* delayTimeParam;

	// smoothing helps prevent audio clicks
	juce::LinearSmoothedValue<float> gainSmoother; 
	juce::LinearSmoothedValue<float> mixSmoother;
	juce::LinearSmoothedValue<float> feedbackSmoother;
	juce::LinearSmoothedValue<float> stereoSmoother;
	juce::LinearSmoothedValue<float> lowCutSmoother;
	juce::LinearSmoothedValue<float> highCutSmoother;
};
