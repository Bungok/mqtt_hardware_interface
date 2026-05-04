#include "mqtt_hardware_interface/mqtt_hardware_interface.hpp"
#include "mqtt_hardware_interface/mqtt_subscriber.hpp"


#include <vector>

namespace mqtt_hardware_interface
{

hardware_interface::CallbackReturn MqttHardwareInterface::on_init(
  const hardware_interface::HardwareComponentInterfaceParams & params)
{
  if (hardware_interface::SystemInterface::on_init(params) != CallbackReturn::SUCCESS)
  {
    return CallbackReturn::ERROR;
  }

  params_ = params;

  mqtt_broker_uri = params.hardware_info.hardware_parameters.at("mqtt_broker_uri");
  mqtt_client_id = params.hardware_info.hardware_parameters.at("mqtt_client_id");
  qos = std::stoi(params.hardware_info.hardware_parameters.at("qos"));

  if (mqtt_broker_uri.empty())
  {
    RCLCPP_ERROR_STREAM(
      rclcpp::get_logger("MqttHardwareInterface"),
      "MqttHardwareInterface broker uri for this hardware is empty. Need to set a uri address of the Mqtt broker.");
    return CallbackReturn::ERROR;
  }

  // initialize the configurations for each command and state/interface of each joint
  for (const hardware_interface::ComponentInfo & joint : params.hardware_info.joints)
  {
    // get parameters for each StateInterface
    for (auto & state_interface : joint.state_interfaces)
    {
      std::string state_interface_name = joint.name + "/" + state_interface.name;
      // initialize with no command received
      state_interface_to_states_[state_interface_name] = 0.0;
      // topic, set to default if not provided
      const std::string topic = state_interface.parameters.at("topic");
      if (topic.empty())
      {
        RCLCPP_WARN_STREAM(
          rclcpp::get_logger("MqttHardwareInterface"),
          "topic is empty for interface[" + state_interface_name + "].");
        return CallbackReturn::ERROR;
      }
      state_interface_to_config_.insert(std::make_pair(
        state_interface_name,
        topic));
    }

    // get parameters for each CommandInterface
    for (auto & command_interface : joint.command_interfaces)
    {
      std::string command_interface_name = joint.name + "/" + command_interface.name;

      // set initial to no command
      command_interface_to_commands_[command_interface_name] = 0.0;

      // topic, set to default if not provided
      const std::string topic = command_interface.parameters.at("topic");
      if (topic.empty())
      {
        RCLCPP_WARN_STREAM(
          rclcpp::get_logger("MqttHardwareInterface"),
          "topic is empty for interface[" + command_interface_name + "].");
        return CallbackReturn::ERROR;
      }
      command_interface_to_config_.insert(std::make_pair(
        command_interface_name,
        topic));
    }
  }
  RCLCPP_DEBUG_STREAM(rclcpp::get_logger("MqttHardwareInterface"), "Finished `on_init`!");

  return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MqttHardwareInterface::on_configure(
      const rclcpp_lifecycle::State & previous_state)
{
  mqtt_client = std::make_shared<mqtt::async_client>(mqtt_broker_uri, mqtt_client_id);
  conn_opts = std::make_shared<mqtt::connect_options>();

  mqtt_callback = std::make_shared<callback>(*mqtt_client, *conn_opts);
  //callback(*mqtt_client, *conn_opts);
  mqtt_client->set_callback(*mqtt_callback);

    mqtt_callback->set_config(state_interface_to_states_,state_interface_to_config_,qos);
  mqtt_client->connect()->wait();
    
  
  return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MqttHardwareInterface::on_cleanup(
      const rclcpp_lifecycle::State & previous_state)
{
    mqtt_client->disconnect()->wait();
    return CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> MqttHardwareInterface::export_state_interfaces()
{
  // joints
  std::vector<hardware_interface::StateInterface> state_interfaces;
  for (const auto & joint : params_.hardware_info.joints)
  {
    for (const auto & state_interface : joint.state_interfaces)
    {
      std::string state_interface_name = joint.name + "/" + state_interface.name;
      state_interfaces.emplace_back(hardware_interface::StateInterface(
        joint.name, state_interface.name, &state_interface_to_states_[state_interface_name]));
    }
  }

  // sensors
  for (const auto & sensor : params_.hardware_info.sensors)
  {
    for (const auto & state_interface : sensor.state_interfaces)
    {
      std::string state_interface_name = sensor.name + "/" + state_interface.name;
      state_interfaces.emplace_back(hardware_interface::StateInterface(
        sensor.name, state_interface.name, &state_interface_to_states_[state_interface_name]));
    }
  }

  return state_interfaces;
}



std::vector<hardware_interface::CommandInterface> MqttHardwareInterface::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (const auto & joint : params_.hardware_info.joints)
  {
    for (const auto & command_interface : joint.command_interfaces)
    {
      std::string command_interface_name = joint.name + "/" + command_interface.name;
      command_interfaces.emplace_back(hardware_interface::CommandInterface(
        joint.name, command_interface.name,
        &command_interface_to_commands_[command_interface_name]));
    }
  }

  return command_interfaces;
}

hardware_interface::return_type MqttHardwareInterface::read(
const rclcpp::Time & time, const rclcpp::Duration & period)
{
  for(auto state : mqtt_callback->get_states()){
    state_interface_to_states_[state.first] = state.second;
  } 
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type MqttHardwareInterface::write(
const rclcpp::Time & time, const rclcpp::Duration & period)
{
  for(auto command : command_interface_to_config_){
    mqtt::topic top(*mqtt_client, command.second, qos);
    mqtt::token_ptr tok;
    tok = top.publish(std::to_string(command_interface_to_commands_[command.first]));
    tok->wait();
  }
        
  return hardware_interface::return_type::OK;
}

}

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  mqtt_hardware_interface::MqttHardwareInterface, hardware_interface::SystemInterface)
