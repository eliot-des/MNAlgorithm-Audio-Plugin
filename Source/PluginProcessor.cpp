/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Test_MNAlgorithm_v1_4AudioProcessor::Test_MNAlgorithm_v1_4AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
     ),
#else 
    :

#endif
parameters(*this, nullptr, juce::Identifier("CircuitLive"), {
        std::make_unique<juce::AudioParameterFloat>("input gain", "Input Gain", juce::NormalisableRange{ -40.f, 40.f ,0.1f, 1.f, false }, 0.f),
        std::make_unique<juce::AudioParameterFloat>("output gain", "Output Gain", juce::NormalisableRange{ -40.f, 40.f ,0.1f, 1.f, false }, 0.f),
        std::make_unique<juce::AudioParameterInt>("mix", "Mix", 0, 100, 100),
        std::make_unique<juce::AudioParameterInt>("oversampling","Oversampling", 1, 4, 1)
    }) {

    inputGainParameter     = parameters.getRawParameterValue("input gain");
    outputGainParameter    = parameters.getRawParameterValue("output gain");
    mixPercentageParameter = parameters.getRawParameterValue("mix");
    oversamplingParameter  = parameters.getRawParameterValue("oversampling");

    int numInputChannels = getTotalNumInputChannels();

    //initialize all the "oversamplers"
    for (int i = 0; i < 3; ++i) {
        oversampler[i] = std::make_unique<juce::dsp::Oversampling<float>>(numInputChannels, i + 1,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
    }
};

Test_MNAlgorithm_v1_4AudioProcessor::~Test_MNAlgorithm_v1_4AudioProcessor()
{
}

//==============================================================================
const juce::String Test_MNAlgorithm_v1_4AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Test_MNAlgorithm_v1_4AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Test_MNAlgorithm_v1_4AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Test_MNAlgorithm_v1_4AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Test_MNAlgorithm_v1_4AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Test_MNAlgorithm_v1_4AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Test_MNAlgorithm_v1_4AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Test_MNAlgorithm_v1_4AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Test_MNAlgorithm_v1_4AudioProcessor::getProgramName (int index)
{
    return {};
}

void Test_MNAlgorithm_v1_4AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void Test_MNAlgorithm_v1_4AudioProcessor::loadNetlistFile(const juce::String& path) {

    //suspend processing while loading the netlist, don't know if this is good practice...
    suspendProcessing(true);
    try {
        netlist.reset();
        netlist.init(path.toStdString());

        if (netlist.isInitialized) {
            netlist.prepareChannels(getTotalNumInputChannels());
            netlist.setSampleRate(currentSampleRate);
            netlist.solve_system();
        }
    }
    catch (...) {
        // Handle exceptions or errors later...
        netlist.isInitialized = false;
    }
    suspendProcessing(false);
}

//==============================================================================
void Test_MNAlgorithm_v1_4AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto& os : oversampler)
        os->initProcessing(samplesPerBlock);
}

void Test_MNAlgorithm_v1_4AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Test_MNAlgorithm_v1_4AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
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

void Test_MNAlgorithm_v1_4AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    // Bypass all the processing if the netlist is not initialized
    if (!netlist.isInitialized) return;


    const auto inputGain = inputGainParameter->load();
    const auto outputGain = outputGainParameter->load();
    const auto mixPercentage = mixPercentageParameter->load();
    const auto oversamplingIndex = static_cast<int>(oversamplingParameter->load());

    netlist.setInputGain(inputGain);
    netlist.setOutputGain(outputGain);
    netlist.setMixPercentage(mixPercentage);


    const double effectiveSampleRate = currentSampleRate * std::pow(2.0, oversamplingIndex - 1);
    if (effectiveSampleRate != netlist.sampleRate) {
        netlist.setSampleRate(effectiveSampleRate);
        netlist.clear_system();
        netlist.solve_system();
    }


    juce::dsp::AudioBlock<float> inputblock{ buffer };

    if (oversamplingIndex > 1) {
        auto oversampledblock = oversampler[oversamplingIndex - 2]->processSamplesUp(inputblock);
        netlist.processBlock(oversampledblock);
        oversampler[oversamplingIndex - 2]->processSamplesDown(inputblock);
    }
    else {
        netlist.processBlock(inputblock);
    }
    
}

//==============================================================================
bool Test_MNAlgorithm_v1_4AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Test_MNAlgorithm_v1_4AudioProcessor::createEditor()
{
    return new Test_MNAlgorithm_v1_4AudioProcessorEditor (*this, parameters);
}

//==============================================================================
void Test_MNAlgorithm_v1_4AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Test_MNAlgorithm_v1_4AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Test_MNAlgorithm_v1_4AudioProcessor();
}