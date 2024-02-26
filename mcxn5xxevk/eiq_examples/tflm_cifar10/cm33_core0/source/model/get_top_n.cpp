/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.
   Copyright 2020 NXP

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "get_top_n.h"

// Returns the top N confidence values over threshold in the provided vector,
// sorted by confidence in descending order.
void MODEL_GetTopN(const uint8_t* tensorData, int tensorSize, tensor_type_t tensorType,
                   size_t numResults, float threshold, result_t* topResults)
{
  for (int i = 0; i < numResults; i++) {
    topResults[i] = {.score = 0.0f, .index = -1};
  }

  const long count = tensorSize;
  for (int i = 0; i < count; i++) {
    float value = 0.0f;
    switch (tensorType) {
      case kTensorType_FLOAT32: {
        const float* predictions = reinterpret_cast<const float*>(tensorData);
        value = predictions[i];
        break;
      }
      case kTensorType_UINT8: {
        const uint8_t* predictions = tensorData;
        value = predictions[i] / 255.0;
        break;
      }
      case kTensorType_INT8: {
        const int8_t* predictions = reinterpret_cast<const int8_t*>(tensorData);
        value = ((int)predictions[i] + 128) / 255.0;
        break;
      }
    }
    // Only add it if it beats the threshold and has a chance at being in
    // the top N.
    if (value < threshold) {
      continue;
    }

    result_t pass = {.score = 0.0f, .index = -1};
    for (int n = 0; n < numResults; n++) {
      if (pass.index >= 0) {
        result_t swap = topResults[n];
        topResults[n] = pass;
        pass = swap;
      } else if (topResults[n].score < value) {
        pass = topResults[n];
        topResults[n] = {.score = value, .index = i};
      }
    }
  }
}
