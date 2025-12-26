# Fusion360 → LARCS Workflow

## Overview

This guide explains how to create robot models in Autodesk Fusion360 and use them in LARCS Matrix simulation. The workflow evolves across three phases from manual to fully automated.

## Phase 2.1: Manual Export (Current)

### Step 1: Design Your Robot

**Component Structure:**
```
Robot Assembly
├── Base
├── LeftWheel
├── RightWheel
├── IMU_Mount (for sensor placement)
└── Additional Components
```

**Best Practices:**
- Use clear, descriptive names
- Organize components hierarchically
- Set proper mass properties (Physical Material)
- Use constraints for movable parts (wheels, arms)

### Step 2: Set Physical Properties

For each component:
1. Right-click → Physical Material
2. Select appropriate material (Aluminum, Steel, Plastic, etc.)
3. Or set custom density
4. Verify mass in component properties

### Step 3: Export Meshes

**For Each Component:**
1. Right-click component → Save As Mesh
2. Format: STL or OBJ
3. Quality: High (for visual), Low (for collision)
4. Export to `meshes/` directory

**Export Settings:**
```
Format: STL Binary
Refinement: High
Units: Millimeters (convert to meters in LARCS)
Structure: One file per component
```

### Step 4: Create YAML Config

Manually create robot configuration:

```yaml
# configs/my_robot.yaml
robot:
  name: "my_robot"
  
  base:
    mass: 5.0  # Copy from Fusion360
    size: [0.3, 0.25, 0.15]
    # mesh: "meshes/base.stl"  # Future
  
  wheels:
    - name: "left_wheel"
      radius: 0.05  # Measure in Fusion360
      width: 0.03
      position: [0, 0.15, 0]  # From component transform
      encoder:
        model: "incremental"
        resolution: 2048
        wheel_radius: 0.05
        topic: "/robot/wheel/left"
  
  sensors:
    - name: "imu_base"
      type: "imu"
      position: [0, 0, 0.05]  # From IMU_Mount position
      spec:
        model: "BMI088"
        # ... sensor specs
```

### Step 5: Run Simulation

```bash
larcs-matrix --config configs/my_robot.yaml
```

## Phase 2.2: USD Export (Planned Q1 2024)

### Step 1: Design in Fusion360

Same as Phase 2.1, with emphasis on:
- Accurate mass properties
- Proper joint definitions
- Component naming for sensors

### Step 2: Export to USD

**Option A: Third-Party Plugin**
```
# If USD exporter available for Fusion360
File → Export → USD (.usd)
```

**Option B: STEP → USD Converter**
```bash
# Export as STEP
File → Export → STEP

# Convert to USD (using custom tool)
step2usd robot.step -o robot.usd
```

### Step 3: Edit USD Metadata

Add LARCS-specific information:

```python
# Add to robot.usd
def Xform "Base" (
    physics:mass = 5.0
    physics:centerOfMass = (0, 0, 0.05)
)

def Xform "IMU_Mount" (
    custom string larcs:sensor:type = "imu"
    custom string larcs:sensor:model = "BMI088"
)
```

### Step 4: Load in LARCS

```bash
larcs-matrix --usd robot.usd
```

## Phase 2.3: Fusion360 Plugin (Planned Q2 2024)

### Installation

```bash
# Download LARCS Fusion360 Plugin
# Install via Fusion360 Add-ins Manager
```

### Usage

**1. Design Robot in Fusion360**

No special requirements! Just design normally.

**2. Mark Sensor Locations**

Create components with special names:
- `LiDAR_Front` - LiDAR sensor
- `Camera_Main` - RGB camera
- `IMU_Base` - IMU sensor
- `Encoder_LeftWheel` - Encoder (on wheel)

**3. One-Click Export**

1. Select root component
2. Click "LARCS" in toolbar
3. Click "Export for Simulation"
4. Choose export location
5. Done!

**Plugin Automatically:**
- Extracts all mesh data
- Calculates mass properties
- Identifies sensors by naming
- Creates complete USD file
- Generates basic config YAML
- Validates export

**4. Review and Customize**

```bash
# Generated files:
robot.usd          # Complete USD with physics
robot_config.yaml  # Simulation parameters
sensors.yaml       # Sensor specifications (auto-detected)
```

Edit `robot_config.yaml` if needed:
```yaml
# Auto-generated, but customizable
sensors:
  - name: "LiDAR_Front"  # Detected from component name
    type: "lidar"        # Inferred from prefix
    model: "VLP-16"      # Default, can override
    rate: 10             # Hz
```

**5. Run in LARCS**

```bash
larcs-matrix --usd robot.usd --config robot_config.yaml
```

## Component Naming Conventions

### Sensor Prefixes

The plugin recognizes these prefixes:

| Prefix | Sensor Type | Example |
|--------|-------------|---------|
| `IMU_` | IMU/Gyro | `IMU_Base`, `IMU_Arm` |
| `Encoder_` | Wheel Encoder | `Encoder_LeftWheel` |
| `LiDAR_` | LiDAR | `LiDAR_Front`, `LiDAR_360` |
| `Camera_` | RGB Camera | `Camera_Main` |
| `Depth_` | Depth Camera | `Depth_Front` |
| `RealSense_` | RealSense D435 | `RealSense_Front` |
| `GPS_` | GPS Module | `GPS_Antenna` |

### Wheel Identification

Wheels should contain "Wheel" in name:
- `LeftWheel`, `RightWheel`
- `FrontLeftWheel`, `FrontRightWheel`
- `Wheel_FL`, `Wheel_FR`, `Wheel_RL`, `Wheel_RR`

### Special Components

| Name | Purpose |
|------|---------|
| `Base` | Main body (required) |
| `*Wheel*` | Wheel (auto-detected) |
| `*Caster*` | Caster wheel (passive) |
| `*Mount` | Mounting point (not physical) |

## Sample Robot CAD

### Differential Drive Robot

**Components:**
1. **Base** (300mm x 250mm x 150mm)
   - Material: ABS Plastic
   - Mass: ~5kg

2. **LeftWheel** (Ø100mm x 30mm)
   - Material: Rubber
   - Position: [0, 150, 0]mm

3. **RightWheel** (Ø100mm x 30mm)
   - Material: Rubber
   - Position: [0, -150, 0]mm

4. **CasterWheel_Front** (Ø50mm)
   - Material: Plastic
   - Position: [100, 0, -50]mm

5. **IMU_Base** (20mm x 20mm x 5mm)
   - Material: PCB (custom)
   - Position: [0, 0, 50]mm

**Joints:**
- LeftWheel: Revolute joint, axis: Y
- RightWheel: Revolute joint, axis: Y
- CasterWheel: Free joint (passive)

### Ackermann Steering Robot

**Components:**
1. **Chassis**
2. **FrontLeftWheel** + **FrontLeft_Steering**
3. **FrontRightWheel** + **FrontRight_Steering**
4. **RearLeftWheel**
5. **RearRightWheel**
6. **LiDAR_Front**

## Export Checklist

Before exporting:

- [ ] All components properly named
- [ ] Physical materials assigned
- [ ] Mass properties calculated (Analysis → Mass Properties)
- [ ] Sensor locations marked with components
- [ ] Joint axes defined correctly
- [ ] Origin at appropriate location
- [ ] Units are consistent (meters recommended)
- [ ] Coordinate system: X-forward, Y-left, Z-up
- [ ] No duplicate or overlapping geometry

## Common Issues

### Issue: Wrong Scale

**Problem:** Robot appears too large/small in simulation

**Solution:**
- Check export units (should be meters)
- Verify scale factor in config
- Use consistent units throughout

### Issue: Missing Mass

**Problem:** Robot has no physics

**Solution:**
- Assign physical materials in Fusion360
- Check mass properties (Analysis → Mass Properties)
- Ensure mass is exported to USD (Phase 2.2+)

### Issue: Sensors Not Detected

**Problem:** Plugin doesn't find sensors

**Solution:**
- Check component naming (use exact prefixes)
- Ensure components are at top level (not nested deep)
- Verify plugin version

### Issue: Joint Not Working

**Problem:** Wheels don't rotate

**Solution:**
- Verify joint type (should be Revolute for wheels)
- Check joint axis direction
- Ensure joint limits are set correctly

## Tips & Tricks

### Performance Optimization

**Collision Geometry:**
- Create separate "collision" bodies with simplified geometry
- Use cylinders/boxes instead of complex meshes where possible
- Name collision bodies with `_collision` suffix

**Visual Geometry:**
- Keep visual detail high for aesthetics
- Plugin will separate visual vs collision automatically

### Testing in Fusion360

Before export:
1. Run Motion Study to verify joints
2. Check interference detection
3. Validate mass properties
4. Review component hierarchy

### Version Control

Recommended structure:
```
my_robot/
├── fusion360/
│   └── robot.f3d          # Fusion360 archive
├── exported/
│   ├── robot.usd          # USD export
│   ├── robot_config.yaml  # Generated config
│   └── meshes/            # Mesh files
└── configs/
    └── custom.yaml        # Customized config
```

Commit to git:
- ✅ robot.usd
- ✅ *.yaml configs
- ✅ Simplified .stl exports
- ❌ robot.f3d (too large, binary)

## Resources

- [Fusion360 Documentation](https://www.autodesk.com/products/fusion-360/documentation)
- [USD Format Specification](https://graphics.pixar.com/usd/docs/index.html)
- [LARCS Plugin GitHub](https://github.com/dev-Earth/LARCS_Fusion360_Plugin) (coming soon)

## Example Robots

Sample CAD files available:
- Simple 2-wheel differential drive
- 4-wheel Ackermann steering
- Mecanum wheel platform
- 6-wheel rocker-bogie

Download: [LARCS Example Robots](https://github.com/dev-Earth/LARCS_Examples)

## Getting Help

- Open issue on GitHub
- Check Fusion360 forum
- Join LARCS Discord (coming soon)
