#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "brain/RqNervousSystem.h"
#include "genome/Genome.h"
#include "utils/AbstractFile.h"
#include "utils/analysis.h"
#include "utils/misc.h"

struct Arguments {
    std::vector<std::string> args;
    bool help;
    std::string mode;
    std::string run;
    int agent;
    std::string stage;
    float min;
    float max;
    int count;
    double perturbation;
    double threshold;
    int repeats;
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
    out << "Usage:" << std::endl;
    out << "  " << args[0] << " all RUN STAGE PERTURBATION REPEATS RANDOM QUIESCENT STEPS" << std::endl;
    out << "  " << args[0] << " single RUN AGENT STAGE MIN MAX COUNT PERTURBATION REPEATS RANDOM QUIESCENT STEPS" << std::endl;
    out << "  " << args[0] << " onset RUN STAGE MAX PERTURBATION THRESHOLD REPEATS RANDOM QUIESCENT STEPS" << std::endl;
    out << std::endl;
    out << "Calculate state space expansion (SSE) or onset of criticality (OOC)." << std::endl;
    out << "See http://hdl.handle.net/2022/24539 for more information." << std::endl;
    out << "  all     Report SSE for all agents using the actual maximum synaptic weight (w_max)." << std::endl;
    out << "  single  Report SSE for a single agent over a range of w_max values." << std::endl;
    out << "  onset   Report OOC for all agents." << std::endl;
    out << std::endl;
    out << "  RUN           Run directory" << std::endl;
    out << "  AGENT         Agent ID" << std::endl;
    out << "  STAGE         Life stage (incept, birth, or death)" << std::endl;
    out << "  MIN           Minimum w_max value" << std::endl;
    out << "  MAX           Maximum w_max value" << std::endl;
    out << "  COUNT         Number of w_max values" << std::endl;
    out << "  PERTURBATION  Magnitude of perturbation" << std::endl;
    out << "  THRESHOLD     Threshold SSE" << std::endl;
    out << "  REPEATS       Number of calculations per agent" << std::endl;
    out << "  RANDOM        Number of random timesteps per calculation" << std::endl;
    out << "  QUIESCENT     Number of quiescent timesteps per calculation" << std::endl;
    out << "  STEPS         Number of reporting timesteps per calculation" << std::endl;
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
        PARSE(mode, , mode == "all" || mode == "single" || mode == "onset", "Invalid mode")
        PARSE(run, , exists(run + "/endStep.txt"), "Not a Polyworld run")
        if (mode == "single") {
            PARSE(agent, std::stoi, agent >= 1, "Invalid agent ID")
        }
        PARSE(stage, , stage == "incept" || stage == "birth" || stage == "death", "Invalid life stage")
        if (mode == "single") {
            PARSE(min, std::stof, min >= 0.0f, "Invalid minimum w_max value")
            PARSE(max, std::stof, max > min, "Maximum w_max value less than or equal to minimum")
            PARSE(count, std::stoi, count > 1, "Invalid number of w_max values")
        } else if (mode == "onset") {
            PARSE(max, std::stof, max >= 0.0f, "Invalid maximum w_max value")
        }
        PARSE(perturbation, std::stod, perturbation > 0.0, "Invalid magnitude of perturbation")
        if (mode == "onset") {
            PARSE(threshold, std::stod, threshold > 0.0 && threshold <= 1.0, "Invalid threshold SSE")
        }
        PARSE(repeats, std::stoi, repeats >= 1, "Invalid number of calculations")
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
    if (arguments.mode == "single") {
        out << "# AGENT = " << arguments.agent << std::endl;
    }
    out << "# STAGE = " << arguments.stage << std::endl;
    if (arguments.mode == "onset") {
        out << "# MAX = " << arguments.max << std::endl;
    }
    out << "# PERTURBATION = " << arguments.perturbation << std::endl;
    if (arguments.mode == "onset") {
        out << "# THRESHOLD = " << arguments.threshold << std::endl;
    }
    if (arguments.mode != "single") {
        out << "# REPEATS = " << arguments.repeats << std::endl;
    }
    out << "# RANDOM = " << arguments.random << std::endl;
    out << "# QUIESCENT = "<< arguments.quiescent << std::endl;
    out << "# STEPS = " << arguments.steps << std::endl;
    return out;
}

double getExpansion(genome::Genome* genome, RqNervousSystem* cns, const Arguments& arguments, int repeats);
double getExpansion(genome::Genome* genome, RqNervousSystem* cns, const Arguments& arguments);

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
    int minAgent;
    int maxAgent;
    if (arguments.mode == "single") {
        maxAgent = minAgent = arguments.agent;
    } else {
        minAgent = 1;
        maxAgent = analysis::getMaxAgent(arguments.run);
    }
    for (int agent = minAgent; agent <= maxAgent; agent++) {
        AbstractFile* synapses = analysis::getSynapses(arguments.run, agent, arguments.stage);
        if (synapses == NULL) {
            continue;
        }
        genome::Genome* genome = analysis::getGenome(arguments.run, agent);
        RqNervousSystem* cns = analysis::getNervousSystem(genome, synapses);
        if (arguments.mode == "all") {
            double expansion = getExpansion(genome, cns, arguments);
            std::cout << agent << " " << expansion << std::endl;
        } else if (arguments.mode == "single") {
            for (int index = 0; index < arguments.count; index++) {
                float maxWeight = interp((float)index / (arguments.count - 1), arguments.min, arguments.max);
                synapses->seek(0, SEEK_SET);
                analysis::setMaxWeight(cns, synapses, maxWeight);
                for (int repeat = 1; repeat <= arguments.repeats; repeat++) {
                    double expansion = getExpansion(genome, cns, arguments, 1);
                    std::cout << maxWeight << " " << expansion << std::endl;
                }
            }
        } else if (arguments.mode == "onset") {
            float onset = std::numeric_limits<float>::infinity();
            float start = 0.0f;
            int multiplier = 1;
            int resolution = 0;
            bool found = false;
            while (true) {
                float maxWeight = 0.0f;
                for ( ; multiplier <= 10; multiplier++) {
                    maxWeight = multiplier * pow(10.0f, resolution) + start;
                    if (maxWeight > arguments.max || maxWeight >= onset) {
                        break;
                    }
                    synapses->seek(0, SEEK_SET);
                    analysis::setMaxWeight(cns, synapses, maxWeight);
                    double expansion = getExpansion(genome, cns, arguments, 1);
                    if (expansion >= arguments.threshold * 0.9 && arguments.repeats > 1) {
                        expansion = getExpansion(genome, cns, arguments);
                    }
                    if (expansion >= arguments.threshold) {
                        onset = maxWeight;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    if (resolution == 0) {
                        break;
                    }
                    start = onset - pow(10.0f, resolution);
                    multiplier = 1;
                    resolution--;
                } else {
                    if (maxWeight > arguments.max) {
                        break;
                    }
                    multiplier = 2;
                    resolution++;
                }
            }
            std::cout << agent << " " << onset << std::endl;
        }
        delete cns;
        delete genome;
        delete synapses;
    }
    return 0;
}

double getExpansion(genome::Genome* genome, RqNervousSystem* cns, const Arguments& arguments, int repeats) {
    return analysis::getExpansion(
            genome,
            cns,
            arguments.perturbation,
            repeats,
            arguments.random,
            arguments.quiescent,
            arguments.steps);
}

double getExpansion(genome::Genome* genome, RqNervousSystem* cns, const Arguments& arguments) {
    return getExpansion(genome, cns, arguments, arguments.repeats);
}
