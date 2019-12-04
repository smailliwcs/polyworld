#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "brain/Brain.h"
#include "brain/RqNervousSystem.h"
#include "genome/Genome.h"
#include "genome/GenomeUtil.h"
#include "sim/globals.h"
#include "utils/AbstractFile.h"
#include "utils/analysis.h"
#include "utils/datalib.h"
#include "utils/misc.h"

#define COMMA ,

struct Arguments {
    std::vector<std::string> args;
    bool help;
    std::string driven;
    std::string passive;

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
    out << "Usage: " << args[0] << " DRIVEN PASSIVE" << std::endl;
    out << std::endl;
    out << "Generate selection-agnostic agents paired to a driven run." << std::endl;
    out << "I.e., simulate a passive run in an offline fashion." << std::endl;
    out << std::endl;
    out << "  DRIVEN   Driven run directory" << std::endl;
    out << "  PASSIVE  Passive run directory" << std::endl;
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
        PARSE(driven, , exists(driven + "/endStep.txt"), "Not a Polyworld run")
        PARSE(passive, , !exists(passive), "Already exists")
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

void copyDir(const std::string& source, const std::string& target, const std::string& path);
void copyFile(const std::string& source, const std::string& target, const std::string& path);
void copyAbstractFile(const std::string& source, const std::string& target, const std::string& path);
void initialize(const std::string& driven, const std::string& passive);
std::ofstream openBirthsDeathsLog(const std::string& run);
DataLibWriter openLifeSpansWriter(const std::string& run);
std::map<int, genome::Genome*>::iterator choose(std::map<int, genome::Genome*>& genomes);
void logGenome(const std::string& run, int agent, genome::Genome* genome);
void logSynapses(const std::string& run, int agent, const std::string& stage, RqNervousSystem* cns);

int main(int argc, char** argv) {
    Arguments arguments(argc, argv);
    if (!arguments.parse()) {
        return 1;
    }
    if (arguments.help) {
        std::cout << arguments.usage();
        return 0;
    }
    initialize(arguments.driven, arguments.passive);
    std::map<int, int> births;
    std::map<int, genome::Genome*> genomes;
    std::ofstream log = openBirthsDeathsLog(arguments.passive);
    DataLibWriter writer = openLifeSpansWriter(arguments.passive);
    int initAgentCount = analysis::getInitAgentCount(arguments.driven);
    for (int agent = 1; agent <= initAgentCount; agent++) {
        births[agent] = 0;
        genomes[agent] = analysis::getGenome(arguments.driven, agent);
        copyAbstractFile(arguments.driven, arguments.passive, "/genome/agents/genome_" + std::to_string(agent) + ".txt");
        if (Brain::config.learningMode != Brain::Configuration::LEARN_NONE) {
            copyAbstractFile(arguments.driven, arguments.passive, "/brain/synapses/synapses_" + std::to_string(agent) + "_incept.txt");
        }
        copyAbstractFile(arguments.driven, arguments.passive, "/brain/synapses/synapses_" + std::to_string(agent) + "_birth.txt");
    }
    std::map<int, std::list<analysis::Event> > events = analysis::getEvents(arguments.driven);
    int maxTimestep = analysis::getMaxTimestep(arguments.driven);
    for (int timestep = 1; timestep <= maxTimestep; timestep++) {
        if (timestep % 100 == 0) {
            std::cerr << timestep << std::endl;
        }
        std::map<int, std::list<analysis::Event> >::iterator eventsIter = events.find(timestep);
        if (eventsIter != events.end()) {
            itfor(std::list<analysis::Event>, eventsIter->second, timestepEventsIter) {
                analysis::Event event = *timestepEventsIter;
                if (event.type == "BIRTH" || event.type == "CREATION") {
                    births[event.agent] = timestep;
                    std::map<int, genome::Genome*>::iterator parent1 = choose(genomes);
                    std::map<int, genome::Genome*>::iterator parent2;
                    do {
                        parent2 = choose(genomes);
                    } while (parent2->first == parent1->first);
                    genome::Genome* child = genome::GenomeUtil::createGenome();
                    child->crossover(parent1->second, parent2->second, true);
                    genomes[event.agent] = child;
                    logGenome(arguments.passive, event.agent, child);
                    RqNervousSystem* cns = new RqNervousSystem();
                    cns->grow(child);
                    if (Brain::config.learningMode != Brain::Configuration::LEARN_NONE) {
                        logSynapses(arguments.passive, event.agent, "incept", cns);
                    }
                    cns->prebirth();
                    logSynapses(arguments.passive, event.agent, "birth", cns);
                    delete cns;
                    log << timestep << " BIRTH " << event.agent << " " << parent1->first << " " << parent2->first << std::endl;
                } else if (event.type == "DEATH") {
                    std::map<int, genome::Genome*>::iterator genome = choose(genomes);
                    genomes.erase(genome->first);
                    delete genome->second;
                    log << timestep << " DEATH " << genome->first << std::endl;
                    writer.addRow(genome->first, births[genome->first], "PASSIVE", timestep, "PASSIVE");
                } else {
                    std::cerr << "Unhandled event: " << event.type << std::endl;
                    return 1;
                }
            }
        }
    }
    itfor(std::map<int COMMA genome::Genome*>, genomes, genomesIter) {
        writer.addRow(genomesIter->first, births[genomesIter->first], "PASSIVE", maxTimestep, "PASSIVE");
        delete genomesIter->second;
    }
    log.close();
    return 0;
}

void copyDir(const std::string& source, const std::string& target, const std::string& path) {
    SYSTEM(("cp -R " + (source + path) + " " + (target + path)).c_str());
}

void copyFile(const std::string& source, const std::string& target, const std::string& path) {
    SYSTEM(("cp " + (source + path) + " " + (target + path)).c_str());
}

void copyAbstractFile(const std::string& source, const std::string& target, const std::string& path) {
    std::string extension = globals::recordFileType == AbstractFile::TYPE_GZIP_FILE ? ".gz" : "";
    SYSTEM(("cp " + (source + path + extension) + " " + (target + path + extension)).c_str());
}

void initialize(const std::string& driven, const std::string& passive) {
    analysis::initialize(driven);
    makeDirs(passive);
    copyFile(driven, passive, "/endStep.txt");
    copyFile(driven, passive, "/original.wfs");
    copyFile(driven, passive, "/original.wf");
    copyFile(driven, passive, "/normalized.wf");
    makeDirs(passive + "/genome");
    copyDir(driven, passive, "/genome/meta");
    makeDirs(passive + "/genome/agents");
    makeDirs(passive + "/brain/synapses");
}

std::ofstream openBirthsDeathsLog(const std::string& run) {
    std::ofstream log(run + "/BirthsDeaths.log");
    log << "% Timestep Event Agent# Parent1 Parent2" << std::endl;
    return log;
}

DataLibWriter openLifeSpansWriter(const std::string& run) {
    DataLibWriter writer((run + "/lifespans.txt").c_str());
    const char* columnNames[] = { "Agent", "BirthStep", "BirthReason", "DeathStep", "DeathReason", NULL };
    const datalib::Type columnTypes[] = { datalib::INT, datalib::INT, datalib::STRING, datalib::INT, datalib::STRING };
    writer.beginTable("LifeSpans", columnNames, columnTypes);
    return writer;
}

std::map<int, genome::Genome*>::iterator choose(std::map<int, genome::Genome*>& genomes) {
    std::map<int, genome::Genome*>::iterator genome = genomes.begin();
    std::advance(genome, (int)rrand(0, genomes.size()));
    return genome;
}

void logGenome(const std::string& run, int agent, genome::Genome* genome) {
    std::string path = run + "/genome/agents/genome_" + std::to_string(agent) + ".txt";
    AbstractFile* file = AbstractFile::open(globals::recordFileType, path.c_str(), "w");
    genome->dump(file);
    delete file;
}

void logSynapses(const std::string& run, int agent, const std::string& stage, RqNervousSystem* cns) {
    std::string path = run + "/brain/synapses/synapses_" + std::to_string(agent) + "_" + stage + ".txt";
    AbstractFile* file = AbstractFile::open(globals::recordFileType, path.c_str(), "w");
    cns->getBrain()->dumpSynapses(file, agent);
    delete file;
}
