#pragma once

// Make Windows headers leaner and avoid macro collisions (min/max, GDI ERROR, etc.)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef NOGDI
#define NOGDI
#endif

#include <windows.h>


#ifndef GLAD_GL_H_
#include <glad/glad.h>
#endif

#include "Config.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <unordered_map>
#include <map>
#include <fstream>
#include <format>
#include <algorithm>
#include <random>
#include <functional>

// Include mapbox/earcut.hpp
#include <mapbox/earcut.hpp>