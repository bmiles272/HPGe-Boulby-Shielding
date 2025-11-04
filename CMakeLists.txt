cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(HPGeShielding)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(sim sim.cc ${sources} ${headers})
target_include_directories(sim PRIVATE include)
target_link_libraries(sim ${Geant4_LIBRARIES})

add_custom_target(HPGeShielding DEPENDS sim)

