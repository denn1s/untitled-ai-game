cmake_minimum_required(VERSION 3.10)

project(GAME)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external)

FetchContent_Declare(
  llama
  GIT_REPOSITORY git@github.com:ggerganov/llama.cpp.git
  GIT_TAG        master
)

FetchContent_GetProperties(llama)
if(NOT llama_POPULATED)
  FetchContent_Populate(llama)
  add_subdirectory(${llama_SOURCE_DIR} ${llama_BINARY_DIR})
endif()

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
      ${llama_SOURCE_DIR}
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(TBB REQUIRED)

include_directories("/usr/include/fmod/")
set(FMOD_LIBRARIES "/usr/lib/libfmod.so")

# link libraries
target_link_libraries(${PROJECT_NAME}
  ${SDL2_LIBRARIES}
  SDL2_image
  ${GLM_LIBRARIES}
  SDL2_ttf
  common
  llama
  TBB::tbb
  ${FMOD_LIBRARIES}
)
