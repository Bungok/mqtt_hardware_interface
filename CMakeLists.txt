cmake_minimum_required(VERSION 3.8)
project(mqtt_hardware_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# define ROS Package dependencies
set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
  rclpy 
)

# find dependencies
find_package(ament_cmake REQUIRED)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

# mqtt hardware interface
add_library(
  mqtt_hardware_interface
  SHARED
  src/mqtt_hardware_interface.cpp
)
target_include_directories(
  mqtt_hardware_interface
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/mqtt_hardware_interface>
)
target_link_libraries(mqtt_hardware_interface ${pluginlib_TARGETS})
ament_target_dependencies(mqtt_hardware_interface ${THIS_PACKAGE_INCLUDE_DEPENDS})

pluginlib_export_plugin_description_file(
  hardware_interface mqtt_hardware_interface.xml)

install(
  DIRECTORY include/
  DESTINATION include/mqtt_hardware_interface
)

install(
  TARGETS mqtt_hardware_interface
  EXPORT export_mqtt_hardware_interface
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}
)

ament_export_targets(export_mqtt_hardware_interface HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})
ament_package()
