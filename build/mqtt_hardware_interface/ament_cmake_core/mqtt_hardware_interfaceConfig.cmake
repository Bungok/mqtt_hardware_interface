# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_mqtt_hardware_interface_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED mqtt_hardware_interface_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(mqtt_hardware_interface_FOUND FALSE)
  elseif(NOT mqtt_hardware_interface_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(mqtt_hardware_interface_FOUND FALSE)
  endif()
  return()
endif()
set(_mqtt_hardware_interface_CONFIG_INCLUDED TRUE)

# output package information
if(NOT mqtt_hardware_interface_FIND_QUIETLY)
  message(STATUS "Found mqtt_hardware_interface: 0.0.0 (${mqtt_hardware_interface_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'mqtt_hardware_interface' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT mqtt_hardware_interface_DEPRECATED_QUIET)
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(mqtt_hardware_interface_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "ament_cmake_export_targets-extras.cmake;ament_cmake_export_dependencies-extras.cmake")
foreach(_extra ${_extras})
  include("${mqtt_hardware_interface_DIR}/${_extra}")
endforeach()
