#ifndef __ENGINE_H__
#define __ENGINE_H__


#include "common.h"
#include "engine/types.h"
#include "engine/spike_train.h"
#include "engine/neuron.h"
#include "engine/synapse.h"
#include "engine/layer.h"
#include "engine/network.h"
#include "engine/data_gen.h"
#include "engine/callback.h"
#include "engine/experiment.h"

#include "engine/spike_train.c"
#include "engine/neuron.c"
#include "engine/synapse.c"
#include "engine/layer.c"
#include "engine/network.c"
#include "engine/data_gen.c"
#include "engine/callback.c"
#include "engine/callbacks/meta_dumper.c"
#include "engine/callbacks/data_dumper.c"
#include "engine/experiment.c"

#endif // __ENGINE_H__
