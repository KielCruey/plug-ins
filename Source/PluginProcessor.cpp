#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProtectYourEars.h"

DelayAudioProcessor::DelayAudioProcessor()
     : AudioProcessor (BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    params(apvts)
{ }

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
    int maxDelayInSamples = int(std::ceil(numSamples) * 2); // if remainder, then round up
    delayLine.setMaximumDelayInSamples(maxDelayInSamples);
    delayLine.reset();

    //DBG(maxDelayInSamples);

    // resets feedback
    feedbackL = 0.0f;
    feedbackR = 0.0f;
}

void DelayAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

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

    float* channelDataL = buffer.getWritePointer(channel::left);
    float* channelDataR = buffer.getWritePointer(channel::right);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        params.smoothen();

        float delayInSamples = params.delayTime / 1000.0f * sampleRate;
        delayLine.setDelay(delayInSamples);

        // reading input samples --  x[n]
        float dryL = channelDataL[sample];
        float dryR = channelDataR[sample];

        // insert into delay line -- z^(-N)
        delayLine.pushSample(channel::left, dryL + feedbackL);
        delayLine.pushSample(channel::right, dryR + feedbackR);

        float wetL = delayLine.popSample(channel::left);
        float wetR = delayLine.popSample(channel::right);

        // multi-tap delay
        //wetL += delayLine.popSample(0, delayInSamples * 2.0f, false) * 0.7f;
        //wetR += delayLine.popSample(1, delayInSamples * 2.0f, false) * 0.7f;

        feedbackL = wetL * params.feedback;
        feedbackR = wetR * params.feedback;

        float mixL = dryL + wetL * params.mix;
        float mixR = dryR + wetR * params.mix;

        // output -- y[n]
        channelDataL[sample] = mixL * params.gain;
        channelDataR[sample] = mixR * params.gain;
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
    DBG(apvts.copyState().toXmlString());
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
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
