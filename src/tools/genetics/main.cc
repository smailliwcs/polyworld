#include <iostream>
#include <map>
#include <string>

#include "genome/GenomeUtil.h"
#include "sim/globals.h"
#include "utils/AbstractFile.h"
#include "utils/analysis.h"
#include "utils/misc.h"

struct Args {
    std::string run;
};

void printUsage(int, char**);
bool tryParseArgs(int, char**, Args&);
void printBirth(std::string, int);
void printDeath(int);
void printStep(int);

int main(int argc, char** argv) {
    Args args;
    if (!tryParseArgs(argc, argv, args)) {
        printUsage(argc, argv);
        return 1;
    }
    analysis::initialize(args.run);
    std::cout << "SIZE " << genome::GenomeUtil::schema->getMutableSize() << std::endl;
    int initAgentCount = analysis::getInitAgentCount(args.run);
    for (int agent = 1; agent <= initAgentCount; agent++) {
        printBirth(args.run, agent);
    }
    printStep(0);
    std::map<int, std::list<analysis::Event> > events = analysis::getEvents(args.run);
    int maxTimestep = analysis::getMaxTimestep(args.run);
    for (int timestep = 1; timestep <= maxTimestep; timestep++) {
        std::map<int, std::list<analysis::Event> >::iterator eventsIter = events.find(timestep);
        if (eventsIter != events.end()) {
            itfor(std::list<analysis::Event>, eventsIter->second, timestepEventsIter) {
                analysis::Event event = *timestepEventsIter;
                if (event.type == "BIRTH") {
                    printBirth(args.run, event.agent);
                } else if (event.type == "DEATH") {
                    printDeath(event.agent);
                } else if (event.type != "VIRTUAL") {
                    std::cerr << "Unhandled event: " << event.type << std::endl;
                }
            }
        }
        printStep(timestep);
    }
    return 0;
}

void printUsage(int argc, char** argv) {
    std::cerr << "Usage: " << argv[0] << " RUN" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Generates the population genetics log for an existing run." << std::endl;
    std::cerr << std::endl;
    std::cerr << "  RUN  Run directory" << std::endl;
}

bool tryParseArgs(int argc, char** argv, Args& args) {
    if (argc != 2) {
        return false;
    }
    std::string run;
    try {
        int argi = 1;
        run = std::string(argv[argi++]);
        if (!exists(run + "/endStep.txt")) {
            return false;
        }
    } catch (...) {
        return false;
    }
    args.run = run;
    return true;
}

void printBirth(std::string run, int agent) {
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
