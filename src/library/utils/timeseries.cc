#include "timeseries.h"

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "brain/Brain.h"
#include "brain/NeuronModel.h"
#include "brain/RqNervousSystem.h"
#include "sim/globals.h"
#include "utils/AbstractFile.h"
#include "utils/misc.h"

void timeseries::writeHeader(std::ostream& out, int agent, RqNervousSystem* cns) {
    out << "# AGENT " << agent << std::endl;
    NeuronModel::Dimensions dims = cns->getBrain()->getDimensions();
    out << "# DIMENSIONS";
    out << " " << dims.numNeurons;
    out << " " << dims.numInputNeurons;
    out << " " << dims.numOutputNeurons;
    out << std::endl;
}

void timeseries::writeNerves(std::ostream& out, RqNervousSystem* cns) {
    out << "# BEGIN NERVES" << std::endl;
    const NervousSystem::NerveList& nerves = cns->getNerves();
    citfor(NervousSystem::NerveList, nerves, it) {
        out << (*it)->name << " " << (*it)->getNeuronCount() << std::endl;
    }
    out << "# END NERVES" << std::endl;
}

void timeseries::writeSynapses(std::ostream& out, RqNervousSystem* cns) {
    std::map<short, std::set<short> > synapses;
    NeuronModel::Dimensions dims = cns->getBrain()->getDimensions();
    NeuronModel* model = cns->getBrain()->getNeuronModel();
    for (int synapse = 0; synapse < dims.numSynapses; synapse++) {
        short neuron1;
        short neuron2;
        float weight;
        float learningRate;
        model->get_synapse(synapse, neuron1, neuron2, weight, learningRate);
        synapses[neuron1].insert(neuron2);
    }
    out << "# BEGIN SYNAPSES" << std::endl;
    for (int neuron = 0; neuron < dims.numNeurons; neuron++) {
        if (synapses[neuron].size() == 0) {
            continue;
        }
        out << neuron;
        citfor(std::set<short>, synapses[neuron], it) {
            out << " " << *it;
        }
        out << std::endl;
    }
    out << "# END SYNAPSES" << std::endl;
}

void timeseries::writeInVivo(std::ostream& out, const std::string& run, int agent) {
    std::string path = run + "/brain/function/brainFunction_" + std::to_string(agent) + ".txt";
    if (!AbstractFile::exists(globals::recordFileType, path.c_str())) {
        path = run + "/brain/function/incomplete_brainFunction_" + std::to_string(agent) + ".txt";
    }
    AbstractFile* file = AbstractFile::open(globals::recordFileType, path.c_str(), "r");
    int rc;
    int version;
    rc = file->scanf("version %d", &version);
    assert(rc == 1);
    assert(version == 1);
    long _agent;
    int numNeurons;
    rc = file->scanf("brainFunction %ld %d", &_agent, &numNeurons);
    assert(rc == 2);
    assert(_agent == agent);
    out << "# BEGIN TIME SERIES" << std::endl;
    while (true) {
        bool done = false;
        for (int neuron = 0; neuron < numNeurons; neuron++) {
            int _neuron;
            double activation;
            rc = file->scanf("%d %lf", &_neuron, &activation);
            if (rc <= 0) {
                assert(neuron == 0);
                done = true;
                break;
            }
            assert(rc == 2);
            assert(_neuron == neuron);
            if (neuron > 0) {
                out << " ";
            }
            out << activation;
        }
        if (done) {
            break;
        }
        out << std::endl;
    }
    out << "# END TIME SERIES" << std::endl;
    delete file;
}

void timeseries::writeInVitro(std::ostream& out, RqNervousSystem* cns, int transient, int steps) {
    cns->getBrain()->randomizeActivations();
    for (int step = 1; step <= transient; step++) {
        cns->update(false);
    }
    NeuronModel::Dimensions dims = cns->getBrain()->getDimensions();
    double* activations = new double[dims.numNeurons];
    out << "# BEGIN TIME SERIES" << std::endl;
    for (int step = 1; step <= steps; step++) {
        cns->getBrain()->getActivations(activations + dims.numInputNeurons, dims.numInputNeurons, dims.getNumNonInputNeurons());
        cns->update(false);
        cns->getBrain()->getActivations(activations, 0, dims.numInputNeurons);
        for (int neuron = 0; neuron < dims.numNeurons; neuron++) {
            if (neuron > 0) {
                out << " ";
            }
            out << activations[neuron];
        }
        out << std::endl;
    }
    out << "# END TIME SERIES" << std::endl;
    delete[] activations;
}
