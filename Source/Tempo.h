#pragma once

#include <JuceHeader.h>

class Tempo
{
public: 
	Tempo();

	void reset() noexcept;
	void update(const juce::AudioPlayHead* playHead) noexcept;
	double getMillisecondsForNoteLength(int index) const noexcept;

	double getTempo();

private:
	double bpm;
};
