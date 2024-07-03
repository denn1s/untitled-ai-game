cmake_minimum_required(VERSION 3.10)

project(GAME)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Add the executable target for the parent project
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

# Include the ExternalProject module
include(ExternalProject)

# Define the external project
ExternalProject_Add(
    llama.cpp
    PREFIX ${CMAKE_BINARY_DIR}/external/llama.cpp
    GIT_REPOSITORY https://github.com/ggerganov/llama.cpp.git
    GIT_TAG master
    UPDATE_DISCONNECTED 1
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

# Get the properties of the external project
ExternalProject_Get_Property(llama.cpp BINARY_DIR)
ExternalProject_Get_Property(llama.cpp SOURCE_DIR)
set(LLAMA_CPP_LIB_DIR ${BINARY_DIR})
set(LLAMA_CPP_INCLUDE_DIR ${SOURCE_DIR})

# Add the include directories of the external project
include_directories(${LLAMA_CPP_INCLUDE_DIR})
include_directories(${LLAMA_CPP_INCLUDE_DIR}/include)
include_directories(${LLAMA_CPP_INCLUDE_DIR}/common)
include_directories(${LLAMA_CPP_INCLUDE_DIR}/ggml/include)

# Create targets for the external project libraries
add_library(llama_common STATIC IMPORTED)
set_target_properties(llama_common PROPERTIES
    IMPORTED_LOCATION ${LLAMA_CPP_LIB_DIR}/common/libcommon.a
    INTERFACE_INCLUDE_DIRECTORIES ${LLAMA_CPP_INCLUDE_DIR}
)

add_library(llama_ggml SHARED IMPORTED)
set_target_properties(llama_ggml PROPERTIES
    IMPORTED_LOCATION ${LLAMA_CPP_LIB_DIR}/ggml/src/libggml.so
    INTERFACE_INCLUDE_DIRECTORIES ${LLAMA_CPP_INCLUDE_DIR}
)

add_library(llama_core SHARED IMPORTED)
set_target_properties(llama_core PROPERTIES
    IMPORTED_LOCATION ${LLAMA_CPP_LIB_DIR}/src/libllama.so
    INTERFACE_INCLUDE_DIRECTORIES ${LLAMA_CPP_INCLUDE_DIR}
)

# Add dependencies to ensure the external project is built before the parent project
add_dependencies(${PROJECT_NAME} llama.cpp)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(TBB REQUIRED)

include_directories("/usr/include/fmod/")
set(FMOD_LIBRARIES "/usr/lib/libfmod.so")

# Link libraries
target_link_libraries(${PROJECT_NAME}
  ${SDL2_LIBRARIES}
  SDL2_image
  ${GLM_LIBRARIES}
  SDL2_ttf
  llama_ggml
  llama_common
  llama_core
  TBB::tbb
  ${FMOD_LIBRARIES}
)

