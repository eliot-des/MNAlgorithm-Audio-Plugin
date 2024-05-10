/*
  ==============================================================================

    processStategy.cpp
    Created: 10 May 2024 12:39:04pm
    Author:  eliot

  ==============================================================================
*/
#include "processStartegy.h"
#include "netlist.h"
// Ensure all needed component classes are fully available either through direct includes or through Netlist.h

void LinearProcessStrategy::processBlock(Netlist& netlist, juce::dsp::AudioBlock<float>& audioBlock) {
    const auto mix = netlist.mixPercentage / 100.0f;

    for (auto channel = 0; channel < audioBlock.getNumChannels(); ++channel) {
        auto* channelSamples = audioBlock.getChannelPointer(channel);

        netlist.b = netlist.channelBStates[channel];
        netlist.x = netlist.channelXStates[channel];

        for (auto i = 0; i < audioBlock.getNumSamples(); i++) {
            const auto inputSample = channelSamples[i];
            const auto inputCircuitSample = inputSample * std::pow(10, netlist.inputGain / 20);

            for (auto& source : netlist.voltageSources) {
                auto externalSource = std::dynamic_pointer_cast<ExternalVoltageSource>(source);
                if (externalSource) {
                    externalSource->update(inputCircuitSample);
                }
                source->stamp(netlist);
            }
            for (auto& comp : netlist.reactiveComponents) {
                comp->updateVoltage(netlist);
                comp->stamp(netlist);
            }

            netlist.x.tail(netlist.x.size() - 1) = netlist.luDecomp.solve(netlist.b.tail(netlist.b.size() - 1));
            for (auto& voltageProbe : netlist.voltageProbes) {
                voltageProbe->getVoltage(netlist);
            }

            float outputCircuitSample = netlist.voltageProbes[0]->value;
            float outputSample = outputCircuitSample * std::pow(10.0f, netlist.outputGain / 20.0f);
            channelSamples[i] = outputSample * mix + (1 - mix) * inputSample;
        }

        netlist.channelBStates[channel] = netlist.b;
        netlist.channelXStates[channel] = netlist.x;
    }
}


void NonLinearProcessStrategy::processBlock(Netlist& netlist, juce::dsp::AudioBlock<float>& audioBlock) {
    const auto mix = netlist.mixPercentage / 100.0f;

    for (auto channel = 0; channel < audioBlock.getNumChannels(); ++channel) {
        auto* channelSamples = audioBlock.getChannelPointer(channel);

        netlist.b = netlist.channelBStates[channel];
        netlist.x = netlist.channelXStates[channel];

        for (auto i = 0; i < audioBlock.getNumSamples(); i++) {
            const auto inputSample = channelSamples[i];
            const auto inputCircuitSample = inputSample * std::pow(10, netlist.inputGain / 20);

            for (auto& source : netlist.voltageSources) {
                auto externalSource = std::dynamic_pointer_cast<ExternalVoltageSource>(source);
                if (externalSource) {
                    externalSource->update(inputCircuitSample);
                }
                source->stamp(netlist);
            }

            for (auto& comp : netlist.reactiveComponents) {
                comp->updateVoltage(netlist);
            }
            //Newton-Raphson method
            for (unsigned k = 1; k < 16; k++) {
                /*
                Have to improve the way to update the values of the diodes,
                since re - stamping the whole system at each iteration is not efficient.
                In theory, we should just stamp the new values of the diodes in the A matrix and b vector.
                Here, we are oblige to reset all the system, since the stamping operation
                is done by a "+=" operation, and we can't just remove the contribution of the diodes
                in the A matrix and b vector
                */

                netlist.A.setZero();
                netlist.b.setZero();

                for (auto& diode : netlist.diodes) {
                    diode->update_voltage(netlist);
                    diode->update_Id(netlist);
                    diode->update_Geq(netlist);
                    diode->update_Ieq(netlist);
                }

                for (auto& comp : netlist.components) {
                    comp->stamp(netlist);
                }

                netlist.luDecomp.compute(netlist.A.bottomRightCorner(netlist.A.rows() - 1, netlist.A.cols() - 1));

                Eigen::VectorXd x_old = netlist.x;
                netlist.x.tail(netlist.x.size() - 1) = netlist.luDecomp.solve(netlist.b.tail(netlist.b.size() - 1));

                if ((x_old.tail(x_old.size() - 1) - netlist.x.tail(netlist.x.size() - 1)).norm() < 1e-6) {
                    break;
                }
            }

            for (auto& voltageProbe : netlist.voltageProbes) {
                voltageProbe->getVoltage(netlist);
            }

            float outputCircuitSample = netlist.voltageProbes[0]->value;
            float outputSample = outputCircuitSample * std::pow(10.0f, netlist.outputGain / 20.0f);
            channelSamples[i] = outputSample * mix + (1 - mix) * inputSample;
        }

        netlist.channelBStates[channel] = netlist.b;
        netlist.channelXStates[channel] = netlist.x;
    }
}   