//netlist.cpp
#include "Netlist.h"
#include "component.h"
#include "chrono"

Netlist::Netlist(const std::string& filename) {
    init(filename);
}

void Netlist::init(const std::string& filename) {

    components = createComponentListFromTxt(filename);
    voltageProbes = getComponents<VoltageProbe>();
    //if there is no component in the netlist or not any voltageProbes
    //the system is not initialized and we not define the A matrix, the vector x and b, and so on
    if (components.empty() || voltageProbes.empty()) {
        isInitialized = false;  
        return;
	}
    else {
        isInitialized = true;
        
    }

    resistances = getComponents<Resistance>();
    reactiveComponents = getComponents<ReactiveComponent>();
    idealOPAs = getComponents<IdealOPA>();
    voltageSources = getComponents<VoltageSource>();
    currentSources = getComponents<CurrentSource>();

    m = std::size(voltageSources) + std::size(reactiveComponents) + std::size(idealOPAs);
    n = getNodeNbr();       // Total number of unique nodes

    A.resize(n + m, n + m);
    x.resize(n + m);
    b.resize(n + m);

    A.setZero();
    x.setZero();
    b.setZero();
}


void Netlist::reset() {

    components.clear();
    resistances.clear();
    reactiveComponents.clear();
    idealOPAs.clear();
    voltageSources.clear();
    currentSources.clear();
    voltageProbes.clear();

    A.setZero();
    x.setZero();
    b.setZero();
    // Reset the LU decomposition object
    // don't seem to be the problem
    luDecomp = Eigen::PartialPivLU<Eigen::MatrixXd>(); 
    m = 0;
    n = 0;

    isInitialized = false;
}


void Netlist::clear_system() {
    A.setZero();
    x.setZero();
    b.setZero();
}


void Netlist::solve_system() {
    for (const auto& comp : reactiveComponents) comp->setResistance(1 / sampleRate);
    for (const auto& comp : components) comp->stamp(*this);
    luDecomp.compute(A.bottomRightCorner(m + n - 1, m + n - 1)); 
}


void Netlist::processBlock(juce::dsp::AudioBlock<float>& audioBlock) {

    const auto mix = mixPercentage / 100.0f;

    for (auto channel = 0; channel < audioBlock.getNumChannels(); ++channel) {
        auto* channelSamples = audioBlock.getChannelPointer(channel);

        // Use saved states for this channel
        b = channelBStates[channel];
        x = channelXStates[channel];

        for (auto i = 0; i < audioBlock.getNumSamples(); i++) {
            const auto inputSample = channelSamples[i];
            const auto inputCircuitSample = inputSample * std::pow(10, inputGain / 20);

            // Update the system based on the input sample
            for (auto& source : voltageSources) {
                std::shared_ptr<ExternalVoltageSource> externalSource = std::dynamic_pointer_cast<ExternalVoltageSource>(source);
                if (externalSource) {
                    externalSource->update(inputCircuitSample);
                }
                source->stamp(*this);
            }
            for (auto& comp : reactiveComponents) {
                comp->updateVoltage(*this);
                comp->stamp(*this);
            }

            /* Solve the system
            *  We remove the first row and column of the A matrix, 
            *  as the first element of the vector b because they refer to the ground node
            */
            x.tail(x.size() - 1) = luDecomp.solve(b.tail(b.size() - 1));

            //actualize the voltage value on the voltage probes
            for (auto& voltageProbe : voltageProbes) {
				voltageProbe->getVoltage(*this);
			}

            //test with the first voltage probe
            float outputCircuitSample = voltageProbes[0]->value;

            float outputSample = outputCircuitSample * std::pow(10.0f, outputGain / 20.0f);

            // Apply mixing (dry/wet)
            channelSamples[i] = outputSample * mix + (1 - mix) * inputSample;
        }

        // Save the updated states for this channel
        channelBStates[channel] = b;
        channelXStates[channel] = x;
    }
}

//====================================================================================================
//====================================================================================================

void Netlist::setInputGain(float inputGain) {
    this->inputGain = inputGain;
}


void Netlist::setOutputGain(float outputGain) {
    this->outputGain = outputGain;
}


void Netlist::setMixPercentage(float mixPercentage) {
    this->mixPercentage = mixPercentage;
}

void Netlist::setSampleRate(double sampleRate) {
    this->sampleRate = sampleRate;
}

void Netlist::prepareChannels(int numChannels) {
    channelBStates.resize(numChannels, b);
    channelXStates.resize(numChannels, x);
}


//====================================================================================================
//====================================================================================================
// Calculate the total number of unique nodes (including ground)
// Very simple implementation, not optimized and don't deal with the case of disconnected components
unsigned Netlist::getNodeNbr() {
    std::unordered_set<unsigned> nodes;
    for (const auto& comp : components) {
        nodes.insert(comp->start_node);
        nodes.insert(comp->end_node);
    }
    return nodes.size();
}


//====================================================================================================
//====================================================================================================
std::vector<std::string> Netlist::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


// Factory method to create components from a definition string
std::shared_ptr<Component> Netlist::createComponent(const std::string& netlistLine, unsigned idx) {
    auto tokens = split(netlistLine, ' ');
    std::string symbol = tokens[0];
    unsigned start_node = std::stoi(tokens[1]);
    unsigned end_node = std::stoi(tokens[2]);
    double value = std::stod(tokens[3]);

    // Depending on the symbol, instantiate the appropriate component
    switch (symbol[0]) {
    case 'V':
        if (symbol[1] == 'i') {
            return std::make_shared<ExternalVoltageSource>(start_node, end_node, value, idx);
        }
        else if (symbol[1] == 'o') {
            return std::make_shared<VoltageProbe>(start_node, end_node);
        }
        else {
            return std::make_shared<VoltageSource>(start_node, end_node, value, idx);
        }
    case 'R':
        return std::make_shared<Resistance>(start_node, end_node, value);
    case 'C':
        return std::make_shared<Capacitor>(start_node, end_node, value, idx);
    case 'L':
        return std::make_shared<Inductance>(start_node, end_node, value, idx);
    case 'I':
        return std::make_shared<CurrentSource>(start_node, end_node, value);
    case 'O':
        return std::make_shared<IdealOPA>(start_node, end_node, value, idx);
    default:
        throw std::runtime_error("Unknown component symbol: " + symbol);
    }
}

std::vector<std::shared_ptr<Component>> Netlist::createComponentListFromTxt(const std::string& filename) {
    std::vector<std::shared_ptr<Component>> components;
    std::ifstream netlistTxt(filename);
    std::string line;
    unsigned idx = 0;

    if (netlistTxt.is_open()) {
        while (std::getline(netlistTxt, line)) {
            if (!line.empty()) {
                auto component = createComponent(line, idx);
                if (dynamic_cast<VoltageSource*>(component.get()) != nullptr ||
                    dynamic_cast<ReactiveComponent*>(component.get()) != nullptr ||
                    dynamic_cast<IdealOPA*>(component.get()) != nullptr) {
                    idx++;
                }
                if (component) {
                    components.push_back(std::move(component));
                }
            }
        }
        netlistTxt.close();
    }
    else {
        std::cout << "Unable to open the netlist file" << std::endl;
    }
    return components;
}