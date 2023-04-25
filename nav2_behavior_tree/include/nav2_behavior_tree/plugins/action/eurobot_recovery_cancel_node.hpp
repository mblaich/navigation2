#ifndef NAV2_BEHAVIOR_TREE__PLUGINS__ACTION__EUROBOT_RECOVERY_CANCEL_NODE_HPP_
#define NAV2_BEHAVIOR_TREE__PLUGINS__ACTION__EUROBOT_RECOVERY_CANCEL_NODE_HPP_

#include "nav2_behavior_tree/bt_cancel_action_node.hpp"
#include "eurobot_navigation_interfaces/action/eurobot_recovery.hpp"

namespace nav2_behavior_tree{

class EurobotRecoveryCancel : public BtCancelActionNode<eurobot_navigation_interfaces::action::EurobotRecovery>
{
public:
  /**
   * @brief A constructor for nav2_behavior_tree::BackUpAction
   * @param xml_tag_name Name for the XML tag for this node
   * @param action_name Action name this node creates a client for
   * @param conf BT node configuration
   */
  EurobotRecoveryCancel(
    const std::string & xml_tag_name,
    const std::string & action_name,
    const BT::NodeConfiguration & conf);

  /**
   * @brief Function to perform some user-defined operation on tick
   */
  void on_tick();

  /**
   * @brief Creates list of BT ports
   * @return BT::PortsList Containing basic ports along with node-specific ports
   */
  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({});
  }
};

}

#endif  // NAV2_BEHAVIOR_TREE__PLUGINS__ACTION__EUROBOT_RECOVERY_CANCEL_NODE_HPP_