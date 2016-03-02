#include "BeingCarriedSensor.h"

#include <assert.h>

#include "agent.h"
#include "brain/NervousSystem.h"
#include "utils/RandomNumberGenerator.h"

#define DISABLE_PROPRIOCEPTION true

BeingCarriedSensor::BeingCarriedSensor( agent *self )
{
	this->self = self;
}

BeingCarriedSensor::~BeingCarriedSensor()
{
}

void BeingCarriedSensor::sensor_grow( NervousSystem *cns )
{
	nerve = cns->getNerve( "BeingCarried" );
}

void BeingCarriedSensor::sensor_prebirth_signal( RandomNumberGenerator *rng )
{
	if( Brain::config.enableLearning )
		nerve->set( rng->drand() );
	else
		nerve->set( 0.0 );
}

void BeingCarriedSensor::sensor_update( bool print )
{
	float activation;

	if( self->BeingCarried() )
		activation = 1.0;
	else
		activation = 0.0;

	nerve->set( activation );
}
