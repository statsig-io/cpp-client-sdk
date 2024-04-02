#pragma once

#include <string>

#include "statsig_user.h"

namespace statsig {

enum class ValueSource {
  Uninitialized,
  Loading,
  NoValues,
  Cache,
  Network,
  Bootstrap
};

std::string GetValueSourceString(const ValueSource &source) {
  switch (source) {
    case ValueSource::Uninitialized:return "Uninitialized";
    case ValueSource::Loading:return "Loading";
    case ValueSource::NoValues:return "NoValues";
    case ValueSource::Cache:return "Cache";
    case ValueSource::Network:return "Network";
    case ValueSource::Bootstrap:return "Bootstrap";
  }
}

class EvaluationsDataProvider {
 public:
  virtual std::optional<std::string> GetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user
  ) {
    return std::nullopt;
  };

  virtual void SetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user,
      const std::string &data
  ) {}

  virtual ValueSource GetSource() {
    return ValueSource::NoValues;
  }

  virtual bool IsTerminal() {
    return false;
  }
};

}

