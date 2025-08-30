# Billiards
<img width="1912" height="1033" alt="image" src="https://github.com/user-attachments/assets/6a6c3ff1-e8a2-4898-bb85-82c03b984535" />
<img width="1914" height="1028" alt="image" src="https://github.com/user-attachments/assets/7e1e97a9-6b8d-41fa-964e-86645ce89fc3" />
<img width="1911" height="1031" alt="image" src="https://github.com/user-attachments/assets/ef8f8d6f-d299-408a-b3b3-56c54c59c526" />

## Overview
This repository contains the source code for a dual-player Billiards game created with OpenGL in Visual Studio. The game provides an immersive billiards experience with realistic graphics, physics, and customizable settings.

## Table of Contents
1. [Features](#features)
2. [Technologies Used](#technologies-used)
3. [Libraries](#libraries)
4. [Controls](#controls)
5. [Game Rules](#game-rules)
6. [Prerequisites](#prerequisites)
7. [Installation](#installation)
8. [Usage](#usage)
9. [Building with CMake](#building-with-cmake) 
10. [Running the Game](#running-the-game)
11. [Directory Structure](#directory-structure)
12. [Dependencies](#dependencies)
13. [Contributing](#contributing)
14. [License](#license)
15. [Contact](#contact)
16. [Acknowledgements](#acknowledgements)

## Features
- Realistic billiards physics with customizable gameplay.
- Dual-player mode with score tracking.
- Control over camera movement, cue ball force, direction, and spin.
- 10 toggleable lights evenly placed around the table rim for enhanced visuals.
- Ability to modify cue strike power and ball friction during the game.
- Comprehensive enforcement of official billiards rules.

## Technologies Used
- C / C++
- OpenGL
- CMake
- Visual Studio

## Libraries
The following libraries are used in the project:
- **GLFW**: For handling window creation and input.
- **GLAD**: For managing OpenGL extensions.
- **GLM**: For mathematics related to graphics rendering.
- **FreeType**: For rendering text.
- **stb_image**: For loading images as textures.
- **tiny_obj_loader**: For loading 3D model files.
- **earcut-hpp** – Polygon triangulation

## Controls
- **Camera Movement**: Use `W`, `A`, `S`, `D` to move the camera, and `Q`, `E` to move it up and down.
- **Cue Ball Force & Direction**: Use arrow keys to adjust force and direction.
- **Spin**: Adjust spin using the 2D projection of the cue ball.
- **Lighting**: Toggle the 10 lights around the table rim.
- **Game Settings**: Modify cue strike power and ball friction in-game.

## Game Rules
The game adheres to official billiards rules, including:
1. Players take turns striking the cue ball.
2. A valid shot requires the cue ball to hit the target ball and then at least one ball must reach a rail or be pocketed.
3. Fouls include:
   - Failing to hit any ball.
   - Pocketing the cue ball.
   - Hitting the opponent's ball first.
4. The game ends when all balls of one player are pocketed, followed by the 8-ball, provided no foul is committed.

For detailed rules, refer to the [World Pool-Billiard Association (WPA)](https://wpapool.com/rules-of-play/).

## Prerequisites
Before you begin, ensure you have:
1. **Visual Studio 2022** (with C++ Desktop Development tools installed).  
2. **CMake** ≥ 3.24.  
3. **vcpkg** installed locally (and bootstrapped at least once):
   ```powershell
   git clone https://github.com/microsoft/vcpkg C:/DEV/vcpkg
   cd C:/DEV/vcpkg
   .\bootstrap-vcpkg.bat

## Installation
### Clone from GitHub
1. Clone the repository:
    ```bash
    git clone https://github.com/UltraDeveloper7/Billiards.git
    cd Billiards
    ```

### Download as ZIP
1. Download the ZIP file from GitHub:
    - Go to the repository page.
    - Click on the "Code" button and select "Download ZIP".
    - Extract the ZIP file and navigate to the directory.

## Usage
To play the Billiards game, build and run the project in Visual Studio, then use the controls described above to interact with the game.

## Building with CMake
**Step 1: Clone**
```
git clone https://github.com/UltraDeveloper7/Billiards.git
cd Billiards
```
**Step 2: Configure with CMake GUI**
1. Open **CMake GUI**.
2. **Where is the source code:** set to the repo root (`Billiards/`).
3. **Where to build the binaries:** create or select `Billiards/build`.
4. Click **Configure** → choose Visual Studio 17 2022, platform `x64`.
5. In **cache entries**, set:
```
CMAKE_TOOLCHAIN_FILE = C:/DEV/vcpkg/scripts/buildsystems/vcpkg.cmake
```
6. Click **Configure** again. CMake will automatically use `vcpkg.json` to fetch and build all required dependencies.
7. Click **Generate** → then Open Project in Visual Studio.

**Step 3: Build**
- In Visual Studio, select `Debug` or `Release`.
- Build the **Billiards** target.

## Running the Game
1. **Open the Project**:
   - Open the Visual Studio solution file (`build/Billiards.sln`) in the build directory.
   - Ensure `assets/` and `src/shaders/` are copied next to the exe (CMake handles this automatically).
     Example layout after build:
     ```
     build/bin/Debug/
        Billiards.exe
        assets/
        src/shaders/
     ```
2. **Run the Game**:
   - Run directly from Visual Studio (`F5`) or by launching `Billiards.exe` from the build folder.

## Directory Structure
```
Billiards/
├── assets/              # Fonts, HDRs, models, textures
├── src/                 # Source code
│   ├── core/            # Core engine code
│   ├── gameplay/        # Game logic
│   ├── interface/       # UI
│   ├── objects/         # Ball, table, cue definitions
│   ├── shaders/         # GLSL shaders
│   ├── Config.hpp
│   ├── Logger.hpp
│   ├── main.cpp
│   ├── precompiled.cpp/.h
├── CMakeLists.txt       # CMake build script
├── vcpkg.json           # Declares dependencies
└── build/               # Out-of-source build (generated)
```

## Dependencies
- **Wavefront OBJ loading with MTL**: The game supports loading `.obj` files, including material files with Physically Based Rendering (PBR) extensions. All `.obj` models were created using [Blender](https://www.blender.org/).
- **PBR Point Lights**: Implements Physically Based Rendering (PBR) for point light sources, offering realistic lighting effects.
- **IBL using HDR Irradiance Map and GGX Microfacet Model**: Uses Image-Based Lighting (IBL) with HDR maps for realistic reflections and lighting.
- **Multiple Texture Maps per Material**: Supports multiple textures for each material, such as diffuse, specular, and normal maps.
- **FreeType-Based Text Renderer**: Uses the FreeType library for rendering crisp and scalable text in the game.
- **Deferred Scene Loading**: Loads scenes in a deferred manner to improve performance and responsiveness.
- **Modifiable Point Light Count**: Allows dynamic adjustment of the number of point lights in the scene.
- **Model Vertex Deduplication**: Optimizes 3D models by eliminating redundant vertices, reducing memory usage.
- **Resizable Viewport with Text Scaling**: Ensures that the viewport adjusts dynamically to window resizing, maintaining proper text scaling and aspect ratio.

## Contributing
1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/YourFeature`).
3. Commit your changes (`git commit -m 'Add some feature'`).
4. Push to the branch (`git push origin feature/YourFeature`).
5. Open a pull request.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact
If you have any questions or issues, please contact [konstantinostoutounas@gmail.com](mailto:konstantinostoutounas@gmail.com).

## Acknowledgements
- Special thanks to the contributors and open-source libraries that made this project possible.

---

Thank you for playing the Billiards game! Enjoy your match!

Feel free to update the content as necessary and add any missing details.
