#include "mqtt_hardware_interface/mqtt_hardware_interface.hpp"

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
  qos = params.hardware_info.hardware_parameters.at("qos");

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
  conn_opts = std::make_shared<mqtt::connect_options_builder::v5>()
                        .clean_start(false)
                        .properties({{mqtt::property::SESSION_EXPIRY_INTERVAL, 604800}})
                        .finalize();
  try 
  {
    // Start consumer before connecting to make sure to not miss messages
    mqtt_client->start_consuming();

    // Connect to the server
    auto tok = cli->connect(&conn_opts);

    // Getting the connect response will block waiting for the
    // connection to complete.
    auto rsp = tok->get_connect_response();

    RCLCPP_DEBUG_STREAM(rclcpp::get_logger("MqttHardwareInterface"), "Connected to the mqtt broker");

    if(!rsp.is_session_present()){
      for (auto conf : state_interface_to_config_){
        mqtt_client->subscribe(conf.second,qos)->wait();
      }

      for (auto conf : command_interface_to_config_){
        mqtt_client->subscribe(conf.second,qos)->wait();
      }
    }
    
  }
  catch (const mqtt::exception& exc) {
        RCLCPP_ERROR_STREAM(
        rclcpp::get_logger("MqttHardwareInterface"),
        "Mqtt error: " + exc);
        return CallbackReturn::ERROR;
    }

  return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn MqttHardwareInterface::on_cleanup(
      const rclcpp_lifecycle::State & previous_state)
{
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
  for(auto interface : state_interface_to_states_)
  {
    auto evt = mqtt_client->consume_event();
    if (const auto* p = evt.get_message_if()) {
        auto& msg = *p;
        state_interface_to_states_[msg->get_topic()] = std::stof(msg->to_string());
    }
  }
  
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type MqttHardwareInterface::read(
const rclcpp::Time & time, const rclcpp::Duration & period)
{
  auto pubmsg = mqtt::make_message(TOPIC, PAYLOAD1);
  return hardware_interface::return_type::OK;
}

}

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  mqtt_hardware_interface::MqttHardwareInterface, hardware_interface::SystemInterface)