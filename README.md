# Industry 4.0 – VR Production Facility Simulator

A VR simulation of an industrial chemical production facility built in **Unreal Engine 5.5** as a DHBW TIF23A semester project. Users wear a VR headset and physically walk around a virtual factory, operate pumps and valves, manage production orders, monitor equipment via screens, and watch the facility through a 6-camera CCTV system.

---

## Table of Contents

1. [What This Project Is](#what-this-project-is)
2. [What the Factory Simulates](#what-the-factory-simulates)
3. [How to Set It Up](#how-to-set-it-up)
4. [How to Run It](#how-to-run-it)
5. [How the Production System Works](#how-the-production-system-works)
6. [What You Can Do in VR](#what-you-can-do-in-vr)
7. [Blueprint Architecture](#blueprint-architecture)
8. [Levels](#levels)
9. [The LightingTool Plugin](#the-lightingtool-plugin)
10. [Project File Structure](#project-file-structure)
11. [Configuration Reference](#configuration-reference)

---

## What This Project Is

This is an **Unreal Engine 5.5 VR application**. There is no traditional source code — all logic is written in **Blueprints**, which are UE5's visual scripting system. When this document refers to "code", it means Blueprint assets (`.uasset` files) inside the `Content/Blueprints/` folder.

The project starts in **VR mode by default**. A supported OpenXR headset (Meta Quest, HTC Vive, Valve Index, or Windows Mixed Reality) is required to use it as intended. It can also be tested without a headset using keyboard and mouse (see [How to Run It](#how-to-run-it)).

---

## What the Factory Simulates

The simulation represents a **batch chemical production facility** — the kind used in industries like food processing, pharmaceuticals, or materials manufacturing. The factory has real industrial components:

```
Raw Material Storage (Silo)
         │
         ▼  [Pump moves material]
   Reactor Vessel  ◄── Recipe defines how to process it
         │
         ▼  [Valves control the output path]
     Output / Completion
```

### Physical Equipment in the Factory

| Equipment | Blueprint | What It Does |
|---|---|---|
| **Silo** | `BP_Silo` | Stores raw materials. Has a fluid level that drops as material is consumed. |
| **Reactor** | `BP_Reactor` | The main processing vessel where the recipe is carried out. Tracks its own fluid level and state. |
| **Pump** | `BP_Pump` | Moves fluid from the Silo into the Reactor. Can be turned on/off. |
| **Valve** | `BP_Valve` | Controls which path fluid flows through. Can be opened or closed. |
| **Pipes** | `BP_PipeVisualOnly` | Visual-only connections between equipment. Carry no logic — the pump and valves control actual flow. |
| **Emergency Stop** | `BP_Emergency_Stop` | A large red button that immediately halts all production and resets equipment states. |

Fluid levels are visualised using **Niagara particle effects** (`NS_FluidLevel`) — liquid visually rises and falls in the Silo and Reactor as production runs.

---

## How to Set It Up

> **Important:** Cloning this repo alone is not enough to run the project. The 3D environments and character assets are not stored in git (they are large binary files, some over 100 MB). You must install them separately from the Epic Games Launcher and Fab before opening the project. Follow every step below or the levels will be full of missing/pink assets.

### Step 1 — Install Unreal Engine 5.5

Download and install the **Epic Games Launcher**, then install **Unreal Engine 5.5** from the Library tab.

### Step 2 — Clone the Repository

```bash
git clone https://github.com/akoSiThaesler/Temp3.git
```

### Step 3 — Install Required Asset Packs

All of the packs below are **free**. For each one:
1. Find it in the Epic Games Launcher under **Unreal Engine → Fab** or **Library → Vault**
2. Click **Add to Project**
3. Select this project (`Industry_4`) and engine version **5.5**

#### UE Built-in Content Packs
These come with the engine — add them via **Edit → Plugins** or by adding a template:

| Pack Name | Installs into | Why it's needed |
|---|---|---|
| Starter Content | `Content/StarterContent/` | Basic materials and meshes used across levels |
| VR Template | `Content/VRTemplate/` | Core VR pawn, locomotion, and interaction framework |
| First Person Template | `Content/FirstPerson/` + `Content/FirstPersonArms/` | Player character and arm animations |
| Level Prototyping | `Content/LevelPrototyping/` | Modular geometry used in some areas |
| MannequinsXR | `Content/Characters/` | VR-compatible player mannequin |

To add these: open the project, go to **Edit → Add Feature or Content Pack**, and add **Virtual Reality** and **First Person** template content.

#### Marketplace / Fab Packs
Search for each by the exact name on **fab.com** or in the Epic Games Launcher Fab tab:

| Pack Name (search exactly this) | Installs into | Used for |
|---|---|---|
| `Factory Exterior and Interior` | `Content/FactoryExteriorInteriorBundle/` | The factory building shell — walls, floors, ceiling |
| `Leartes Studios - Office Environment` | `Content/LeartesStudiosOffice/` | Office/control room area of the factory |
| `Modular Corridor` (or Corridor Assets) | `Content/CorridorAssets/` | Hallways connecting areas |
| `Elevator and Corridor` | `Content/Elevator_corridor/` | Elevator area |
| `Vigilante - Military Vehicles` | `Content/VigilanteContent/` | Exterior vehicle props |
| `Quantum Character` | `Content/QuantumCharacter/` | NPC character model |
| `Laboratory` | `Content/Laboratory/` | Laboratory level environment |

#### Fab Free Assets (individual assets, not packs)
These are individual free assets from fab.com. Search each name:

| Asset Name | Installs into | Used for |
|---|---|---|
| `AC Motor` | `Content/Fab/AC_Motor/` | Pump motor 3D model |
| `Emergency Stop Button` | `Content/Fab/Emergency_Stop_Button/` | E-Stop button model |
| `Iconic Half-Life Red Valve` | `Content/Fab/Iconic_Half-Life_red_valve/` | Valve 3D model |
| `Machinery Device` | `Content/Fab/Machinery_device/` | Industrial machinery props |
| `Monitor` | `Content/Fab/Monitor/` | Wall monitor screen model |
| `Poco Phone` | `Content/Fab/Poco_Phone/` | Handheld tablet model |
| `Ultrawide Monitor` | `Content/Fab/Ultrawide_Monitor/` | Secondary monitor model |
| Megascans surfaces | `Content/Fab/Megascans/` | Floor and wall surface materials |

### Step 4 — Open the Project

1. Double-click `Industry_4.uproject`
2. Unreal Engine will ask to compile the **LightingTool** plugin — click **Yes**
3. Wait for shaders to compile (this can take 10–30 minutes on first open)
4. Open `Content/Main.umap`

### Step 5 — Verify Everything Loaded

In the editor, check the **Output Log** (Window → Output Log). If you see errors like `Asset not found` or references to any of the `Content/Fab/` or `Content/FactoryExterior...` paths, that asset pack was not installed correctly. Re-add it via the Epic Launcher and restart the editor.

---

## How to Run It

### In VR (Default)

Connect your VR headset, then press **Play** in the editor. The game launches directly into VR. The headset must be active before pressing Play.

### Testing Without a Headset

The game defaults to VR mode. To disable it:

1. Open `Config/DefaultGame.ini`
2. Find `bStartInVR=True` and change it to `bStartInVR=False`
3. Press Play — the game runs in a regular window

**Keyboard controls for desktop mode:**

| Key | Action |
|---|---|
| `W A S D` | Move around |
| `Q / E` | Move up / down |
| `Mouse` | Look around |
| `E` | Interact with objects (replaces VR trigger) |
| `Tab` | Toggle spectator camera |

---

## How the Production System Works

### The Three Data Structures

The production system is built around three data types defined as Blueprint structs:

**`STR_Recipe`** — a formula for making a product. Defines ingredients, ratios, processing parameters, duration, and temperature.

**`STR_Order`** — a customer request. Contains what product is needed, how much, and a deadline.

**`FSTR_PumpState` / `FSTR_ValveState`** — live equipment states. Track whether each pump is running and whether each valve is open or closed.

### The Central Brain — `BP_FacilityDataManager`

This is a single Blueprint actor placed in the Main level that acts as the **central data store for the entire simulation**. Every other system reads from and writes to it:

- Stores the full recipe database
- Tracks all active orders
- Holds the current state of every pump and valve
- Records production history

Nothing talks to anything else directly. The monitor UI reads orders from `BP_FacilityDataManager`. The equipment actors report their state back to it. This keeps all systems decoupled.

### A Full Production Run — Step by Step

```
1. Operator approaches the wall monitor (BP_InteractableMonitor)
         │
2. Opens the Orders screen (WBP_OrderUI)
   → Sees a list of pending orders pulled from BP_FacilityDataManager
         │
3. Selects an order → opens WBP_OrderUIDetail
   → Views what product is needed and in what quantity
         │
4. Navigates to Recipes (WBP_RecipeEntry / WBP_RecipeOrder)
   → Selects or creates a recipe that matches the order
         │
5. Opens the Flow System screen (WBP_FlowSystem)
   → Opens the correct valves (BP_Valve) for this production run
   → Starts the pump (BP_Pump) to move material from Silo to Reactor
         │
6. Opens the Fluid Levels screen (WBP_FluidLevels)
   → Watches the Silo level drop and Reactor level rise in real time
         │
7. Reactor processes the material (BP_Reactor runs its logic)
         │
8. Production completes
   → Operator uses the CCTV screens to visually confirm equipment is idle
   → Order is marked complete in BP_FacilityDataManager
         │
9. If anything goes wrong at any step:
   → Operator hits the Emergency Stop button (BP_Emergency_Stop)
   → All pumps stop, all valves reset
```

---

## What You Can Do in VR

### VR Controller Mapping

| Button | Action |
|---|---|
| **Grip (both hands)** | Grab and hold objects |
| **Right Trigger** | Interact / select |
| **Left Trigger** | Secondary interact |
| **Right Menu Button** | Toggle right UI panel |
| **Left Menu Button** | Toggle left UI panel |
| **Thumbstick** | Move through the environment |

A **VR laser pointer beam** (`NS_VRBeam` Niagara effect) extends from your controller so you can accurately point at screens and buttons from a distance.

### Interactable Objects

#### The Wall Monitor (`BP_InteractableMonitor`)

The primary control interface. A large screen on the factory wall that the operator points at and activates. It has **10 switchable screens**:

| Screen Widget | What It Shows / Does |
|---|---|
| `WBP_Home` | Dashboard — overall facility status at a glance |
| `WBP_OrderUI` | List of all active orders |
| `WBP_OrderUIDetail` | Full details of one selected order |
| `WBP_RecipeEntry` | Browse the recipe database |
| `WBP_RecipeEntryDetail` | View full parameters of one recipe |
| `WBP_RecipeCreation` | Create a brand new recipe |
| `WBP_RecipeOrder` | Assign a recipe to an order |
| `WBP_FluidLevels` | Live read-out of fluid levels in Silo and Reactor |
| `WBP_FlowSystem` | Open/close valves, start/stop pumps |
| `WBP_ESResetFlushInv` | Emergency stop controls and inventory flush/reset |

Navigation between screens uses 6 tab buttons along the top of the monitor (Tab1–Tab6 UI assets).

#### The Handheld Tablet (`BP_Tablet`)

A smaller portable device that can be picked up and carried around the factory. Displays a status header (`WBP_Header`) with key information. Useful for monitoring while walking around equipment.

#### Physical Equipment

All production equipment can be interacted with directly in VR:
- **Pumps** — show visual feedback (spinning, sound) when running
- **Valves** — physically open and close
- **Emergency Stop** — a large red button mounted on the factory floor

All interactable objects implement **`InteractionInterface`**, a shared Blueprint interface that standardises how VR hands and the keyboard `E` key trigger the same underlying logic.

#### CCTV Monitors (`BP_CCTV`)

Six cameras (`CCTV_CAM_1` through `CCTV_CAM_6`) are positioned around the factory. Their feeds are rendered to screen materials on monitors throughout the facility, giving the operator a bird's-eye view of multiple areas simultaneously.

---

## Blueprint Architecture

All custom logic lives in `Content/Blueprints/`. Here is a full breakdown:

### Production (`Content/Blueprints/Production/`)

The core simulation logic:

```
BP_FacilityDataManager  ← Central data hub (singleton actor in Main level)
         │
         ├── BP_Silo          (raw material storage + fluid level)
         ├── BP_Reactor       (processing vessel + fluid level)
         ├── BP_Pump          (fluid transfer, uses FSTR_PumpState)
         ├── BP_Valve         (flow control, uses FSTR_ValveState)
         ├── BP_Emergency_Stop (kills everything)
         │
         ├── STR_Order        (data structure: order definition)
         ├── STR_Recipe       (data structure: recipe definition)
         ├── FSTR_PumpState   (data structure: pump on/off state)
         └── FSTR_ValveState  (data structure: valve open/closed state)

Visual effects:
         ├── NS_FluidLevel    (Niagara: liquid in Silo/Reactor)
         └── NS_VRBeam        (Niagara: VR controller laser pointer)

Pipes (visual only, no logic):
         ├── BP_PipeVisualOnly
         ├── Bp_pipe_straight_Reactor_Silo
         └── Bp_pipe_straight_Silo_Reactor
```

### Interactable Screens (`Content/Blueprints/InteractableScreens/`)

```
BP_InteractableMonitor  ← Wall-mounted interactive actor
└── WBP_InteractableMonitorDisplay  ← Container widget
    ├── WBP_Home
    ├── WBP_OrderUI
    ├── WBP_OrderUIDetail
    ├── WBP_RecipeEntry
    ├── WBP_RecipeEntryDetail
    ├── WBP_RecipeCreation
    ├── WBP_RecipeOrder
    ├── WBP_FluidLevels
    ├── WBP_FlowSystem
    └── WBP_ESResetFlushInv

BP_Tablet  ← Handheld device
└── WBP_Header  ← Status bar widget

InteractionInterface  ← Shared interface implemented by all interactable actors
```

UI icon assets (stored in `MonitorImg/`): pump, reactor, silo, valve open/closed, tab buttons, primary/secondary button states.

### CCTV (`Content/Blueprints/CCTV/`)

```
BP_CCTV  ← Camera controller
├── CCTV_CAM_1  →  CCTV_CAM_1_Mat  (renders to screen material)
├── CCTV_CAM_2  →  CCTV_CAM_2_Mat
├── CCTV_CAM_3  →  CCTV_CAM_3_Mat
├── CCTV_CAM_4  →  CCTV_CAM_4_Mat
├── CCTV_CAM_5  →  CCTV_CAM_5_Mat
└── CCTV_CAM_6  →  CCTV_CAM_6_Mat
```

### Lighting (`Content/Blueprints/Lights/`)

```
BP_DayNightLamps  ← Toggles ambient lighting between day and night states
```

---

## Levels

| Map File | Actors | Purpose |
|---|---|---|
| `Content/Main.umap` | 8,047 | **The main game** — fully built factory with all equipment, monitors, CCTV, and props. This is what users experience. |
| `Content/Setup1.umap` | 700 | Simplified version for onboarding or tutorial use |
| `Content/Laboratory.umap` | 488 | Laboratory environment, likely for recipe testing scenarios |
| `Content/NpcWelt.umap` | 139 | World with NPC positions — staff/worker simulation |
| `Content/Playground.umap` | 44 | Minimal developer sandbox for testing individual features |

> **Why are there 8,047 actors in Main?** UE5 uses a system called **One File Per Actor** where every object placed in a level (each prop, light, piece of equipment, wall, floor tile) is stored as its own small file in `Content/__ExternalActors__/`. This is normal and enables better teamwork — two people can place objects without git conflicts. Those 8,047 files are the entire contents of the factory floor.

---

## The LightingTool Plugin

Located in `Plugins/LightingTool/`, this is the **Ultimate Lighting Tool v1.2.1** by Leartes Studios. It's a third-party editor plugin with three modules:

### LightingTool (Editor Module)
A lighting design tool used when building the levels. It provides a UI panel inside the UE editor to:
- Manage lists of lights across the scene
- Adjust light parameters in bulk
- Preview day/night lighting configurations
- Visualise light sensor data

### LightingGame (Runtime Module)
Runs at game time. Contains **`LTLightSensor`** — an actor that measures how bright a specific point in the scene is. Used to make gameplay respond to lighting conditions (e.g., detecting if an area is too dark).

### CameraManager (Editor + Runtime Module)
Manages camera presets and views. Provides:
- Saved camera positions and settings (`CMCameraPresetData`)
- A UI for switching between preset camera angles
- Cinematics and fly-through support
- Camera preset import/export

This module drives the CCTV camera system and any pre-authored camera shots in the level.

---

## Project File Structure

```
Industry_4.uproject           ← Open this to launch the project in UE5
│
├── Config/
│   ├── DefaultGame.ini       ← VR startup, game mode, compression settings
│   ├── DefaultEngine.ini     ← Rendering (ray tracing, Nanite), streaming, VR
│   ├── DefaultInput.ini      ← All VR controller and keyboard bindings
│   └── DefaultEditorSettings.ini
│
├── Content/
│   ├── Blueprints/           ← ALL custom game logic (64 blueprints)
│   │   ├── Production/       ← Equipment actors and data structures
│   │   ├── InteractableScreens/ ← Monitor, tablet, and all UI widgets
│   │   ├── CCTV/            ← 6-camera security system
│   │   └── Lights/          ← Day/night lighting controller
│   ├── Materials/            ← Custom shared materials (glass, monitor screen)
│   ├── silo/                 ← Custom 3D silo model (FBX + textures)
│   ├── WIS/                  ← Showroom map assets
│   ├── __ExternalActors__/   ← Level actor data (UE5 One-File-Per-Actor)
│   └── __ExternalObjects__/  ← Level object data
│
├── Plugins/
│   └── LightingTool/         ← Ultimate Lighting Tool plugin (C++ source included)
│
└── Documentation/
    ├── DHBW_TIF23A_Industrie4.0.pdf     ← Full project documentation (German)
    ├── Gruppe D - Industrie 4.0 Dokumentation.pdf  ← Semester deliverable
    ├── UML.png                           ← System architecture diagram
    └── UML-Info.docx                     ← Class structure notes
```

---

## Configuration Reference

### `Config/DefaultGame.ini` — Key Settings

| Setting | Value | Meaning |
|---|---|---|
| `GameDefaultMap` | `/Game/Main` | Opens Main.umap on launch |
| `bStartInVR` | `True` | Auto-launches in VR mode |
| `GameModeClassAlias` | `BP_FirstPersonGameMode` | Uses first-person VR character |

### `Config/DefaultEngine.ini` — Key Settings

| Setting | Value | Meaning |
|---|---|---|
| `r.RayTracing` | Enabled | High-quality reflections and shadows |
| `r.Nanite` | Enabled | High-polygon mesh rendering for detailed assets |
| `r.TextureStreaming` | Enabled | Loads textures on demand (saves VRAM) |
| `TextureStreamingPoolSize` | `1500` | MB reserved for streaming textures |

### `Config/DefaultInput.ini` — VR Platforms Supported

- Meta Quest (OculusTouch controllers)
- HTC Vive
- Valve Index
- Windows Mixed Reality
