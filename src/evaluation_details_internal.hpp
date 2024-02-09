#pragma once

#include "evaluation_details.h"

namespace statsig::evaluation_details {

struct SourceInfo {
  ValueSource source;
  long lcut;
  long received_at;
};

EvaluationDetails Uninitialized() {
  return {"Uninitialized", 0, 0};
}

EvaluationDetails UnrecognizedFromSourceInfo(SourceInfo info) {
  auto result = GetValueSourceString(info.source);

  if (info.source == ValueSource::Uninitialized || info.source == ValueSource::NoValues) {
    return {result, 0, 0};
  }

  return {result + ":Unrecognized", info.lcut, info.received_at};
}

EvaluationDetails RecognizedFromSourceInfo(SourceInfo info) {
  auto result = GetValueSourceString(info.source);
  return {result + ":Recognized", info.lcut, info.received_at};
}

}