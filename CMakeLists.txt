cmake_minimum_required(VERSION 3.24)
project(Billiards LANGUAGES CXX)

# ---- C++ & MSVC runtime ------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# ---- Sources -----------------------------------------------------------------
file(GLOB_RECURSE BILLIARDS_SRC CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_SOURCE_DIR}/src/*.c"
  "${CMAKE_SOURCE_DIR}/src/*.h"
  "${CMAKE_SOURCE_DIR}/src/*.hpp"
)
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/appicon.rc")
  list(APPEND BILLIARDS_SRC "${CMAKE_SOURCE_DIR}/appicon.rc")
endif()

add_executable(Billiards ${BILLIARDS_SRC})
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${BILLIARDS_SRC})

# Exe output + run from exe dir in Visual Studio
set_target_properties(Billiards PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Billiards>"
)

target_include_directories(Billiards PRIVATE "${CMAKE_SOURCE_DIR}/src")

# Opt-in to GLM experimental extensions if you use them
target_compile_definitions(Billiards PRIVATE GLM_ENABLE_EXPERIMENTAL)

# ---- Dependencies via vcpkg (manifest mode) ---------------------------------
find_package(OpenGL REQUIRED)
find_package(glfw3          CONFIG REQUIRED)
find_package(glad           CONFIG REQUIRED)
find_package(Freetype       CONFIG REQUIRED)
find_package(tinyxml2       CONFIG REQUIRED)
find_package(glm            CONFIG REQUIRED)
find_package(tinyobjloader  CONFIG REQUIRED)

# stb (header-only)
find_path(STB_INCLUDE_DIR "stb_image.h"
  PATHS
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
    "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}"
    "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}"
  PATH_SUFFIXES include
)
if(NOT STB_INCLUDE_DIR)
  message(FATAL_ERROR "stb_image.h not found. Ensure 'stb' is in vcpkg.json and vcpkg toolchain is active.")
endif()
target_include_directories(Billiards PRIVATE "${STB_INCLUDE_DIR}")

# mapbox/earcut.hpp (header-only) from vcpkg port "earcut-hpp"
find_path(MAPBOX_EARCUT_INCLUDE_DIR "mapbox/earcut.hpp"
  PATHS
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}"
    "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}"
    "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}"
  PATH_SUFFIXES include
)
if(NOT MAPBOX_EARCUT_INCLUDE_DIR)
  find_path(MAPBOX_EARCUT_INCLUDE_DIR "mapbox/earcut.hpp")
endif()
if(NOT MAPBOX_EARCUT_INCLUDE_DIR)
  message(FATAL_ERROR "mapbox/earcut.hpp not found. Ensure 'earcut-hpp' is in vcpkg.json and the vcpkg toolchain is active.")
endif()
target_include_directories(Billiards PRIVATE "${MAPBOX_EARCUT_INCLUDE_DIR}")

# glfw target name portability
if(TARGET glfw)
  set(GLFW_TARGET glfw)
elseif(TARGET glfw::glfw)
  set(GLFW_TARGET glfw::glfw)
elseif(TARGET glfw3)
  set(GLFW_TARGET glfw3)
else()
  message(FATAL_ERROR "Could not resolve a GLFW target name.")
endif()

target_link_libraries(Billiards PRIVATE
  ${GLFW_TARGET}
  glad::glad
  Freetype::Freetype
  tinyxml2::tinyxml2
  glm::glm
  tinyobjloader::tinyobjloader
  OpenGL::GL
)
if(WIN32)
  target_link_libraries(Billiards PRIVATE user32 gdi32 shell32)
endif()

# ---- Resources: list + copy shaders -----------------------------------------
# Add shaders to project (not compiled) and keep them under src/shaders in VS
file(GLOB_RECURSE SHADER_FILES "${CMAKE_SOURCE_DIR}/src/shaders/*")
if(SHADER_FILES)
  target_sources(Billiards PRIVATE ${SHADER_FILES})
  foreach(_s IN LISTS SHADER_FILES)
    set_source_files_properties("${_s}" PROPERTIES HEADER_FILE_ONLY ON)
  endforeach()
  source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${SHADER_FILES})
endif()

# Copy assets and shaders to the locations your app expects:
#   <exe dir>/assets
#   <exe dir>/src/shaders
add_custom_command(TARGET Billiards POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:Billiards>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/src/shaders"
          "$<TARGET_FILE_DIR:Billiards>/src/shaders"
  COMMENT "Copying assets/ and src/shaders/ next to the executable"
)

# Optional: install rules (cmake --install build)
install(TARGETS Billiards RUNTIME DESTINATION bin)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/"      DESTINATION bin/assets)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/shaders/" DESTINATION "bin/src/shaders")
