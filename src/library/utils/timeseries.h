#pragma once

#include <iostream>

#include "brain/RqNervousSystem.h"

namespace timeseries {
    void writeHeader(std::ostream&, int, RqNervousSystem*);
    void writeNerves(std::ostream&, RqNervousSystem*);
    void writeSynapses(std::ostream&, RqNervousSystem*);
    void writeTimeSeries(std::ostream&, RqNervousSystem*, int, int);
}
