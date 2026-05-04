#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

#include "mqtt/async_client.h"
#include "mqtt_hardware_interface/mqtt_subscriber.hpp"


const int N_RETRY_ATTEMPTS = 2; 
    // The MQTT client
    
    // Options to use if we need to reconnect
    
    // An action listener to display the result of actions.
    

    

    // This deomonstrates manually reconnecting to the broker by calling
    // connect() again. This is a possibility for an application that keeps
    // a copy of it's original connect_options, or if the app wants to
    // reconnect with different options.
    // Another way this can be done manually, if using the same options, is
    // to just call the async_client::reconnect() method.
    void callback::reconnect()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2500));
        try {
            cli_.connect(connOpts_, nullptr, *this);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
            exit(1);
        }
    }

    // Re-connection failure
    void callback::on_failure(const mqtt::token& tok)
    {
        std::cout << "Connection attempt failed" << std::endl;
        if (++nretry_ > N_RETRY_ATTEMPTS)
            exit(1);
        reconnect();
    }

    // (Re)connection success
    // Either this or connected() can be used for callbacks.
    void callback::on_success(const mqtt::token& tok)  {}

    // (Re)connection success
    void callback::connected(const std::string& cause) 
    {
        for(auto top : states_){
        cli_.subscribe(top.first, qos, nullptr, subListener_);
        }
    }

    // Callback for when the connection is lost.
    // This will initiate the attempt to manually reconnect.
    void callback::connection_lost(const std::string& cause) 
    {
        std::cout << "\nConnection lost" << std::endl;
        if (!cause.empty())
            std::cout << "\tcause: " << cause << std::endl;

        std::cout << "Reconnecting..." << std::endl;
        nretry_ = 0;
        reconnect();
    }

    // Callback for when a message arrives.
    void callback::message_arrived(mqtt::const_message_ptr msg){ 
        states_[config_[msg->get_topic()]] = std::stof(msg->to_string());
    }

    void callback::delivery_complete(mqtt::delivery_token_ptr token)  {}

    callback::callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
        : nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription")
    {
        qos = 0;
    }

    void callback::set_config(std::unordered_map<std::string, double> stat, std::unordered_map<std::string, std::string> conf, int qs){
        states_ = stat;
        config_ = conf;
        qos = qs;
    }

    std::unordered_map<std::string, double> callback::get_states(){
        return states_;
    }
