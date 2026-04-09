# Industry 4.0 – Kanban Production Simulator

A VR-enabled industrial facility simulator built in **Unreal Engine 5.5** for the DHBW TIF23A semester project. Players can monitor and control a simulated production line through interactable screens and a 6-camera CCTV system, using either VR (OpenXR) or desktop mode.

---

## Project Structure

```
Content/
├── Blueprints/
│   ├── CCTV/                        # Security camera system
│   ├── InteractableScreens/         # Monitor & Tablet UI actors
│   │   └── Monitor/Screens/         # Individual screen widgets
│   ├── Production/                  # Equipment actors & data
│   └── Lights/                      # Day/Night cycle
├── Materials/                       # Custom materials
├── silo/                            # Custom silo 3D model
├── WIS/                             # Showroom map assets
├── __ExternalActors__/              # UE5 One-File-Per-Actor level data
└── __ExternalObjects__/             # UE5 level object data
Plugins/
└── LightingTool/                    # Ultimate Lighting Tool (Leartes Studios)
```

---

## How It Works

### Central Data Manager — `BP_FacilityDataManager`

Everything flows through this single actor placed in the level. It holds the global state for the entire facility:

- Active **orders** and **recipes** (using `STR_Order` and `STR_Recipe` data structures)
- Current **equipment states** (`FSTR_PumpState`, `FSTR_ValveState`)
- **Fluid levels** across silos and the reactor

All UI widgets and equipment blueprints communicate with `BP_FacilityDataManager` rather than with each other directly, keeping systems decoupled.

---

### Production Equipment

Each piece of equipment is a self-contained Blueprint actor:

| Blueprint | Role |
|---|---|
| `BP_Pump` | Controls material flow between containers |
| `BP_Reactor` | Processes materials according to a recipe |
| `BP_Silo` | Stores raw or finished materials |
| `BP_Valve` | Opens/closes flow paths between equipment |
| `BP_Emergency_Stop` | Halts all production and resets states |

Pipes between equipment are visual-only (`BP_PipeVisualOnly`, `Bp_pipe_straight_Reactor_Silo`, etc.) and carry no logic — flow state is managed by the valves and pumps. Fluid levels are visualized using Niagara particle effects (`NS_FluidLevel`).

---

### Interactable Screens

Players interact with the facility through two physical actors in the level:

**`BP_InteractableMonitor`** — A wall-mounted monitor with a multi-screen UI:

| Widget | Purpose |
|---|---|
| `WBP_Home` | Dashboard overview |
| `WBP_FluidLevels` | Live silo and reactor level display |
| `WBP_FlowSystem` | Visual pipe/flow state diagram |
| `WBP_OrderUI` / `WBP_OrderUIDetail` | Browse and manage production orders |
| `WBP_RecipeCreation` / `WBP_RecipeEntry` | Create and edit recipes |
| `WBP_RecipeOrder` | Submit an order for a recipe |
| `WBP_ESResetFlushInv` | Emergency stop controls and inventory reset |

**`BP_Tablet`** — A handheld tablet with a header navigation widget (`WBP_Header`), providing portable access to key screens.

Both actors implement `InteractionInterface`, the common interface that allows VR hand tracking and desktop click events to trigger the same interaction logic.

---

### CCTV System

Six camera actors (`CCTV_CAM_1` through `CCTV_CAM_6`) feed into `BP_CCTV`, which renders their views to screen materials displayed on monitors in the facility. Each camera has its own render target material (`CCTV_CAM_*_Mat`).

---

### VR & Interaction

The project starts in VR by default (`bStartInVR=True` in `DefaultGame.ini`). Input is handled via:

- **OpenXR** for headset and controller tracking
- **OpenXRHandTracking** for hand-based interaction
- **OpenXREyeTracker** for gaze input
- **NS_VRBeam** — a Niagara laser pointer effect for pointing at screens

To run in desktop mode, launch with `-nohmd` or disable `bStartInVR` in `Config/DefaultGame.ini`.

---

### Lighting

**`BP_DayNightLamps`** drives the day/night lighting cycle. The **LightingTool** plugin (Ultimate Lighting Tool v1.2.1 by Leartes Studios) provides the editor tooling for HDRI-based environment lighting and IES light profiles used across the facility.

---

## Levels

| Map | Purpose |
|---|---|
| `Main.umap` | Primary production facility — the main playable level |
| `NpcWelt.umap` | NPC environment for future training scenarios |
| `Laboratory.umap` | Lab environment |
| `Playground.umap` | Developer sandbox for testing |
| `Setup1.umap` | Initial configuration scene |

---

## Setup

1. Install **Unreal Engine 5.5** via the Epic Games Launcher
2. Install marketplace content packs via the Launcher (these are not tracked in git):
   - Factory Exterior/Interior Bundle
   - Leartes Studios Office
   - Vigilante Content, Corridor Assets, Elevator Corridor
   - Quantum Character, Laboratory assets
   - Fab assets, UE Starter Content, VR Template
3. Open `Industry_4.uproject` — UE will prompt to compile the LightingTool plugin
4. Open `Content/Main.umap` and press Play

---

## Documentation

Full project documentation is in `/Documentation/`:
- `DHBW_TIF23A_Industrie4.0.pdf` — main technical documentation
- `Gruppe D - Industrie 4.0 Dokumentation.pdf` — semester deliverable
- `UML.png` — system architecture diagram
