# Define minimal required version of CMake.
cmake_minimum_required(VERSION "3.25")

# Project definition
project(MatterSolution)

include(ExternalProject)

ExternalProject_Add(MatterLightSwitch
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../MatterLightSwitch/cmake_gcc
  BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/../MatterLightSwitch/cmake_gcc/build
  CMAKE_ARGS --preset project
  BUILD_COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG>
  INSTALL_COMMAND ""
  BUILD_ALWAYS true
  USES_TERMINAL_BUILD 1
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../MatterLightSwitch/cmake_gcc
)
ExternalProject_Add(Matter-Bootloader
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../Matter-Bootloader/cmake_gcc
  BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/../Matter-Bootloader/cmake_gcc/build
  CMAKE_ARGS --preset project
  BUILD_COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG>
  INSTALL_COMMAND ""
  BUILD_ALWAYS true
  USES_TERMINAL_BUILD 1
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../Matter-Bootloader/cmake_gcc
)

ExternalProject_Add_StepDependencies(Matter-Bootloader build MatterLightSwitch)


set(post_build_command ${POST_BUILD_EXE} postbuild "./MatterSolution.slpb")


add_custom_target(postbuild ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    COMMAND ${post_build_command}
)

add_dependencies(postbuild MatterLightSwitch Matter-Bootloader)

# BEGIN_SIMPLICITY_STUDIO_METADATA=eJytUktv1DAQ/itWVKH20MwWDkjbQAVdDpGSLsJbAVKkldeZbgyOJ9gOKUL8906bbraFIi4cPf5e8/iZyLx8X+Tn+erzWq4uF/lyLZfF5SpfXqzL5eJdkcyT7Oy6teI7+mDIvaqSk3RWJQKdptq4LRfeyPM8r5Kz15XLSkm2jwwMglkuzK+DYUgTYzcHGIYhHV6k5LfwfDY7gU9lIXWDrTo2LkTlNLIyM+aOLlSLoVMaR0BBWsXR/4FWMFZtQqqphRBgZ52iJs9KnEeILNxX71783qFKqnuLH/AKPfeCQZiaxYPV6UD+6511WquoqgQm6qJc4JVx5pb/d/w9nAkdhbjpja1lxG4q88e+eHDI6VvlavRHYsKLZ996iqcHh7v0R1CqGNFPPQbbbUZQBn9aZPC0ddZ5+oI6Csfj5fijaGG2TZSDibrh+dv9qKGhFoEX49Bb9xKkaTtrtIk/ZOxrQ/CRW5e3rf8WD54Qhn+kOH5LFC0pnsT/TfFIeFomPNrmeCvw4Fgy2N8yP5NfNwgLHCU==END_SIMPLICITY_STUDIO_METADATA