#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "genome/GenomeUtil.h"
#include "sim/globals.h"
#include "utils/AbstractFile.h"
#include "utils/analysis.h"
#include "utils/misc.h"

struct Arguments {
    std::vector<std::string> args;
    bool help;
    std::string run;

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
    out << "Usage: " << args[0] << " RUN" << std::endl;
    out << std::endl;
    out << "Generate the population genetics log for an existing run." << std::endl;
    out << "See Logs::PopulationGeneticsLog for more information." << std::endl;
    out << std::endl;
    out << "  RUN  Run directory" << std::endl;
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

void printBirth(const std::string& run, int agent);
void printDeath(int agent);
void printStep(int step);

int main(int argc, char** argv) {
    Arguments arguments(argc, argv);
    if (!arguments.parse()) {
        return 1;
    }
    if (arguments.help) {
        std::cout << arguments.usage();
        return 0;
    }
    analysis::initialize(arguments.run);
    std::cout << "SIZE " << genome::GenomeUtil::schema->getMutableSize() << std::endl;
    int initAgentCount = analysis::getInitAgentCount(arguments.run);
    for (int agent = 1; agent <= initAgentCount; agent++) {
        printBirth(arguments.run, agent);
    }
    printStep(0);
    std::map<int, std::list<analysis::Event> > events = analysis::getEvents(arguments.run);
    int maxTimestep = analysis::getMaxTimestep(arguments.run);
    for (int timestep = 1; timestep <= maxTimestep; timestep++) {
        std::map<int, std::list<analysis::Event> >::iterator eventsIter = events.find(timestep);
        if (eventsIter != events.end()) {
            itfor(std::list<analysis::Event>, eventsIter->second, timestepEventsIter) {
                analysis::Event event = *timestepEventsIter;
                if (event.type == "BIRTH" || event.type == "CREATION") {
                    printBirth(arguments.run, event.agent);
                } else if (event.type == "DEATH") {
                    printDeath(event.agent);
                } else if (event.type != "VIRTUAL") {
                    std::cerr << "Unhandled event: " << event.type << std::endl;
                    return 1;
                }
            }
        }
        printStep(timestep);
    }
    return 0;
}

void printBirth(const std::string& run, int agent) {
    std::cout << "BIRTH " << agent << std::endl;
    std::string path = run + "/genome/agents/genome_" + std::to_string(agent) + ".txt";
    AbstractFile* file = AbstractFile::open(globals::recordFileType, path.c_str(), "r");
    file->cat();
    delete file;
    std::cout << std::endl;
}

void printDeath(int agent) {
    std::cout << "DEATH " << agent << std::endl;
}

void printStep(int step) {
    std::cout << "STEP " << step << std::endl;
}
