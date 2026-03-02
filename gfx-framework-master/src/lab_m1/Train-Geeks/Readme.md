# Train Geeks - Mini Metro

**Implementation language:** OpenGL + C++  
**Framework:** gfx-framework (Visual Studio)

---

## Overview

This project implements the **Mini Metro** mini-game variant from the Train Geeks assignment. A train moves autonomously along a fixed circular route, stopping at three themed stations to collect passenger symbols and deliver them back to the main station within a time limit.

---

## Game Description

The map contains **three resource stations**, each identified by a unique geometric shape:
- **Cube Station** - passengers represented as small cubes
- **Pyramid Station** - passengers represented as small pyramids
- **Sphere Station** - passengers represented as small spheres

At the start of each round, a set of **passenger symbols** is placed above the main station. Each symbol's shape indicates which station it needs to travel from. The train visits resource stations, collects the required passengers one by one (with a brief stop per passenger), then returns to the main station to deliver them.

Each successful delivery increases the number of symbols in the next order. The symbols above the main station **gradually shift toward red** in the shader as the delivery timer counts down. If the order is not completed within **30 seconds**, the game ends.

---

## Train

The locomotive and wagon meshes are **built procedurally** from 3D primitives (parallelepipeds and cylinders):

- **Locomotive** - engine body (horizontal cylinder), driver cabin (vertical parallelepiped), multiple cylindrical wheels
- **Wagon** - rectangular body on four cylindrical wheels

The train follows a linked-list of `Rail` segments. For each segment the train linearly interpolates its position:

```cpp
train.progress += deltaTime * train.speed;
if (train.progress >= 1.f) {
    train.progress = 0.f;
    train.rail = nextRail(train.rail);
}
train.position = lerp(train.rail->startPosition, train.rail->endPosition, train.progress);
```

The train's orientation (`yaw`) is derived from the current rail direction vector.

---

## Environment

### Terrain

The map is composed of 2D colored quads placed side by side to form different terrain zones:

| Color  | Terrain  | Rail Type |
|--------|----------|-----------|
| Green  | Field    | Normal    |
| Blue   | Water    | Bridge    |
| Brown  | Mountain | Tunnel    |

Additional decorative elements (trees) are scattered on the field terrain.

### Rail Types

Three visually distinct rail variants are implemented:

- **Normal rail** - dark parallel bars on a wooden sleeper base
- **Bridge rail** - rail elevated on alternating-color pillars
- **Tunnel rail** - rail enclosed within transversal tunnel arch segments

All three types also have **intersection** variants (T-junctions, L-turns) that correctly connect adjacent segments in any of the four cardinal directions.

### Stations

- **Main station** - the delivery hub, rendered as a more complex building mesh with a platform, walls, and a distinctive roof
- **Cube / Pyramid / Sphere stations** - each rendered as a small station building whose identifying symbol is embedded in the structure

---

## Collision / Proximity Detection

Station detection uses a **radius-based proximity check**: when the distance between the train's position and a station center drops below `kStationRadius = 4.0f` units, the train stops and begins collecting/delivering symbols.  
For the intersection stopping logic, a `kStopOffset` of 1.5 units keeps the train clear of the junction center.

---

## Camera

Two camera modes are supported, toggled at runtime:

- **Free camera** - WASD + mouse look, full 3D navigation
- **Centered / TPS camera** - locked at a fixed offset behind and above the locomotive, following it automatically

Both modes use a **perspective projection** (60 deg FoV).

---

## Minimap

After the main scene is rendered, the **depth buffer is cleared** and the scene is re-rendered from above using an **orthographic camera**, placed in a corner of the screen. Key elements are rendered with simplified 2D icons:

- Train - arrow mesh
- Stations - star mesh
- Rails - grey stripe mesh

---

## HUD & End Screen

- An on-screen **elapsed time** counter is rendered using `TextRenderer` (Hack-Bold font).
- The delivery timer is visualised via the **shader-driven color shift** of passenger symbols above the main station (base color to red).
- A full-screen **end game overlay** is shown when the delivery timer expires.

---

## Controls

| Key / Action | Effect |
|---|---|
| `W / A / S / D` | Move free camera |
| Mouse drag | Rotate free camera |
| `C` | Toggle centered / free camera |
| Scroll wheel | Zoom (free camera) |

---
