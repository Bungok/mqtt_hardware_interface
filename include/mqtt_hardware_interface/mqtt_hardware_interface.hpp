#ifndef MQTT_SYSTEM_INTERFACE
#define MQTT_SYSTEM_INTERFACE

#include "hardware_interface/system_interface.hpp"

#include <unordered_map>
#include <string>
#include <memory>

#include "mqtt/client.h"

namespace mqtt_hardware_interface
{

class MqttHardwareInterface : public hardware_interface::SystemInterface
{
public:
    MqttHardwareInterface(){}

    hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareComponentInterfaceParams & params) override;

    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

    hardware_interface::CallbackReturn on_shutdown(
      const rclcpp_lifecycle::State & previous_state) override;

    hardware_interface::CallbackReturn on_configure(
      const rclcpp_lifecycle::State & previous_state) override;

    hardware_interface::CallbackReturn on_cleanup(
      const rclcpp_lifecycle::State & previous_state) override;
  
    hardware_interface::CallbackReturn on_activate(
      const rclcpp_lifecycle::State & previous_state) override;
  
    hardware_interface::CallbackReturn on_deactivate(
      const rclcpp_lifecycle::State & previous_state) override;

    hardware_interface::CallbackReturn on_error(
      const rclcpp_lifecycle::State & previous_state) override;
  
    hardware_interface::return_type read(
      const rclcpp::Time & time, const rclcpp::Duration & period) override;
  
    hardware_interface::return_type write(
      const rclcpp::Time & time, const rclcpp::Duration & period) override;

protected:
  std::unordered_map<std::string, double> state_interface_to_states_;
  std::unordered_map<std::string, double> command_interface_to_commands_;
  std::unordered_map<std::string, std::string> state_interface_to_config_;
  std::unordered_map<std::string, std::string> command_interface_to_config_;
  hardware_interface::HardwareComponentInterfaceParams params_;
  std::string mqtt_broker_uri;
  std::string mqtt_client_id="MqttHardwareInterface";
  int qos = 0;
  std::shared_ptr<mqtt::async_client> mqtt_client;
  std::shared_ptr<mqtt::connect_options::v5> conn_opts;
};

}


#endif