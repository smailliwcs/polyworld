#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "brain/Brain.h"
#include "brain/NeuronModel.h"
#include "brain/RqNervousSystem.h"
#include "genome/Genome.h"
#include "utils/AbstractFile.h"
#include "utils/analysis.h"
#include "utils/misc.h"

struct Arguments {
    std::vector<std::string> args;
    bool help;
    std::string run;
    int agent;
    std::string stage;
    float min;
    float max;
    int count;
    int random;
    int quiescent;
    int steps;

    Arguments(int argc, char** argv);
    std::string usage();
    bool parse();

private:
    void fail(int argi, const std::string& message);
};

Arguments::Arguments(int argc, char** argv) :
        args(argv, argv + argc) { }

std::string Arguments::usage() {
    std::ostringstream out;
    out << "Usage: " << args[0] << " RUN AGENT STAGE MIN MAX COUNT RANDOM QUIESCENT STEPS" << std::endl;
    out << std::endl;
    out << "Generate data for a bifurcation diagram." << std::endl;
    out << "The maximum synaptic weight (w_max) is used as the control parameter." << std::endl;
    out << "All output neuron activations are reported for each w_max value." << std::endl;
    out << std::endl;
    out << "  RUN        Run directory" << std::endl;
    out << "  AGENT      Agent ID" << std::endl;
    out << "  STAGE      Life stage (incept, birth, or death)" << std::endl;
    out << "  MIN        Minimum w_max value" << std::endl;
    out << "  MAX        Maximum w_max value" << std::endl;
    out << "  COUNT      Number of w_max values" << std::endl;
    out << "  RANDOM     Number of random timesteps" << std::endl;
    out << "  QUIESCENT  Number of quiescent timesteps" << std::endl;
    out << "  STEPS      Number of reporting timesteps" << std::endl;
    return out.str();
}

bool Arguments::parse() {
    if (args.size() > 1) {
        std::string arg1 = args[1];
        if (arg1 == "-h" || arg1 == "--help") {
            help = true;
            return true;
        }
    }
    help = false;
    unsigned argi = 1;
    try {
#define PARSE(ARGUMENT, CONVERTER, PREDICATE, MESSAGE) \
{ \
    if (argi >= args.size()) { \
        std::cerr << usage(); \
        return false; \
    } \
    ARGUMENT = CONVERTER(args[argi]); \
    if (!(PREDICATE)) { \
        fail(argi, MESSAGE); \
        return false; \
    } \
    argi++; \
}
        PARSE(run, , exists(run + "/endStep.txt"), "Not a Polyworld run")
        PARSE(agent, std::stoi, agent >= 1, "Invalid agent ID")
        PARSE(stage, , stage == "incept" || stage == "birth" || stage == "death", "Invalid life stage")
        PARSE(min, std::stof, min >= 0.0f, "Invalid minimum w_max value")
        PARSE(max, std::stof, max > min, "Maximum w_max value less than or equal to minimum")
        PARSE(count, std::stoi, count > 1, "Invalid number of w_max values")
        PARSE(random, std::stoi, random >= 0, "Invalid number of random timesteps")
        PARSE(quiescent, std::stoi, quiescent >= 0, "Invalid number of quiescent timesteps")
        PARSE(steps, std::stoi, steps >= 1, "Invalid number of reporting timesteps")
#undef PARSE
    } catch (...) {
        fail(argi, "Invalid argument");
        return false;
    }
    if (argi != args.size()) {
        std::cerr << usage();
        return false;
    }
    return true;
}

void Arguments::fail(int argi, const std::string& message) {
    std::cerr << args[0] << ": " << args[argi] << ": " << message << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Arguments& arguments) {
    out << "# AGENT = " << arguments.agent << std::endl;
    out << "# STAGE = " << arguments.stage << std::endl;
    out << "# RANDOM = " << arguments.random << std::endl;
    out << "# QUIESCENT = "<< arguments.quiescent << std::endl;
    return out;
}

int main(int argc, char** argv) {
    Arguments arguments(argc, argv);
    if (!arguments.parse()) {
        return 1;
    }
    if (arguments.help) {
        std::cout << arguments.usage();
        return 0;
    }
    std::cout << arguments;
    analysis::initialize(arguments.run);
    AbstractFile* synapses = analysis::getSynapses(arguments.run, arguments.agent, arguments.stage);
    if (synapses == NULL) {
        return 0;
    }
    genome::Genome* genome = analysis::getGenome(arguments.run, arguments.agent);
    RqNervousSystem* cns = analysis::getNervousSystem(genome, synapses);
    delete genome;
    cns->getBrain()->freeze();
    NeuronModel::Dimensions dims = cns->getBrain()->getDimensions();
    double* activations = new double[dims.numOutputNeurons];
    for (int index = 0; index < arguments.count; index++) {
        float maxWeight = interp((float)index / (arguments.count - 1), arguments.min, arguments.max);
        synapses->seek(0, SEEK_SET);
        analysis::setMaxWeight(cns, synapses, maxWeight);
        cns->getBrain()->randomizeActivations();
        cns->setMode(RqNervousSystem::RANDOM);
        for (int step = 1; step <= arguments.random; step++) {
            cns->update(false);
        }
        cns->setMode(RqNervousSystem::QUIESCENT);
        for (int step = 1; step <= arguments.quiescent; step++) {
            cns->update(false);
        }
        for (int step = 1; step <= arguments.steps; step++) {
            cns->update(false);
            cns->getBrain()->getActivations(activations, dims.getFirstOutputNeuron(), dims.numOutputNeurons);
            std::cout << maxWeight;
            for (int neuron = 0; neuron < dims.numOutputNeurons; neuron++) {
                std::cout << " " << activations[neuron];
            }
            std::cout << std::endl;
        }
    }
    delete[] activations;
    delete cns;
    delete synapses;
    return 0;
}
