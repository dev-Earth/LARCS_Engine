#pragma once
#include <memory>
#include <vector>

// Stub PhysX types for now - will be replaced when PhysX is available
// This allows compilation without PhysX installed
namespace physx {
class PxFoundation;
class PxPhysics;
class PxScene;
class PxMaterial;
class PxDefaultCpuDispatcher;
class PxDefaultAllocator;
class PxDefaultErrorCallback;
}  // namespace physx

namespace larcs::sim {

class World {
 public:
  World();
  ~World();

  bool initialize();
  void shutdown();

  // シミュレーションステップ
  void step(double dt);

  // PhysX Scene取得
  physx::PxScene* get_scene() { return scene_; }
  physx::PxPhysics* get_physics() { return physics_; }

  // 時刻管理
  double get_sim_time() const { return sim_time_; }
  void set_time_scale(double scale) { time_scale_ = scale; }
  double get_time_scale() const { return time_scale_; }

 private:
  physx::PxDefaultAllocator* allocator_ = nullptr;
  physx::PxDefaultErrorCallback* error_callback_ = nullptr;
  physx::PxFoundation* foundation_ = nullptr;
  physx::PxPhysics* physics_ = nullptr;
  physx::PxDefaultCpuDispatcher* dispatcher_ = nullptr;
  physx::PxScene* scene_ = nullptr;
  physx::PxMaterial* default_material_ = nullptr;

  double sim_time_ = 0.0;
  double time_scale_ = 1.0;  // リアルタイム=1.0, 10倍速=10.0
};

}  // namespace larcs::sim
