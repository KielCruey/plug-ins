#include "PluginProcessor.h"
#include "PluginEditor.h"

DelayAudioProcessorEditor::DelayAudioProcessorEditor (DelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // setting up UI (knobs) in groups
    delayGroup.setText("Delay");
    delayGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    delayGroup.addAndMakeVisible(delayTimeKnob); // adding delayTime to group
    addAndMakeVisible(delayGroup);

    feedbackGroup.setText("Feedback");
    feedbackGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    feedbackGroup.addAndMakeVisible(feedbackKnob);
    feedbackGroup.addAndMakeVisible(stereoKnob);
    addAndMakeVisible(feedbackGroup);

    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputGroup);

    // changing color
    gainKnob.slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::green);

    setSize (500, 330);

    setLookAndFeel(&mainLF);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void DelayAudioProcessorEditor::paint (juce::Graphics& g) {
    // adding texture to UI
    auto noise = juce::ImageCache::getFromMemory(BinaryData::Noise_png, BinaryData::Noise_pngSize);
    auto fillType = juce::FillType(noise, juce::AffineTransform::scale(0.5f));
    g.setFillType(fillType);
    g.fillRect(getLocalBounds());

    // drawing the banner on the plug-in's UI
    auto rect = getLocalBounds().withHeight(40);
    g.setColour(Colors::header);
    g.fillRect(rect);

    auto image = juce::ImageCache::getFromMemory(BinaryData::Logo_png, BinaryData::Logo_pngSize);

    int destWidth = image.getWidth() / 2;
    int destHeight = image.getHeight() / 2;

    g.drawImage(image, getWidth() / 2 - destWidth / 2, 0, destWidth, destHeight,
        0, 0, image.getWidth(), image.getHeight());
}

void DelayAudioProcessorEditor::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();

    int y = 50; // y position
    int height = bounds.getHeight() - 60;

    delayGroup.setBounds(10, y, 110, height);
    outputGroup.setBounds(bounds.getWidth() - 160, y, 150, height);
    feedbackGroup.setBounds(delayGroup.getRight() + 10, y, outputGroup.getX() - delayGroup.getRight() - 20, height);

    // position the knobs inside the groups
    delayTimeKnob.setTopLeftPosition(20, 20); // relative to the top left of the UI group created
    mixKnob.setTopLeftPosition(20, 20); // relative to the top left of the UI group created
    gainKnob.setTopLeftPosition(mixKnob.getX(), mixKnob.getBottom() + 10); // relative to the top left of the UI group created
    feedbackKnob.setTopLeftPosition(20, 20); // relative to the top left of the UI group created
    stereoKnob.setTopLeftPosition(feedbackKnob.getRight() + 20, 20);
}