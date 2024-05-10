//netlist.cpp
#include "Netlist.h"
#include "component.h"


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

    resistances         = getComponents<Resistance>();
    reactiveComponents  = getComponents<ReactiveComponent>();
    idealOPAs           = getComponents<IdealOPA>();
    voltageSources      = getComponents<VoltageSource>();
    currentSources      = getComponents<CurrentSource>();
    diodes 			    = getComponents<Diode>();

    m = std::size(voltageSources) + std::size(reactiveComponents) + std::size(idealOPAs);
    n = getNodeNbr();       // Total number of unique nodes

    A.resize(n + m, n + m);
    x.resize(n + m);
    b.resize(n + m);

    A.setZero();
    x.setZero();
    b.setZero();

    initializeProcessStrategy();
}


void Netlist::reset() {

    components.clear();
    resistances.clear();
    reactiveComponents.clear();
    idealOPAs.clear();
    voltageSources.clear();
    currentSources.clear();
    voltageProbes.clear();
    diodes.clear();

    A.setZero();
    x.setZero();
    b.setZero();
     
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
    luDecomp.compute(A.bottomRightCorner(A.rows() - 1, A.cols() - 1));
}


//====================================================================================================
//====================================================================================================
void Netlist::setStrategy(std::unique_ptr<ProcessStrategy> strategy) {
    processStrategy = std::move(strategy);
}

void Netlist::processBlock(juce::dsp::AudioBlock<float>& audioBlock) {
    //if (processStrategy) {
        processStrategy->processBlock(*this, audioBlock);
    //}
}

void Netlist::initializeProcessStrategy() {
    if (diodes.empty()) {
        setStrategy(std::make_unique<LinearProcessStrategy>());
    }
    else {
        setStrategy(std::make_unique<NonLinearProcessStrategy>());
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
    case 'D':
        return std::make_shared<Diode>(start_node, end_node);
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