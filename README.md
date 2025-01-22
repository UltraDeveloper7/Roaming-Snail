# Billiards

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
9. [Running the Game](#running-the-game)
10. [Directory Structure](#directory-structure)
11. [Dependencies](#dependencies)
12. [Contributing](#contributing)
13. [License](#license)
14. [Contact](#contact)
15. [Acknowledgements](#acknowledgements)

## Features
- Realistic billiards physics with customizable gameplay.
- Dual-player mode with score tracking.
- Control over camera movement, cue ball force, direction, and spin.
- 10 toggleable lights evenly placed around the table rim for enhanced visuals.
- Ability to modify cue strike power and ball friction during the game.
- Comprehensive enforcement of official billiards rules.

## Technologies Used
- C
- C++
- OpenGL
- Visual Studio

## Libraries
The following libraries are used in the project:
- **GLFW**: For handling window creation and input.
- **GLAD**: For managing OpenGL extensions.
- **GLM**: For mathematics related to graphics rendering.
- **FreeType**: For rendering text.
- **stb_image**: For loading images as textures.
- **tiny_obj_loader**: For loading 3D model files.

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
- Visual Studio installed with C++ development tools.
- OpenGL support on your system.

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

## Running the Game
1. **Open the Project**:
   - Open the Visual Studio solution file (`Billiards.sln`) in the root directory.

2. **Build the Project**:
   - Build the solution in Visual Studio to compile the game.

3. **Run the Game**:
   - Start the game by running the compiled executable from Visual Studio.

## Directory Structure
```
Billiards/
├── README.md
├── assets/
│   ├── fonts/         # Font files used in the project.
│   ├── hdr/           # High Dynamic Range images for lighting.
│   ├── models/        # 3D models used in the game.
│   ├── textures/      # Texture files for materials and surfaces.
├── src/
│   ├── core/          # Core functionality like game engine systems.
│   ├── glad/          # OpenGL library for rendering.
│   ├── include/       # Header files for shared definitions.
│   ├── interface/     # UI elements and interaction logic.
│   ├── objects/       # Definitions for game objects like balls and tables.
│   ├── shaders/       # Shader programs for rendering effects.
│   ├── Config.hpp     # Configuration settings for the game.
│   ├── Logger.hpp     # Logging utility for debugging.
│   ├── main.cpp       # Entry point of the application.
│   ├── stdafx.cpp     # Precompiled header source for faster compilation.
│   ├── stdafx.h       # Precompiled header file for standard includes.
├── x64/               # Compiled binaries for the 64-bit architecture.
├── Billiards.sln      # Visual Studio solution file.
├── Billiards.vcxproj  # Visual Studio project file.
├── freetype.dll       # FreeType library for font rendering.
├── log/               # Directory for log files (empty initially).
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
