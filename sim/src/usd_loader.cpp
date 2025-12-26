#include "larcs/sim/usd_loader.hpp"

#include <spdlog/spdlog.h>

namespace larcs::sim {

bool USDLoader::load(const std::string& usd_path,
                     std::vector<MeshData>& meshes) {
  spdlog::warn("USD loading not yet implemented (Phase 2.2)");
  spdlog::info("Would load USD file: {}", usd_path);

  // TODO: Implement USD loading when USD library is available
  // - Open USD stage
  // - Traverse hierarchy
  // - Extract meshes and transforms
  // - Convert to PhysX format

  (void)meshes;  // Suppress warning
  return false;
}

physx::PxTriangleMesh* USDLoader::create_triangle_mesh(
    physx::PxPhysics* physics, const MeshData& mesh_data) {
  spdlog::warn("PhysX triangle mesh creation not yet implemented");

  // TODO: Implement when PhysX is available
  // - Create PxTriangleMeshDesc
  // - Set vertices and indices
  // - Cook mesh
  // - Create PxTriangleMesh

  (void)physics;
  (void)mesh_data;
  return nullptr;
}

}  // namespace larcs::sim
