//netlist.h
#pragma once
#include "JuceHeader.h"

#include <Eigen/Dense>
#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <memory>


// Forward declarations to avoid circular dependencies
class Component;
class Resistance;
class ReactiveComponent;
class Capacitor;
class Inductance;
class VoltageSource;
class ExternalVoltageSource;
class CurrentSource;
class IdealOPA;
class VoltageProbe;

class Netlist {//: public std::enable_shared_from_this<Netlist> 
public:
    std::vector<std::shared_ptr<Component>> components;
    std::vector<std::shared_ptr<Resistance>> resistances;
    std::vector<std::shared_ptr<ReactiveComponent>> reactiveComponents;
    std::vector<std::shared_ptr<IdealOPA>> idealOPAs;
    std::vector<std::shared_ptr<VoltageSource>> voltageSources;
    std::vector<std::shared_ptr<CurrentSource>> currentSources;
    std::vector<std::shared_ptr<VoltageProbe>> voltageProbes;

    Eigen::MatrixXd A;
    Eigen::VectorXd x, b;
    Eigen::PartialPivLU<Eigen::MatrixXd> luDecomp;

    std::vector<Eigen::VectorXd> channelBStates;
    std::vector<Eigen::VectorXd> channelXStates;

    unsigned m;
    unsigned n; // Number of unique nodes including the ground node (0)

    bool isInitialized = false;
    double sampleRate;

    // Constructor
    Netlist() = default;                            // Default constructor
    explicit Netlist(const std::string& filename);  // Constructor with filename

    // Public methods
    void init(const std::string& filename);
    void reset();
    void clear_system();
    void solve_system();

    void setInputGain(float inputGain);
    void setOutputGain(float outputGain);
    void setMixPercentage(float mixPercentage);
    void setSampleRate(double sampleRate);
    void prepareChannels(int numChannels);

    void processBlock(juce::dsp::AudioBlock<float>& audioBlock);


    // Generic function to get components of a specific type
    template <typename T>
    std::vector<std::shared_ptr<T>> getComponents() {
        std::vector<std::shared_ptr<T>> specificComponents;

        for (const auto& component : components) {
            std::shared_ptr<T> specificComponent = std::dynamic_pointer_cast<T>(component);
            if (specificComponent) {
                specificComponents.push_back(specificComponent);
            }
        }
        return specificComponents;
    }

private:
    // Private attributes
    float inputGain;
    float outputGain;
    float mixPercentage;

    // Private methods
    std::vector<std::shared_ptr<Component>> createComponentListFromTxt(const std::string& filename);
    std::shared_ptr<Component> createComponent(const std::string& netlistLine, unsigned idx);
    std::vector<std::string> split(const std::string& s, char delimiter);
    unsigned getNodeNbr();
};