//component.h
#pragma once

//Forward declaration of Netlist class to avoid circular dependencies
class Netlist;

class Component {
public:
    unsigned start_node, end_node;
    double value;

    Component(unsigned start_node, unsigned end_node, double value);
    virtual ~Component() = default;

    virtual void stamp(Netlist& netlist) const = 0;
};

class Resistance : public Component {
public:
    double admittance;

    Resistance(unsigned start_node, unsigned end_node, double value);
    virtual void stamp(Netlist& netlist) const override;
};

class ReactiveComponent : public Component {
public:
    unsigned index;
    double resistance;
    double voltage;

    ReactiveComponent(unsigned start_node, unsigned end_node, double value, unsigned index);
    virtual void setResistance(double Ts) = 0;
    virtual void updateVoltage(Netlist& netlist) = 0;
    virtual void stamp(Netlist& netlist) const override;
};

class Capacitor : public ReactiveComponent {
public:
    Capacitor(unsigned start_node, unsigned end_node, double value, unsigned index);
    virtual void setResistance(double Ts) override;
    virtual void updateVoltage(Netlist& netlist) override;
};

class Inductance : public ReactiveComponent {
public:
    Inductance(unsigned start_node, unsigned end_node, double value, unsigned index);
    virtual void setResistance(double Ts) override;
    virtual void updateVoltage(Netlist& netlist) override;
};

class VoltageSource : public Component {
public:
    double voltage;
    unsigned index;

    VoltageSource(unsigned start_node, unsigned end_node, double value, unsigned index);
    virtual void stamp(Netlist& netlist) const override;
};

class ExternalVoltageSource : public VoltageSource {
public:
    ExternalVoltageSource(unsigned start_node, unsigned end_node, double value, unsigned index);
    virtual void update(double new_voltage);
};

class CurrentSource : public Component {
public:
    double current;

    CurrentSource(unsigned start_node, unsigned end_node, double value);
    virtual void stamp(Netlist& netlist) const override;
};

class IdealOPA : public Component {
public:
    unsigned output_node;
    unsigned index;

    IdealOPA(unsigned start_node, unsigned end_node, unsigned output_node, unsigned index);
    virtual void stamp(Netlist& netlist) const override;
};

class VoltageProbe : public Component {
public:
    VoltageProbe(unsigned start_node, unsigned end_node);
    //define the stamp method as something that does nothing
    virtual void stamp(Netlist& netlist) const override {};

	//Method to update the voltage of the probe
	void getVoltage(Netlist& netlist);
};
