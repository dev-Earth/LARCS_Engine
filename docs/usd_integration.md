# USD Integration Guide

## What is USD?

USD (Universal Scene Description) is Pixar's open-source framework for describing, composing, simulating, and collaborating on 3D scenes. It has become an industry standard for interchange between 3D tools.

### Key Benefits for LARCS

- **Industry Standard**: Used by major studios and robotics companies
- **CAD Integration**: Direct export from Fusion360, SolidWorks, etc.
- **Hierarchical**: Naturally represents robot assemblies
- **Extensible**: Can embed custom metadata (sensors, joints, etc.)
- **Composable**: Reference and layer multiple USD files

## Fusion360 → USD → PhysX Pipeline

```
Fusion360 Design
      ↓ Export
   USD File (.usd/.usda)
      ↓ Load
   LARCS Matrix
      ↓ Convert
   PhysX Simulation
```

### Phase 2.1: Basic Mesh Import (Current)

**Supported:**
- Mesh geometry (triangles)
- Transform hierarchies
- Basic material properties

**Not Yet Supported:**
- Physics properties (mass, inertia)
- Joint definitions
- Sensor metadata
- Complex materials

**Example USD Structure:**
```
/Robot
  /Base
    - Mesh: base.obj
    - Transform: [x, y, z]
  /LeftWheel
    - Mesh: wheel.obj
    - Transform: [x, y, z]
  /RightWheel
    - Mesh: wheel.obj
    - Transform: [x, y, z]
```

### Phase 2.2: Physics Properties (Planned)

**Will Support:**
- Mass and center of mass
- Inertia tensors
- Collision vs visual geometry
- Material physics (friction, restitution)

**USD Physics Schema:**
```python
# In USD file
def Xform "Robot" (
    physics:mass = 5.0
    physics:centerOfMass = (0, 0, 0.1)
)
{
    # ...
}
```

### Phase 2.3: Full Automation (Future)

**Fusion360 Plugin:**
- One-click export to USD
- Automatic physics property extraction
- Sensor placement via components
- Joint definition from CAD constraints

**Sensor Metadata:**
```python
# Sensor embedded in USD
def Xform "IMU_Sensor" (
    custom string larcs:sensor:type = "imu"
    custom string larcs:sensor:model = "BMI088"
    custom double larcs:sensor:rate = 200
)
```

## Current USD Implementation

### Loading USD Files

```cpp
#include "larcs/sim/usd_loader.hpp"

std::vector<larcs::sim::USDLoader::MeshData> meshes;
if (larcs::sim::USDLoader::load("robot.usd", meshes)) {
    for (const auto& mesh : meshes) {
        // Convert to PhysX
        auto px_mesh = larcs::sim::USDLoader::create_triangle_mesh(
            physics, mesh
        );
    }
}
```

### USD File Format

USD files can be ASCII (.usda) or binary (.usd):

**ASCII Example (robot.usda):**
```python
#usda 1.0

def Xform "Robot"
{
    def Mesh "Base"
    {
        float3[] extent = [(-0.15, -0.125, -0.075), (0.15, 0.125, 0.075)]
        int[] faceVertexCounts = [3, 3, 3, ...]
        int[] faceVertexIndices = [0, 1, 2, ...]
        point3f[] points = [(0, 0, 0), (0.1, 0, 0), ...]
        
        matrix4d xformOp:transform = ( (1, 0, 0, 0), (0, 1, 0, 0), ... )
    }
}
```

## USD Tools

### Install USD

See `scripts/install_usd.sh` for automated installation.

Manual installation:
```bash
python3 build_scripts/build_usd.py /usr/local/USD
```

### View USD Files

```bash
# View USD file structure
usdcat robot.usd

# View as ASCII
usdcat robot.usd --usdFormat usda

# Validate USD file
usdchecker robot.usd
```

### Convert Formats

```bash
# Binary to ASCII
usdcat robot.usd -o robot.usda

# ASCII to Binary
usdcat robot.usda -o robot.usd
```

## Exporting from CAD Tools

### Fusion360

**Current (Phase 2.1):**
1. File → Export
2. Select "OBJ" or "STL" format
3. Manually create USD from mesh (complex)

**Future (Phase 2.3):**
1. Install LARCS Fusion360 plugin
2. Select robot assembly
3. Click "Export to LARCS"
4. Done! USD file with all metadata created

### SolidWorks

- Use USD Exporter plugin (if available)
- Or export as STEP → convert to USD
- Manual metadata addition required

### Blender

Blender has native USD support:
1. File → Export → Universal Scene Description (.usd)
2. Select objects
3. Configure export options
4. Export

## USD Schema Extensions

### Custom LARCS Schema (Phase 2.3)

We will define custom USD schemas for LARCS:

```python
# larcs_sensor.usda
class "LarcsSensor" (
    inherits = </Xform>
)
{
    string larcs:type
    string larcs:model
    string larcs:topic
    double larcs:rate
}

class "LarcsEncoder" (
    inherits = </LarcsSensor>
)
{
    int larcs:resolution
    double larcs:wheel_radius
}
```

### Using Custom Schema

```python
# In robot USD file
def "LeftWheel" (
    prepend apiSchemas = ["LarcsEncoder"]
)
{
    string larcs:type = "encoder"
    string larcs:model = "incremental"
    int larcs:resolution = 2048
    double larcs:wheel_radius = 0.05
}
```

## Best Practices

### File Organization

```
robot/
  ├── robot.usd         # Main assembly
  ├── base.usd          # Base component
  ├── wheel.usd         # Wheel (referenced twice)
  ├── meshes/
  │   ├── base.obj
  │   └── wheel.obj
  └── textures/
      └── ...
```

### Naming Conventions

- Use descriptive names: `LeftWheel`, not `Object_01`
- Follow hierarchy: `/Robot/Base/Sensor_IMU`
- Use standard prefixes for sensors: `IMU_`, `Encoder_`, `LiDAR_`

### Performance Tips

- Use instancing for repeated geometry (wheels)
- Simplify collision meshes (convex decomposition)
- Separate visual and collision geometry
- Use LOD (Level of Detail) for complex models

## Limitations

### Phase 2.1 Limitations

- No physics properties from USD
- Manual sensor configuration required
- No joint support
- Limited material properties

### General USD Limitations

- Large files for complex meshes
- Requires USD library (not in vcpkg)
- Learning curve for creating custom schemas

## Future Roadmap

### Phase 2.2: Physics Integration
- Implement UsdPhysics schema support
- Mass, inertia, friction from USD
- Collision geometry separation
- Joint definitions

### Phase 2.3: Automation
- Fusion360 plugin with UI
- Automatic property extraction
- Sensor placement tools
- One-click workflow

### Phase 3+: Advanced Features
- Dynamic loading/unloading
- Scene composition
- Multi-robot assemblies
- Integration with NVIDIA Omniverse

## Example Workflow

### 1. Design in Fusion360

Create your robot with proper:
- Component structure
- Mass properties
- Constraints (joints)

### 2. Export USD (Manual - Phase 2.1)

```bash
# Export meshes
# Create USD manually or use converter
```

### 3. Create Config (Phase 2.1)

```yaml
# sim_config.yaml
robot:
  usd_file: "robot.usd"
  sensors:
    - name: "imu_base"
      type: "imu"
      # ... manual configuration
```

### 4. Load in LARCS Matrix

```bash
larcs-matrix --config sim_config.yaml
```

## Resources

- [USD Documentation](https://graphics.pixar.com/usd/docs/index.html)
- [USD Python API](https://graphics.pixar.com/usd/docs/api/index.html)
- [OpenUSD GitHub](https://github.com/PixarAnimationStudios/USD)
- [USD Working Group](https://www.aswf.io/projects/openusd/)

## Getting Help

For USD-related questions:
- Check USD documentation
- Visit USD forum
- Open issue on LARCS GitHub
