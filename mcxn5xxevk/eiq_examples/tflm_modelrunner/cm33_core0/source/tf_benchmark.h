#ifndef TENSORFLOW_LITE_MICRO_BENCHMARK_MAIN_FUNCTIONS_H_
#define TENSORFLOW_LITE_MICRO_BENCHMARK_MAIN_FUNCTIONS_H_

#include "modelrunner.h"

int Model_Setup(NNServer* server);
int Model_RunInference(NNServer* server);

#endif  // TENSORFLOW_LITE_MICRO_BENCHMARK_MAIN_FUNCTIONS_H_
