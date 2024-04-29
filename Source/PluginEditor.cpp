/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Test_MNAlgorithm_v1_4AudioProcessorEditor::Test_MNAlgorithm_v1_4AudioProcessorEditor (Test_MNAlgorithm_v1_4AudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    //===========================Input Gain Dial==============================
    addAndMakeVisible(inputgainSlider);
    inputgainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    inputgainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    inputgainSlider.setTextValueSuffix(" dB");

    inputgainAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "input gain", inputgainSlider));


    addAndMakeVisible(inputgainLabel);
    inputgainLabel.attachToComponent(&inputgainSlider, false);
    inputgainLabel.setText("Input Gain", juce::dontSendNotification);
    inputgainLabel.setJustificationType(juce::Justification::centredBottom);

    //===========================Output Gain Dial==============================
    addAndMakeVisible(outputgainSlider);
    outputgainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    outputgainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    outputgainSlider.setTextValueSuffix(" dB");

    outputgainAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "output gain", outputgainSlider));


    addAndMakeVisible(outputgainLabel);
    outputgainLabel.attachToComponent(&outputgainSlider, false);
    outputgainLabel.setText("Output Gain", juce::dontSendNotification);
    outputgainLabel.setJustificationType(juce::Justification::centredBottom);

    //==============================Mix Dial==================================
    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    mixSlider.setTextValueSuffix(" %");
    mixAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "mix", mixSlider));


    addAndMakeVisible(mixLabel);
    mixLabel.attachToComponent(&mixSlider, false);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centredBottom);

    //======================Oversampling  Toggle==================================

    addAndMakeVisible(osComboBox);
    osComboBoxAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(vts, "oversampling", osComboBox));

    osComboBox.addItem("Off", 1);
    osComboBox.addItem("2x", 2);
    osComboBox.addItem("4x", 3);
    osComboBox.addItem("8x", 4);
    osComboBox.setSelectedId(1);

    addAndMakeVisible(osLabel);
    osLabel.attachToComponent(&osComboBox, false);
    osLabel.setText("Oversampling", juce::dontSendNotification);
    osLabel.setJustificationType(juce::Justification::centredTop);

    //======================OpenFile Button==================================

    addAndMakeVisible(openNetlistButton);
    openNetlistButton.setButtonText("Open Netlist");
    openNetlistButton.onClick = [this] { openNetlistButtonClicked(); };


    setSize(600, 500);
}

void Test_MNAlgorithm_v1_4AudioProcessorEditor::openNetlistButtonClicked() {
    fileChooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser) {
            juce::File file = chooser.getResult();
            if (file.existsAsFile()) {
                audioProcessor.loadNetlistFile(file.getFullPathName());
            }
        });
}


Test_MNAlgorithm_v1_4AudioProcessorEditor::~Test_MNAlgorithm_v1_4AudioProcessorEditor()
{
}

//==============================================================================
void Test_MNAlgorithm_v1_4AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(30.0f);
    g.drawFittedText("Real-Time MNA Algorithm", getLocalBounds(), juce::Justification::centred, 1);
}

void Test_MNAlgorithm_v1_4AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    inputgainSlider.setBounds(490, 20, 100, 100);
    outputgainSlider.setBounds(490, 160, 100, 100);
    mixSlider.setBounds(490, 300, 100, 100);
    osComboBox.setBounds(490, 450, 100, 30);

    openNetlistButton.setBounds(200, 100, 200, 50);

}
