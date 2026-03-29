# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

The build directory is `build/` at the repo root. An existing configured build is already there — use it directly.

```bash
# Build everything
cd build && make -j$(nproc)

# Build only the standalone executable
cd build && make -j$(nproc) smoldyn

# Build only the Python module
cd build && make -j$(nproc) _smoldyn

# Reconfigure from scratch (rarely needed)
cmake -S . -B build -DOPTION_PYTHON=ON -DOPTION_BROWSER_UI=ON
```

Key CMake options (most are ON by default):
- `OPTION_BROWSER_UI` — embeds Mongoose HTTP server for browser-based UI (the active development area)
- `OPTION_PYTHON` — builds the pybind11 Python module
- `OPTION_NSV` — NextSubvolume alternative simulation algorithm
- `OPTION_USE_OPENGL` — OpenGL real-time visualization
- `OPTION_ASAN` — AddressSanitizer (off by default)

## Tests

Tests run via CTest from the build directory. The test env requires `PYTHONPATH` pointing at `build/py/`.

```bash
# Run all tests
cd build && ctest --output-on-failure -j2

# Run a single named test
cd build && ctest -R test_api -V

# Run a Python test file directly (faster iteration)
cd build && PYTHONPATH=./py SMOLDYN_NO_PROMPT=1 python3 ../tests/test_api.py

# List all test names
cd build && ctest -N
```

## Python linting / formatting

```bash
cd build
make lint   # flake8 + mypy (strict)
make fmt    # ruff format
```

## Architecture

Smoldyn is a **particle-based stochastic biochemical simulator**. The central data structure is `simptr` (a pointer to `struct simstruct` defined in `source/Smoldyn/smoldyn.h`), which holds the entire simulation state: molecules, walls, surfaces, reactions, compartments, and graphics config.

### Layers

```
CLI (smoldyn.cpp)  /  Python API (source/python/smoldyn/smoldyn.py)
         ↓
LibSmoldyn public API  (source/Smoldyn/libsmoldyn.cpp / libsmoldyn.h)
         ↓
Core simulation loop  (source/Smoldyn/smolsim.cpp)
    ├── Molecules        smolmolec.c   — species, diffusion, states
    ├── Reactions        smolreact.c   — 0th/1st/2nd order, spatial rates
    ├── Surfaces         smolsurface.c — panels, binding, reflection
    ├── Commands         smolcmd.c     — config-file commands & runtime cmds
    ├── Spatial index    smolboxes.c   — domain decomposition grid
    └── Graphics         smolgraphics.c — OpenGL render + browser SVG
         ↓
Math/utility library  (source/libSteve/)
    — Rn.c (vectors), Geometry.c, random2.c (SFMT RNG), rxnparam.c
```

**Optional subsystems:**
- `source/NextSubVolume/` — stochastic compartment method (NSV), uses Eigen + Boost
- `source/web-server/mongoose.c` — vendored Mongoose embedded HTTP server
- `source/Smoldyn/smolbng.c` — BioNetGen rule-based reaction network integration
- `source/Smoldyn/smolfilament.c` — filament/polymer support

### Browser UI (`OPTION_BROWSER_UI`)

Defined in `source/Smoldyn/server.hh` (included only in `smoldyn.cpp`):
- Starts a Mongoose HTTP server on **port 3141** in a background thread
- Simulation waits for first browser connection before starting
- `GET /` — serves an HTML page (HTMX, polls `/svg` every 50 ms)
- `GET /svg` — calls `simptr_to_svg()` which renders walls + molecules as SVG

`simptr_to_svg()` maps simulation coordinates to SVG pixels with a uniform scale preserving aspect ratio; molecule positions come from `sim->mols->live[ll][m]->pos`, colors from `sim->mols->color[i][ms]`, sizes from `sim->mols->display[i][ms]`.

### Python bindings

`source/python/module.cpp` is the pybind11 entry point; it exposes the C API from `libsmoldyn.h`. The high-level Pythonic API lives entirely in `source/python/smoldyn/smoldyn.py` (the `Simulation`, `Species`, `Surface`, `Reaction`, etc. classes). The compiled extension is `build/py/smoldyn/_smoldyn.*.so`.

### Key data structures (smoldyn.h)

| Struct | ptr typedef | What it holds |
|---|---|---|
| `simstruct` | `simptr` | entire simulation state |
| `molsuperstruct` | `molssptr` | all species, live/dead lists, colors, display sizes |
| `moleculestruct` | `moleculeptr` | single molecule: `pos[]`, `ident` (species), `mstate` |
| `wallstruct` | `wallptr` | boundary wall: `pos`, `wdim`, `side` |
| `rxnsuperstruct` | `rxnssptr` | reaction tables for each order (0/1/2) |
| `graphicssuperstructure` | `graphssptr` | OpenGL/display parameters |

`sim->wlist` is indexed as: `[0]`=xmin, `[1]`=xmax, `[2]`=ymin, `[3]`=ymax, `[4]`=zmin, `[5]`=zmax.
`sim->mols->listtype[ll] == MLTsystem` guards molecule iteration (skip port lists).
