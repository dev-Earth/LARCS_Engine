#pragma once
#include <memory>
#include <string>

#include "larcs/sim/sensor/sensor_base.hpp"

// Forward declare yaml-cpp to avoid requiring it in headers
namespace YAML {
class Node;
}

namespace larcs::sim {

class SensorFactory {
 public:
  static std::unique_ptr<SensorBase> create_from_yaml(
      const std::string& type, const std::string& name,
      const YAML::Node& spec_node);

  // USD support is placeholder for Phase 2.2
  // static std::unique_ptr<SensorBase> create_from_usd(
  //   const std::string& type,
  //   const std::string& name,
  //   const pxr::UsdPrim& prim
  // );
};

}  // namespace larcs::sim
