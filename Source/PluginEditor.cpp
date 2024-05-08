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
    
    
    fileComp.reset(new juce::FilenameComponent("fileComp",
        {},                       // current file
        false,                     // can edit file name,
        false,                    // is directory,
        false,                    // is for saving,
        "*.txt",                  // browser wildcard suffix,
        {},                       // enforced suffix,
        "Select netlist to open"));  // text when nothing selected
    //set default directory to the desktop
    fileComp->setDefaultBrowseTarget(juce::File::getSpecialLocation(juce::File::userDesktopDirectory));

    addAndMakeVisible(fileComp.get());
    fileComp->addListener(this);
      

    //======================TextEditor==================================

    textContent.reset(new juce::TextEditor());
    addAndMakeVisible(textContent.get());
    textContent->setMultiLine(true);
    textContent->setReadOnly(false);
    textContent->setCaretVisible(true);
    textContent->setPopupMenuEnabled(true);
    textContent->setScrollbarsShown(true);
    textContent->setReturnKeyStartsNewLine(true);
    textContent->setFont(juce::Font("Consolas", 20.0f, juce::Font::plain));

    //======================Save Button==================================
    addAndMakeVisible(updateButton);
    updateButton.setButtonText("Update");
    updateButton.onClick = [this] { updateButtonClicked(); };

    setSize(600, 500);
}



void Test_MNAlgorithm_v1_4AudioProcessorEditor::filenameComponentChanged(juce::FilenameComponent* fileComponentThatHasChanged)
{
    if (fileComponentThatHasChanged == fileComp.get())
        readFile(fileComp->getCurrentFile());
}

void Test_MNAlgorithm_v1_4AudioProcessorEditor::readFile(const juce::File& fileToRead)
{
    if (!fileToRead.existsAsFile())
        return;

    audioProcessor.loadNetlistFile(fileToRead.getFullPathName());
    auto fileText = fileToRead.loadFileAsString();
    textContent->setText(fileText);
}

void Test_MNAlgorithm_v1_4AudioProcessorEditor::updateButtonClicked() {
    if (!audioProcessor.netlistPath.isEmpty()) {
        juce::File file(audioProcessor.netlistPath);
        file.replaceWithText(textContent->getText());
        readFile(file);
    }
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
    g.setFont(juce::Font("Century Gothic", 33.0f, juce::Font::bold));
    g.drawFittedText("Circuit-Live", 20, 15, 300, 20, juce::Justification::centredLeft, 1);
}

void Test_MNAlgorithm_v1_4AudioProcessorEditor::resized()
{

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    inputgainSlider.setBounds(490, 20, 100, 100);
    outputgainSlider.setBounds(490, 160, 100, 100);
    mixSlider.setBounds(490, 300, 100, 100);
    osComboBox.setBounds(490, 450, 100, 30);
    
    fileComp->setBounds(20, 50, 360, 30);
    updateButton.setBounds(400, 50, 70, 30);
    textContent->setBounds(20, 100, 450, 380);
    
}
