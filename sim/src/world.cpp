#include "larcs/sim/world.hpp"

#include <spdlog/spdlog.h>

namespace larcs::sim {

World::World() = default;

World::~World() { shutdown(); }

bool World::initialize() {
  spdlog::info("Initializing PhysX World (stub implementation)");

  // TODO: Implement PhysX initialization when PhysX is installed
  // - Create foundation
  // - Create physics
  // - Create scene with gravity
  // - Create default material

  // For now, just return success
  sim_time_ = 0.0;
  time_scale_ = 1.0;

  spdlog::info("PhysX World initialized (stub)");
  return true;
}

void World::shutdown() {
  if (scene_ == nullptr) {
    return;
  }

  spdlog::info("Shutting down PhysX World (stub)");

  // TODO: Implement PhysX cleanup
  // - Release scene
  // - Release physics
  // - Release foundation

  scene_ = nullptr;
  physics_ = nullptr;
  foundation_ = nullptr;
}

void World::step(double dt) {
  // TODO: Implement PhysX simulation step
  // scene_->simulate(dt);
  // scene_->fetchResults(true);

  sim_time_ += dt * time_scale_;
}

}  // namespace larcs::sim
