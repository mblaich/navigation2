// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef NAV2_DYNAMIC_PARAMS__DYNAMIC_PARAMS_CLIENT_HPP_
#define NAV2_DYNAMIC_PARAMS__DYNAMIC_PARAMS_CLIENT_HPP_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/parameter_events_filter.hpp"

namespace nav2_dynamic_params
{

class DynamicParamsClient
{
public:
  explicit DynamicParamsClient(
    rclcpp::Node::SharedPtr node, std::vector<std::string> remote_names = {""})
    : node_(node)
  {
    add_parameter_clients(remote_names);
    init_parameters();
  }

  ~DynamicParamsClient() {}

  void add_parameter_clients(std::vector<std::string> remote_names = {""})
  {
    for (auto & name : remote_names) {
      auto client = std::make_shared<rclcpp::SyncParametersClient>(node_, name);
      parameters_clients_.push_back(client);
    }
  }

  void set_callback(
    std::function<void(const rcl_interfaces::msg::ParameterEvent::SharedPtr)> callback)
  {
    for (auto & client : parameters_clients_) {
      auto sub = client->on_parameter_event(callback);
      event_subscriptions_.push_back(sub);
    }
  }

  void init_parameters(const std::vector<std::string> & param_names)
  {
    for (const auto & client : parameters_clients_) {
      auto params = client->get_parameters(param_names);
      for (const auto & param : params) {
        if (!dynamic_param_map_.count(param.get_name())) {
          dynamic_param_map_[param.get_name()] = param;
        }
      }
    }
  }

  void init_parameters()
  {
    std::vector<std::string> param_names;
    for (const auto & client : parameters_clients_) {
      auto param_list = client->list_parameters({}, 1);
      param_names.insert(param_names.end(), param_list.names.begin(), param_list.names.end());
    }
    init_parameters(param_names);
  }

  std::vector<std::string> get_param_names()
  {
    std::vector<std::string> names;
    for (const auto & entry : dynamic_param_map_) {
      names.push_back(entry.first);
    }
    return names;
  }

  std::map<std::string, rclcpp::Parameter> get_param_map()
  {
    return dynamic_param_map_;
  }

  template<class T>
  bool get_event_param(
    const rcl_interfaces::msg::ParameterEvent::SharedPtr event,
    const std::string & param_name, T & new_value)
  {
    rclcpp::ParameterEventsFilter filter(event, {param_name},
      {rclcpp::ParameterEventsFilter::EventType::NEW,
        rclcpp::ParameterEventsFilter::EventType::CHANGED});
    if (!(filter.get_events()).empty()) {
      auto param_msg = ((filter.get_events()).front()).second;
      auto param_value = rclcpp::Parameter::from_parameter_msg(*param_msg);
      new_value = param_value.get_value<T>();
      dynamic_param_map_[param_name] = param_value;
      return true;
    } else {
      if (dynamic_param_map_.count(param_name) > 0 &&
        !dynamic_param_map_[param_name].get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET)
      {
        new_value = dynamic_param_map_[param_name].get_value<T>();
      } else {
        RCLCPP_DEBUG(rclcpp::get_logger("dynamic_params_client"),
          "Parameter '%s' not set", param_name.c_str());
      }
      return false;
    }
  }

  template<class T>
  bool get_event_param(
    const rcl_interfaces::msg::ParameterEvent::SharedPtr event,
    const std::string & param_name, T & new_value, const T & default_value)
  {
    rclcpp::ParameterEventsFilter filter(event, {param_name},
      {rclcpp::ParameterEventsFilter::EventType::NEW,
        rclcpp::ParameterEventsFilter::EventType::CHANGED});
    if (!(filter.get_events()).empty()) {
      auto param_msg = ((filter.get_events()).front()).second;
      auto param_value = rclcpp::Parameter::from_parameter_msg(*param_msg);
      new_value = param_value.get_value<T>();
      dynamic_param_map_[param_name] = param_value;
      return true;
    } else {
      if (dynamic_param_map_.count(param_name) > 0 &&
        !dynamic_param_map_[param_name].get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET)
      {
        new_value = dynamic_param_map_[param_name].get_value<T>();
      } else {
        RCLCPP_INFO(rclcpp::get_logger("dynamic_params_client"),
          "Parameter '%s' not set, using default", param_name.c_str());
        new_value = default_value;
      }
      return false;
    }
  }

private:
  std::map<std::string, rclcpp::Parameter> dynamic_param_map_;
  std::vector<std::string> dynamic_param_names_;
  std::vector<rclcpp::SyncParametersClient::SharedPtr> parameters_clients_;
  rclcpp::Node::SharedPtr node_;
  std::vector<rclcpp::Subscription
    <rcl_interfaces::msg::ParameterEvent>::SharedPtr> event_subscriptions_;
};

}  // namespace nav2_dynamic_params

#endif  // NAV2_DYNAMIC_PARAMS__DYNAMIC_PARAMS_CLIENT_HPP_