/*
  ==============================================================================

    processStartegy.h
    Created: 10 May 2024 12:39:24pm
    Author:  eliot

  ==============================================================================
*/

#pragma once
#include "component.h"

#include <JuceHeader.h>

class Netlist;

class ProcessStrategy {
public:
    virtual void processBlock(Netlist& netlist, juce::dsp::AudioBlock<float>& audioBlock) = 0;
    virtual ~ProcessStrategy() = default;
};

class LinearProcessStrategy : public ProcessStrategy {
public:
    void processBlock(Netlist& netlist, juce::dsp::AudioBlock<float>& audioBlock) override;
};


class NonLinearProcessStrategy : public ProcessStrategy {
public:
    void processBlock(Netlist& netlist, juce::dsp::AudioBlock<float>& audioBlock) override;
};
