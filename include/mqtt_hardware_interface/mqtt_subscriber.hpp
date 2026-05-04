#ifndef MQTT_SUB
#define MQTT_SUB

#include <string>
#include <unordered_map>

#include "mqtt/client.h"


class action_listener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token& tok) override
    {
        std::cout << name_ << " failure";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        std::cout << std::endl;
    }

    void on_success(const mqtt::token& tok) override
    {
        std::cout << name_ << " success";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty())
            std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        std::cout << std::endl;
    }

public:
    action_listener(const std::string& name) : name_(name) {}
};

class callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
  void reconnect();
  void on_failure(const mqtt::token& tok) override;
  void on_success(const mqtt::token& tok) override;
  void connected(const std::string& cause) override;
  void connection_lost(const std::string& cause) override;
  void message_arrived(mqtt::const_message_ptr msg) override;
  void delivery_complete(mqtt::delivery_token_ptr token) override;

    int nretry_;
    int qos;
    mqtt::async_client& cli_;
    mqtt::connect_options& connOpts_;
    action_listener subListener_;
    std::unordered_map<std::string, double> states_;
    std::unordered_map<std::string, std::string> config_;

  public:
    callback(mqtt::async_client& cli, mqtt::connect_options& connOpts);
    void set_config(std::unordered_map<std::string, double> stat, std::unordered_map<std::string, std::string> conf,int qs);
    std::unordered_map<std::string, double> get_states();

};

#endif