#include <iostream>
#include <sstream>
#include <string>

#include "brain/Brain.h"
#include "brain/RqNervousSystem.h"
#include "utils/analysis.h"
#include "utils/timeseries.h"

struct Arguments {
    std::vector<std::string> args;
    bool help;
    std::string mode;
    std::string run;
    std::string stage;
    int repeats;
    int transient;
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
    out << "  " << args[0] << " in-vivo RUN" << std::endl;
    out << "  " << args[0] << " in-vitro RUN STAGE REPEATS TRANSIENT STEPS" << std::endl;
    out << std::endl;
    out << "Generate neural activation time series." << std::endl;
    out << "  in-vivo   Report time series read from brain function files." << std::endl;
    out << "  in-vitro  Report time series generated using random inputs." << std::endl;
    out << std::endl;
    out << "  RUN        Run directory" << std::endl;
    out << "  STAGE      Life stage (incept, birth, or death)" << std::endl;
    out << "  REPEATS    Number of time series per agent" << std::endl;
    out << "  TRANSIENT  Number of ignored timesteps per time series" << std::endl;
    out << "  STEPS      Number of reporting timesteps per time series" << std::endl;
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
        PARSE(mode, , mode == "in-vivo" || mode == "in-vitro", "Invalid mode")
        PARSE(run, , exists(run + "/endStep.txt"), "Not a Polyworld run")
        if (mode == "in-vivo") {
            stage = "birth";
        } else if (mode == "in-vitro") {
            PARSE(stage, , stage == "incept" || stage == "birth" || stage == "death", "Invalid life stage")
            PARSE(repeats, std::stoi, repeats >= 1, "Invalid number of time series")
            PARSE(transient, std::stoi, transient >= 0, "Invalid number of ignored timesteps")
            PARSE(steps, std::stoi, steps >= 1, "Invalid number of reporting timesteps")
        }
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
    out << "# mode = " << arguments.mode << std::endl;
    if (arguments.mode == "in-vitro") {
        out << "# stage = " << arguments.stage << std::endl;
        out << "# repeats = " << arguments.repeats << std::endl;
        out << "# transient = " << arguments.transient << std::endl;
        out << "# steps = " << arguments.steps << std::endl;
    }
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
    std::cout << "# BEGIN ARGUMENTS" << std::endl;
    std::cout << arguments;
    std::cout << "# END ARGUMENTS" << std::endl;
    analysis::initialize(arguments.run);
    int maxAgent = analysis::getMaxAgent(arguments.run);
    for (int agent = 1; agent <= maxAgent; agent++) {
        RqNervousSystem* cns = analysis::getNervousSystem(arguments.run, agent, arguments.stage);
        if (cns == NULL) {
            continue;
        }
        cns->getBrain()->freeze();
        timeseries::writeHeader(std::cout, agent, cns);
        timeseries::writeNerves(std::cout, cns);
        timeseries::writeSynapses(std::cout, cns);
        std::cout << "# BEGIN TIME SERIES ENSEMBLE" << std::endl;
        if (arguments.mode == "in-vivo") {
            timeseries::writeInVivo(std::cout, arguments.run, agent);
        } else if (arguments.mode == "in-vitro") {
            for (int index = 0; index < arguments.repeats; index++) {
                timeseries::writeInVitro(std::cout, cns, arguments.transient, arguments.steps);
            }
        }
        std::cout << "# END TIME SERIES ENSEMBLE" << std::endl;
        delete cns;
    }
    return 0;
}
