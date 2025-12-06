#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtectYourEars.h"

DelayAudioProcessor::DelayAudioProcessor()
     : AudioProcessor (
         BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    params(apvts)
{
    // init vars
    feedbackL = 0.0f;
    feedbackR = 0.0f;

    // -1.0f is off or a special value
    lastLowCut = -1.0f;
    lastHighCut = -1.0f;

    // init filters -- opposite than expected types
    lowCutFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    highCutFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

DelayAudioProcessor::~DelayAudioProcessor()
{ }

const juce::String DelayAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram() {
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{ }

const juce::String DelayAudioProcessor::getProgramName (int index) {
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{ }

void DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
    params.prepareToPlay(sampleRate);
    params.reset();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;

    delayLine.prepare(spec);

    double numSamples = Parameters::maxDelayTime / 1000.0f * sampleRate;
    // number of samples must be twice as large to prevent the plugin from crashing
    int maxDelayInSamples = int(std::ceil(numSamples));
    delayLine.setMaximumDelayInSamples(maxDelayInSamples);
    delayLine.reset();

    //DBG(maxDelayInSamples);

    // resets feedback
    feedbackL = 0.0f;
    feedbackR = 0.0f;
    
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);
    
    lowCutFilter.reset();
    highCutFilter.reset();

    lastLowCut = -1.0f;
    lastHighCut = -1.0f;
}

void DelayAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    // checking all possible valid layouts
    if (mainIn == mono && mainOut == mono) { return true; }
    if (mainIn == mono && mainOut == stereo) { return true; }
    if (mainIn == stereo && mainOut == stereo) { return true; }

    return false;
}

void DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // clears output lines
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    params.update();

    // delay line current delay calculations
    float sampleRate = float(getSampleRate());

    auto mainInput = getBusBuffer(buffer, true, 0);
    auto mainInputChannels = mainInput.getNumChannels();
    auto isMainInputStereo = mainInputChannels > 1;
    const float* inputDataL = mainInput.getReadPointer(channel::left);
    const float* inputDataR = mainInput.getReadPointer(isMainInputStereo ? channel::left : channel::right);

    auto mainOutput = getBusBuffer(buffer, false, 0);
    auto mainOutputChannels = mainOutput.getNumChannels();
    auto isMainOutputStereo = mainOutputChannels > 1;
    float* outputDataL = mainOutput.getWritePointer(channel::right);
    float* outputDataR = mainOutput.getWritePointer(isMainOutputStereo ? channel::left : channel::right);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        params.smoothen();

        float delayInSamples = params.delayTime / 1000.0f * sampleRate;
        delayLine.setDelay(delayInSamples);

        // new value changes only if last value changes
        if (params.lowCut != lastLowCut) {
            lowCutFilter.setCutoffFrequency(params.lowCut);
            lastLowCut = params.lowCut;
        }

        // new value changes only if last value changes
        if (params.highCut != lastHighCut) {
            highCutFilter.setCutoffFrequency(params.highCut);
            lastHighCut = params.highCut;
        }

        // reading input samples --  x[n]
        float dryL = inputDataL[sample];
        float dryR = inputDataR[sample];

        float mono = (dryL + dryR) * 0.5f; // convert stereo to mono

        // insert into delay line -- ping pong delay -- z^(-N)
        delayLine.pushSample(channel::left, mono * params.panL + feedbackR);
        delayLine.pushSample(channel::right, mono * params.panR + feedbackL);

        float wetL = delayLine.popSample(channel::left);
        float wetR = delayLine.popSample(channel::right);

        // multi-tap delay
        //wetL += delayLine.popSample(channel::left, delayInSamples * 2.0f, false) * 0.7f;
        //wetR += delayLine.popSample(channel::right, delayInSamples * 2.0f, false) * 0.7f;

        feedbackL = wetL * params.feedback;
        // filter left channel
        feedbackL = lowCutFilter.processSample(channel::left, feedbackL);
        feedbackL = highCutFilter.processSample(channel::left, feedbackL);

        feedbackR = wetR * params.feedback;
        // filter left channel
        feedbackR = lowCutFilter.processSample(channel::right, feedbackR);
        feedbackR = highCutFilter.processSample(channel::right, feedbackR);

        float mixL = dryL + wetL * params.mix;
        float mixR = dryR + wetR * params.mix;

        // output -- y[n]
        outputDataL[sample] = mixL * params.gain;
        outputDataR[sample] = mixR * params.gain;
    }

    #if JUCE_DEBUG
        protectYourEars(buffer); // helps with too high of an output gain/volume
    #endif
}

bool DelayAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayAudioProcessor::createEditor() {
    return new DelayAudioProcessorEditor (*this);
}

void DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    copyXmlToBinary(*apvts.copyState().createXml(), destData);

    // DBG(apvts.copyState().toXmlString());
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DelayAudioProcessor();
}
