#ifndef packedForest_h
#define packedForest_h

#ifdef LIKWID_PERFMON
#include <likwid.h>
#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#define LIKWID_MARKER_RESET(regionTag)
#endif



#include "baseFunctions/buildSpecific.h"
#include "baseFunctions/timeLogger.h"
#include "fpSingleton/fpSingleton.h"
#include "baseFunctions/fpForestFactory.h"
#include "baseFunctions/fpForestBase.h"
#include "baseFunctions/fpForest.h"

extern std::map<int, int> nodeCardinalityMap;
extern std::map<int, int> map_node_to_subtree ;
extern std::map<int, int> map_subtree_to_class;
extern std::map<int, int> map_subtree_to_size;
extern 	std::map<int, int> map_node_to_interleaved;
#endif //packedForest_h
