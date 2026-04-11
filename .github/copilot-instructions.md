# RTspice Copilot Instructions

## Build and test commands

Use an out-of-source build:

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"
```

Run registered tests:

```bash
cd build
ctest --output-on-failure
```

Run a single registered test target:

```bash
cd build
ctest -R circuit_test --output-on-failure
```

Parser tests are built as executables but are not registered with `ctest`; run them directly:

```bash
./build/lib/parser/test/line_parser_test
./build/lib/parser/test/component_parser_test
```

No lint command is currently configured in this repository.

## High-level architecture

RTspice is a real-time audio circuit simulator with four main layers:

- `lib/parser`: Boost.Spirit grammars parse a netlist into `components::component::ptr`.
- `lib/components`: component models stamp MNA entries through a shared component interface.
- `lib/circuit`: owns the sparse system, partitions components by behavior (static/dynamic/nonlinear), and solves with CUDA cuSPARSE/cuSOLVER.
- `lib/gui`: JUCE-based VST3 plugin runtime. The audio processor loads netlists, constructs `circuit::circuit`, and advances simulation from `processBlock`.

Execution flow is: netlist file -> `line_parser` (statements) -> `statement_parser` (components) -> `circuit` construction -> plugin `processBlock` drives simulation per audio sample.

## Key repository conventions

- Component lifecycle is strict: implement `is_static`/`is_dynamic`/`is_nonlinear`, then `register_`, `setup`, and `fill() const noexcept`.
- `fill()` is on the hot path (called in audio processing and Newton iterations). Keep it allocation-free and lightweight.
- Node name `"0"` is ground and is treated specially in `circuit` (not registered as a normal node, ground accesses map to zero/dummy entries).
- Netlist parsing is two-stage in the plugin runtime: first parse lines/comments/continuations, then parse each statement into a component.
- The first parsed statement is treated as the circuit title when loading netlists.
- Dynamic components create an internal current-like companion node using the `@J<id>` naming pattern (`dynamic.hpp`).
