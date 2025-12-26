#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Forward declarations for PhysX types
namespace physx {
class PxPhysics;
class PxTriangleMesh;
struct PxVec3;
struct PxTransform;
}  // namespace physx

namespace larcs::sim {

class USDLoader {
 public:
  struct MeshData {
    std::string name;
    std::vector<physx::PxVec3> vertices;
    std::vector<uint32_t> indices;
    physx::PxTransform transform;
  };

  // USD support placeholder - Phase 2.2 will implement full USD integration
  static bool load(const std::string& usd_path, std::vector<MeshData>& meshes);

  // USD → PhysX Geometry変換
  static physx::PxTriangleMesh* create_triangle_mesh(
      physx::PxPhysics* physics, const MeshData& mesh_data);
};

}  // namespace larcs::sim
