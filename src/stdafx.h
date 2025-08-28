#pragma once

#ifndef GLAD_GL_H_
#include <glad/gl.h>
#endif

#include "Config.hpp"

#include <GLFW/glfw3.h>
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