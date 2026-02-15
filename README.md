# vista-explorer

A playground to experiment with and demonstrate the unofficial DNV Vista C++ SDK. Currently features a Gmod tree explorer matching the [official DNV web interface](https://vista.dnv.com/gmod).

<p align="center">
  <img src="docs/images/Gmod_viewer_1.png" alt="Gmod Viewer Screenshot">
</p>

## Features

- **Version Selector**: Browse Gmod trees across VIS versions (v3.4a through v3.10a)
- **Dual Badge System**: Product Types display with both parent function code (green) and type code (red)

## Building

```bash
cmake -B build -G Ninja
ninja -C build
./build/bin/nfx-vista-explorer
```

## Requirements

- C++20 compiler (Clang 19+)
- CMake 3.20+
- OpenGL 4.5+

## Dependencies (fetched automatically via CMake)
- [DNV Vista SDK](https://github.com/nfx-opensource/vista-sdk/tree/feature/cpp/) (C++ branch, unofficial fork) - MIT
- [Official DNV Vista SDK](https://github.com/dnv-opensource/vista-sdk) - MIT
- [ImGui](https://github.com/ocornut/imgui) (docking branch) - MIT
- [GLFW](https://github.com/glfw/glfw) - Zlib
