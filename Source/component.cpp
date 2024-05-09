//component.cpp
#pragma once
#include "component.h"
#include "netlist.h"

Component::Component(unsigned start_node, unsigned end_node, double value)
    : start_node(start_node), end_node(end_node), value(value) {}


Resistance::Resistance(unsigned start_node, unsigned end_node, double value)
    : Component(start_node, end_node, value), admittance(1.0 / value) {}

void Resistance::stamp(Netlist& netlist) const {
    netlist.A(start_node, start_node) += admittance;
    netlist.A(end_node, end_node) += admittance;
    netlist.A(start_node, end_node) -= admittance;
    netlist.A(end_node, start_node) -= admittance;
}


// ReactiveComponent class methods
ReactiveComponent::ReactiveComponent(unsigned start_node, unsigned end_node, double value, unsigned index)
    : Component(start_node, end_node, value), index(index), resistance(0), voltage(0) {}

void ReactiveComponent::stamp(Netlist& netlist) const {
    unsigned n = netlist.n;

    netlist.A(n + index, start_node) = 1;
    netlist.A(n + index, end_node) = -1;
    netlist.A(start_node, n + index) = 1;
    netlist.A(end_node, n + index) = -1;

    netlist.A(n + index, n + index) = -resistance;

    netlist.b(n + index) = voltage;
}


Capacitor::Capacitor(unsigned start_node, unsigned end_node, double value, unsigned index)
    : ReactiveComponent(start_node, end_node, value, index) {}

void Capacitor::setResistance(double Ts) {
    resistance = Ts / (2 * value);
}

void Capacitor::updateVoltage(Netlist& netlist) {
    voltage = (netlist.x(start_node) - netlist.x(end_node)) + resistance * netlist.x(netlist.n + index);
}


Inductance::Inductance(unsigned start_node, unsigned end_node, double value, unsigned index)
    : ReactiveComponent(start_node, end_node, value, index) {}

void Inductance::setResistance(double Ts) {
    resistance = (2 * value) / Ts;
}

void Inductance::updateVoltage(Netlist& netlist) {
    voltage = -((netlist.x(start_node) - netlist.x(end_node)) + resistance * netlist.x(netlist.n + index));
}


VoltageSource::VoltageSource(unsigned start_node, unsigned end_node, double value, unsigned index)
    : Component(start_node, end_node, value), voltage(value), index(index) {}

void VoltageSource::stamp(Netlist& netlist) const {

    netlist.A(start_node, netlist.n + index) = 1;
    netlist.A(end_node, netlist.n + index) = -1;
    netlist.A(netlist.n + index, start_node) = 1;
    netlist.A(netlist.n + index, end_node) = -1;

    netlist.b(netlist.n + index) = voltage;
}


ExternalVoltageSource::ExternalVoltageSource(unsigned start_node, unsigned end_node, double value, unsigned index)
    : VoltageSource(start_node, end_node, value, index) {}

void ExternalVoltageSource::update(double new_voltage) {
    voltage = new_voltage;
}


CurrentSource::CurrentSource(unsigned start_node, unsigned end_node, double value)
    : Component(start_node, end_node, value), current(value) {}

void CurrentSource::stamp(Netlist& netlist) const {
    netlist.b(start_node) -= current;
    netlist.b(end_node) += current;
}


IdealOPA::IdealOPA(unsigned start_node, unsigned end_node, unsigned output_node, unsigned index)
    : Component(start_node, end_node, 0.0), output_node(output_node), index(index) {}

void IdealOPA::stamp(Netlist& netlist) const {
    unsigned n = netlist.n;
    netlist.A(output_node, n + index) = 1;
    netlist.A(n + index, start_node) = 1;
    netlist.A(n + index, end_node) = -1;
}

VoltageProbe::VoltageProbe(unsigned start_node, unsigned end_node)
	: Component(start_node, end_node, 0.0) {}

void VoltageProbe::getVoltage(Netlist& netlist) {
	value = netlist.x(start_node) - netlist.x(end_node);
} 


Diode::Diode(unsigned start_node, unsigned end_node)
    : Component(start_node, end_node, 0.0) {

    //Diode parameters based on the characteristics of the 1N34A model

    N = 1.6;	   //ideality factor
    Is = 2.6e-6;   //reverse saturation current

    Vt = 0.025852;  //thermal voltage at approx. 300K 
    N_Vt = N * Vt;  //N*Vt

    Id = 0;         //current through the diode
    voltage = 0;    //voltage across the diode

    Ieq = 0;        //equivalent current
    Geq = 0;	    //equivalent conductance
}


void Diode::stamp(Netlist& netlist) const {
    unsigned n = netlist.n;

    netlist.A(start_node, start_node) += Geq;
    netlist.A(end_node, end_node) += Geq;
    netlist.A(start_node, end_node) -= Geq;
    netlist.A(end_node, start_node) -= Geq;

    netlist.b(start_node) -= Is;
    netlist.b(end_node) += Is;

}

void Diode::update_voltage(Netlist& netlist) {
    voltage = netlist.x(start_node) - netlist.x(end_node);
}

void Diode::update_Id(Netlist& netlist) {
    Id = Is * std::expm1(voltage / N_Vt); //Id = Is * (std::exp(voltage / N_Vt) - 1);
}

void Diode::update_Geq(Netlist& netlist) {
    Geq = (Is / N_Vt) * std::exp(voltage / N_Vt);
}

void Diode::update_Ieq(Netlist& netlist) {
    Ieq = Id - Geq * voltage;
}