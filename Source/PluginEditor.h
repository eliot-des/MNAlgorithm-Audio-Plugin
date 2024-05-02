/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Test_MNAlgorithm_v1_4AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Test_MNAlgorithm_v1_4AudioProcessorEditor 
    (Test_MNAlgorithm_v1_4AudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~Test_MNAlgorithm_v1_4AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Test_MNAlgorithm_v1_4AudioProcessor& audioProcessor;

    juce::Slider inputgainSlider;
    juce::Slider outputgainSlider;
    juce::Slider mixSlider;
    juce::ComboBox osComboBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   inputgainAttachement;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   outputgainAttachement;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   mixAttachement;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osComboBoxAttachment;

    juce::Label inputgainLabel;
    juce::Label outputgainLabel;
    juce::Label mixLabel;
    juce::Label osLabel;

    juce::TextButton openNetlistButton;
    juce::FileChooser fileChooser{ "Select a netlist file to open...", juce::File{}, "*.txt" };
    std::unique_ptr<juce::TextEditor> textContent;

    void openNetlistButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Test_MNAlgorithm_v1_4AudioProcessorEditor)
};
