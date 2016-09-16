#include "app/custom_message_types/group_value_message.h"

#include <ble_types.h>
#include "cmocka_includes.h"

static BleMessageGroupValueReq mockRequest;
static BleMessageType mockMessageType = 123;
static uint32_t deviceId;

static int test_setup(void **state) {
  deviceId = get_raw_device_id();
  memset(&mockRequest, 0, sizeof(mockRequest));
  mockRequest.messageType = mockMessageType;
  return 0;
}

static void GroupValueMessage_initialize_registers_message_and_adds_terminal_commands() {
  will_return(register_message_type, mockMessageType);

  expect_string(mesh_add_terminal_command, commandName, "vc");
  expect_string(mesh_add_terminal_command, commandName, "grp");
  expect_string(mesh_add_terminal_command, commandName, "val");
  group_value_message_initialize();
}

static void GroupValueMessage_receive_does_not_propagate_when_current_node_is_target() {
  mockRequest.deviceId = deviceId;
  assert_int_equal(DoNotPropagate, receive_group_value_message(0, &mockRequest));
}

static void GroupValueMessage_receive_propagates_to_all_when_current_node_is_not_target() {
  mockRequest.deviceId = 0x9999;
  assert_int_equal(PropagateToAll, receive_group_value_message(0, &mockRequest));
}

static void GroupValueMessage_receive_updates_group_when_requested() {
  uint8_t group = 123;
  mockRequest.deviceId = deviceId;
  mockRequest.newGroup = group;
  mockRequest.setGroup = true;

  expect_value(update_voting_group, newGroup, group);
  receive_group_value_message(0, &mockRequest);
}

static void GroupValueMessage_receive_updates_value_when_requested() {
  uint8_t value = 234;
  mockRequest.deviceId = deviceId;
  mockRequest.newValue = value;
  mockRequest.setValue = true;

  expect_value(update_voting_value, newValue, value);
  receive_group_value_message(0, &mockRequest);
}

static void GroupValueMessage_broadcast_sets_group_information() {
  mockRequest.deviceId = 123;
  mockRequest.setGroup = true;
  mockRequest.newGroup = 4;
  char* commands[3] = { "grp", "123", "4" };

  expect_value(send_to_all_connections, originHandle, BLE_CONN_HANDLE_INVALID);
  expect_value(send_to_all_connections, dataLength, sizeof(mockRequest));
  expect_memory(send_to_all_connections, data, &mockRequest, sizeof(mockRequest));
  broadcast_group_value_message(commands, 3);
}

static void GroupValueMessage_broadcast_sets_value_information() {
  mockRequest.deviceId = 567;
  mockRequest.setValue = true;
  mockRequest.newValue = 8;
  char* commands[3] = { "val", "567", "8" };

  expect_value(send_to_all_connections, originHandle, BLE_CONN_HANDLE_INVALID);
  expect_value(send_to_all_connections, dataLength, sizeof(mockRequest));
  expect_memory(send_to_all_connections, data, &mockRequest, sizeof(mockRequest));
  broadcast_group_value_message(commands, 3);
}

int RunGroupValueMessageTest(void) {
  const struct CMUnitTest tests[] = {
          cmocka_unit_test_setup(GroupValueMessage_initialize_registers_message_and_adds_terminal_commands, test_setup),
          cmocka_unit_test_setup(GroupValueMessage_receive_does_not_propagate_when_current_node_is_target, test_setup),
          cmocka_unit_test_setup(GroupValueMessage_receive_propagates_to_all_when_current_node_is_not_target, test_setup),
          cmocka_unit_test_setup(GroupValueMessage_receive_updates_group_when_requested, test_setup),
          cmocka_unit_test_setup(GroupValueMessage_receive_updates_value_when_requested, test_setup),
          cmocka_unit_test_setup(GroupValueMessage_broadcast_sets_group_information, test_setup),
          cmocka_unit_test_setup(GroupValueMessage_broadcast_sets_value_information, test_setup),
  };

  return cmocka_run_group_tests_name("GroupValueMessageTest", tests, NULL, NULL);
}

int main() {
  return RunGroupValueMessageTest();
}
