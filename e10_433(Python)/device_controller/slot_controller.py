from . import slot_address
import umsgpack
import random
import uasyncio
import queue
import random
from umsgpack import loads as unpack
from time import ticks_ms
from machine import Pin
import random
# from slot_config import slot_address

from .device_config import (
    SlotCommunicationProtocol,
    SlotCanAddressTable,
    DeviceConfig
)
from radio_driver.radio_controller import RadioController
from can_driver.can.can_controller import CANController
from can_driver import CANFrame, ERROR
from . import command_queue

TEST_SLOT_TELEMETRY = {
    SlotCommunicationProtocol.STATION_TIMESTAMP_KEY: ticks_ms(),
    SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_TELEMETRY_CMD_VALUE,
    "VSolar": 5.5,
    "VBat1": 4.25,
    "VBat2": 3.75,
    "VSys": 4.0,
    "ISys": 170,
    "Temp": 24
}

action_start_time = 0
SLOT_RESPONSE_KEY = "response"

RADIO_CHANNEL_DIRECTION = 0
CAN_CHANNEL_DIRECTION = 1


class WrongTaskParametersException(Exception):
    def __init__(self, msg=""):
        super().__init__(msg)


class SlotTaskInterruptedException:
    def __init__(self):
        super().__init__


class StateContainer:
    def __init__(self):
        self.firmware_version = DeviceConfig.FIRMWARE_VER

    @staticmethod
    def get_device_state():
        return [DeviceConfig.FIRMWARE_VER, ticks_ms()]


class SlotController:
    task_handler = None
    controller_cmd_queue_task_handler = None

    initialized = False
    _instance = None
    signalization_pin = Pin(25, Pin.OUT)

    def __init__(self):
        if not self.initialized:
            self.initialized = True
        self._slot_addr = DeviceConfig.INTERNAL_SLOT_ADDRESS

    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            cls._instance = super(SlotController, cls).__new__(cls, *args, **kwargs)
        return cls._instance

    def is_task_parameters_consistent(self, parameters):
        #         [T0, dT, rps, timeout]
        return len(parameters) == 4 and not \
            (parameters[2] != 0 and parameters[3] != 0) and \
            (parameters[3] >= 0) and \
            (parameters[1] < parameters[3] if parameters[3] > 0 else True)

    def handle_received_command(self, message):

        cmd = message[SlotCommunicationProtocol.SLOT_COMMAND_KEY]
        if cmd == SlotCommunicationProtocol.GET_STATE_CMD_VALUE:
            uasyncio.create_task(self.send_slot_state())
        elif cmd == SlotCommunicationProtocol.SET_STATE_CMD_VALUE:
            res = ERROR.ERROR_OK
            uasyncio.create_task(self.send_response(cmd, res))
        elif cmd == SlotCommunicationProtocol.GET_TELEMETRY_CMD_VALUE:
            uasyncio.create_task(self.send_slot_telemetry())
        elif cmd == SlotCommunicationProtocol.SET_SIGNALIZATION_STATE:
            self.change_signalization(int(message["action"]))
            res = ERROR.ERROR_OK
            uasyncio.create_task(self.send_response(cmd, res))
        elif cmd == SlotCommunicationProtocol.GET_SIGNALIZATION_STATE:
            uasyncio.create_task(self.send_signalization_state())
        elif cmd == SlotCommunicationProtocol.ADD_TASK_CMD_VALUE:
            print("Adding Task ... Doing_smth")
            # call function which adding task to scheduler and returns error code
            action_title = message[SlotCommunicationProtocol.SLOT_TASK_ACTION]
            if action_title == SlotCommunicationProtocol.SEND_TELEMETRY_CMD_VALUE:
                action = self.send_slot_telemetry
            elif action_title == SlotCommunicationProtocol.SEND_STATE_CMD_VALUE:
                action = self.send_slot_state
            else:
                raise WrongTaskParametersException("Unknown action title")

            task_params = message[SlotCommunicationProtocol.SLOT_TASK_PARAMETERS]

            #
            if not self.is_task_parameters_consistent(task_params):
                raise WrongTaskParametersException("Task parameters check failed")

            print("Task parameters check: OK")
            #             avaliable_param_fields = parameters_dict.keys()

            # TODO: Create method for parameters consistency check

            #             init_timeout, internal_delay, rep_count, task_timeout = self.check_task_parameters(avaliable_param_fields,
            #                                                                                                parameters_dict)

            task_body = self.construct_task_routine(action, task_params)
            uasyncio.create_task(task_body())

            #
            # self.send_response(cmd, res)
        elif cmd == SlotCommunicationProtocol.EXECUTE_CMD_VALUE:
            print("Executing smt ... ")
            res = ERROR.ERROR_OK
            exec(message['body'])
        else:
            print("Unknown command")

        # TODO: Make radio_controller instance one of SlotController fields
        if DeviceConfig.INTERNAL_RADIO_AVAILABLE:
            radio_controller = RadioController()
            radio_controller.start_rx()

    def change_signalization(self, value):
        self.signalization_pin.value(value)

    def check_task_parameters(self, avaliable_param_fields, parameters_dict):
        if SlotCommunicationProtocol.SLOT_INTERNAL_DELAY in avaliable_param_fields:
            internal_delay = parameters_dict[SlotCommunicationProtocol.SLOT_INTERNAL_DELAY]
        else:
            internal_delay = 100
        if SlotCommunicationProtocol.SLOT_TASK_REPEAT_COUNT in avaliable_param_fields:
            task_timeout = 0
            rep_count = parameters_dict[SlotCommunicationProtocol.SLOT_TASK_REPEAT_COUNT]
        elif SlotCommunicationProtocol.SLOT_TASK_TIMEOUT_VALUE in avaliable_param_fields:
            rep_count = 0
            task_timeout = parameters_dict[SlotCommunicationProtocol.SLOT_TASK_TIMEOUT_VALUE]
        else:
            rep_count = 0
        if SlotCommunicationProtocol.SLOT_TASK_START_TIMEOUT in avaliable_param_fields:
            init_timeout = parameters_dict[SlotCommunicationProtocol.SLOT_TASK_START_TIMEOUT]
        else:
            init_timeout = 0
        return init_timeout, internal_delay, rep_count, task_timeout

    def construct_task_routine(self, action, parameters):
        start_delay = parameters[0]
        internal_delay = parameters[1]
        rep_count = parameters[2]
        timeout = parameters[3]

        print(f"Creating task with following parameters\n \
        Task params : {rep_count=}, {internal_delay=}, {timeout=}, {start_delay=}")

        async def action_wrapper():

            try:
                print("Task started")
                await uasyncio.sleep_ms(start_delay)

                if rep_count > 0:
                    for rep in range(rep_count):
                        await action()
                        await uasyncio.sleep_ms(internal_delay)

                elif timeout > 0:
                    start_time = ticks_ms()
                    while ticks_ms() - start_time < timeout:
                        action_start_time = ticks_ms()
                        await action()
                        #                         print("Action time : {} ms".format(ticks_ms()-action_start_time))
                        await uasyncio.sleep_ms(internal_delay)

            except uasyncio.CancelledError:
                print('Trapped cancelled error.')
                raise SlotTaskInterruptedException
            finally:  # Usual way to do cleanup
                print('Task Ended. Switching radio back to default state')
                if DeviceConfig.INTERNAL_RADIO_AVAILABLE:
                    self.switch_radio_controller_to_defaults()

        return action_wrapper

    @staticmethod
    def switch_radio_controller_to_defaults():
        radio_controller = RadioController()
        radio_controller.full_clear_fifo()
        radio_controller.clear_driver_interrupts()
        radio_controller.start_rx()
        print("Controller state : {}".format(radio_controller.get_tranciever_state()))

    @staticmethod
    def get_station_data_transport_info():
        if DeviceConfig.INTERNAL_RADIO_AVAILABLE:
            transporter = RadioController()
            src_addr = DeviceConfig.RADIO_MODULE_DEFAULT_AX25_ADDRESS
            dst_addr = DeviceConfig.STATION_EMITATOR_DEFAULT_ADDRESS
        else:
            transporter = CANController()
            src_addr = DeviceConfig.INTERNAL_SLOT_ADDRESS
            dst_addr = SlotCanAddressTable.RADIO_MODULE_SLOT_ADDRESS
        return transporter, src_addr, dst_addr

    def transfer_message(self, message, dir: int = 0):
        if dir == RADIO_CHANNEL_DIRECTION:
            if not DeviceConfig.INTERNAL_RADIO_AVAILABLE:
                print("Can't transfer this message")
                return -1

            transporter = RadioController()
            transporter.send_message(src_addr=DeviceConfig.RADIO_MODULE_DEFAULT_AX25_ADDRESS,
                                     dst_addr=DeviceConfig.STATION_EMITATOR_DEFAULT_ADDRESS,
                                     payload=message)
        elif dir == CAN_CHANNEL_DIRECTION:
            print("Resend message via CAN")
            can = CANController()
            uasyncio.create_task(
                self.send_response(message[SlotCommunicationProtocol.SLOT_COMMAND_KEY], 0))
            dst_addr = message.pop(SlotCommunicationProtocol.SLOT_ADDRESS_KEY)
            print(dst_addr)
            print(self._slot_addr)
            # re-dumping is a bad idea and result in loss of operation time
            # add can-message builder
            can.send_message(src_addr=self._slot_addr, dst_addr=dst_addr, payload=message)

    async def send_response(self, exec_cmd, response_code):
        print(f"Sending slot response for {exec_cmd} cmd")
        payload = {
            SlotCommunicationProtocol.STATION_TIMESTAMP_KEY: ticks_ms(),
            SlotCommunicationProtocol.SLOT_COMMAND_KEY: exec_cmd,
            SlotCommunicationProtocol.SLOT_RESPONSE_KEY: response_code
        }
        transporter, src_addr, dst_addr = self.get_station_data_transport_info()
        await transporter.send_message(src_addr, dst_addr, payload)

    async def send_signalization_state(self):
        print("Sending signalization state")
        payload = {
            SlotCommunicationProtocol.STATION_TIMESTAMP_KEY: ticks_ms(),
            SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_SIGNALIZATION_STATE,
            SlotCommunicationProtocol.STATE_VALUE: self.signalization_pin.value()
        }
        transporter, src_addr, dst_addr = self.get_station_data_transport_info()
        await transporter.send_message(src_addr, dst_addr, payload)

    async def send_slot_telemetry(self):
        print("Sending slot telemetry")
        payload = TEST_SLOT_TELEMETRY
        payload["Temp"] = random.uniform(20, 30)
        payload["VSolar"] = random.uniform(2, 5)
        print(f"Telemetry :\n{payload}")
        transporter, src_addr, dst_addr = self.get_station_data_transport_info()
        await transporter.send_message(src_addr, dst_addr, payload)

    async def send_slot_state(self):
        print("Sending slot state")
        payload = {
            SlotCommunicationProtocol.STATION_TIMESTAMP_KEY: ticks_ms(),
            SlotCommunicationProtocol.SLOT_COMMAND_KEY: SlotCommunicationProtocol.SEND_STATE_CMD_VALUE,
            SlotCommunicationProtocol.STATE_VALUE: StateContainer.get_device_state()}
        transporter, src_addr, dst_addr = self.get_station_data_transport_info()
        await transporter.send_message(src_addr, dst_addr, payload)

    def slot_command_handler(self):
        while True:
            next_cmd = await command_queue.get()
            print("Message in bytes : {}".format(next_cmd))
            try:
                unpacked_message = unpack(next_cmd)
                print(f"Message successfully unpacked: {unpacked_message}")
                # TODO: Rebuild logic of "Transfering via message"
                if self.is_message_on_transfer(unpacked_message):
                    print("Transfering message")
                    self.transfer_message(unpacked_message, RADIO_CHANNEL_DIRECTION)
                elif int(unpacked_message[SlotCommunicationProtocol.SLOT_ADDRESS_KEY]) == self._slot_addr:
                    print("Message obtained : \n{}".format(unpacked_message))
                    self.handle_received_command(unpacked_message)
                else:
                    self.transfer_message(unpacked_message, CAN_CHANNEL_DIRECTION)

            except umsgpack.InsufficientDataException:
                print("Message unpack failed")
                res = -1
                uasyncio.create_task(
                    self.send_response(unpacked_message[SlotCommunicationProtocol.SLOT_COMMAND_KEY], res))
                continue

            except KeyError:
                print("Some of following fields might be lost or broken: command/address")
                res = -1
                uasyncio.create_task(
                    self.send_response(unpacked_message[SlotCommunicationProtocol.SLOT_COMMAND_KEY], res))
                continue

            except Exception as e:
                print(e)
                res = -1
                uasyncio.create_task(
                    self.send_response(unpacked_message[SlotCommunicationProtocol.SLOT_COMMAND_KEY], res))
                continue

    @staticmethod
    def is_message_on_transfer(unpacked_message):
        return unpacked_message[
            SlotCommunicationProtocol.SLOT_COMMAND_KEY] == SlotCommunicationProtocol.SEND_TELEMETRY_CMD_VALUE or \
            unpacked_message[
                SlotCommunicationProtocol.SLOT_COMMAND_KEY] == SlotCommunicationProtocol.SEND_STATE_CMD_VALUE

    def initialize(self):
        self.controller_cmd_queue_task_handler = uasyncio.create_task(self.slot_command_handler())
