/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "netlist.h"
//==============================================================================
/**
*/
class Test_MNAlgorithm_v1_4AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Test_MNAlgorithm_v1_4AudioProcessor();
    ~Test_MNAlgorithm_v1_4AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;



    void loadNetlistFile(const juce::String& path);

    double currentSampleRate = 0.0;
private:

    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float>* inputGainParameter = nullptr;
    std::atomic<float>* outputGainParameter = nullptr;
    std::atomic<float>* mixPercentageParameter = nullptr;
    std::atomic<float>* oversamplingParameter = nullptr;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler[3];

    Netlist netlist;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Test_MNAlgorithm_v1_4AudioProcessor)
};
