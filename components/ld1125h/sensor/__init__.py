import esphome.codegen as cg
import esphome.config_validation as cv

# from esphome import automation
from esphome.components import uart
from esphome.components import sensor
from esphome.components import binary_sensor
from esphome.const import (
    CONF_ID,
    CONF_STATE,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_MOTION,
)

CODEOWNERS = ["@ekerstens"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor"]

ld1125h_ns = cg.esphome_ns.namespace("ld1125h")

LD1125hComponent = ld1125h_ns.class_("LD1125hComponent", cg.Component, uart.UARTDevice)

# Config Names
CONF_MOTION_TIMEOUT = "motion_timeout"
CONF_LOG_SENSOR_OUTPUT = "log_sensor_output"
CONF_DISTANCE_SENSOR = "distance"
CONF_MOTION_SENSOR = "motion"
CONF_MOVEMENT_SENSOR = "movement"
CONF_OCCUPANCY_SENSOR = "occupancy"

# LD1125H Sensor Configs
CONF_RMAX = "rmax"  # maximum detection range of the sensor
CONF_MTH1_MOV = "mth1_mov"  # signal sensitivity between 0-2.8 meters
CONF_MTH2_MOV = "mth2_mov"  # signal sensitivity between 2.8-8 meters
CONF_MTH3_MOV = "mth3_mov"  # signal sensitivity beyond 8 meters
CONF_MTH1_OCC = "mth1_occ"  # signal sensitivity between 0-2.8 meters
CONF_MTH2_OCC = "mth2_occ"  # signal sensitivity between 2.8-8 meters
CONF_MTH3_OCC = "mth3_occ"  # signal sensitivity beyond 8 meters
CONF_LOG_VERSION = "log_version"  # LOG version information
CONF_LOG_GET_ALL = "log_get_all"  # LOG version information
CONF_TS_ON = "ts_on"  # ???
CONF_TS_OFF = "ts_off"  # ???
CONF_OUTPUT_MODE = "output_mode"  # Set the sensor to either display output via serial (default 0), or via GPIO port on second header (1)
CONF_TEST_MODE = "test_mode"  # Set to 1 to output the signal strength (Default 0). Example: occ, dis=3.62, str=61.93


# Common Validators
motion_class_schema = binary_sensor.binary_sensor_schema(
    device_class=DEVICE_CLASS_MOTION
)
segment_threshold_sensitivity = cv.int_range(min=0, max=100)


# Config Schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LD1125hComponent),
        cv.Optional(CONF_LOG_SENSOR_OUTPUT, default=False): cv.boolean,
        cv.Optional(
            CONF_MOTION_TIMEOUT, default=1000
        ): cv.positive_time_period_milliseconds,
        # Sensor Configs
        cv.Optional(CONF_RMAX, default=6): cv.float_range(min=0, max=10),
        cv.Optional(CONF_MTH1_MOV, default=80): segment_threshold_sensitivity,
        cv.Optional(CONF_MTH2_MOV, default=50): segment_threshold_sensitivity,
        cv.Optional(CONF_MTH3_MOV, default=20): segment_threshold_sensitivity,
        cv.Optional(CONF_MTH1_OCC, default=60): segment_threshold_sensitivity,
        cv.Optional(CONF_MTH2_OCC, default=55): segment_threshold_sensitivity,
        cv.Optional(CONF_MTH3_OCC, default=20): segment_threshold_sensitivity,
        cv.Optional(CONF_LOG_VERSION, default=False): cv.boolean,
        cv.Optional(CONF_LOG_GET_ALL, default=False): cv.boolean,
        cv.Optional(CONF_TS_ON, default=60): cv.positive_int,
        cv.Optional(CONF_TS_OFF, default=15): cv.positive_int,
        cv.Optional(CONF_OUTPUT_MODE, default=0): cv.int_range(min=0, max=1),
        cv.Optional(CONF_TEST_MODE, default=0): cv.int_range(min=0, max=1),
        # Sub Sensor Configs
        cv.Optional(CONF_DISTANCE_SENSOR): sensor.sensor_schema(
            device_class=DEVICE_CLASS_DISTANCE,
            accuracy_decimals=2,
        ),
        cv.Optional(CONF_MOVEMENT_SENSOR): motion_class_schema,
        cv.Optional(CONF_OCCUPANCY_SENSOR): motion_class_schema,
        cv.Optional(CONF_MOTION_SENSOR): motion_class_schema,
    }
).extend(uart.UART_DEVICE_SCHEMA)

SUB_SENSOR_INIT = [
    (CONF_DISTANCE_SENSOR, "set_distance_sensor", sensor.new_sensor),
    (CONF_MOVEMENT_SENSOR, "set_movement_sensor", binary_sensor.new_binary_sensor),
    (CONF_OCCUPANCY_SENSOR, "set_occupancy_sensor", binary_sensor.new_binary_sensor),
    (CONF_MOTION_SENSOR, "set_motion_sensor", binary_sensor.new_binary_sensor),
]


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_log_sensor_output(config[CONF_LOG_SENSOR_OUTPUT]))
    cg.add(var.set_motion_timeout(config[CONF_MOTION_TIMEOUT]))
    cg.add(var.set_rmax(config[CONF_RMAX]))
    cg.add(var.set_mth1_mov(config[CONF_MTH1_MOV]))
    cg.add(var.set_mth2_mov(config[CONF_MTH2_MOV]))
    cg.add(var.set_mth3_mov(config[CONF_MTH3_MOV]))
    cg.add(var.set_mth1_occ(config[CONF_MTH1_OCC]))
    cg.add(var.set_mth2_occ(config[CONF_MTH2_OCC]))
    cg.add(var.set_mth3_occ(config[CONF_MTH3_OCC]))
    cg.add(var.set_log_version(config[CONF_LOG_VERSION]))
    cg.add(var.set_log_get_all(config[CONF_LOG_GET_ALL]))
    cg.add(var.set_ts_on(config[CONF_TS_ON]))
    cg.add(var.set_ts_off(config[CONF_TS_OFF]))
    cg.add(var.set_output_mode(config[CONF_OUTPUT_MODE]))
    cg.add(var.set_test_mode(config[CONF_TEST_MODE]))

    async def add_sub_sensor(config_name, init_function, new_sensor_funtion):
        sens = await new_sensor_funtion(config[config_name])
        cg.add(getattr(var, init_function)(sens))

    for config_name, init_function, new_sensor_function in SUB_SENSOR_INIT:
        await add_sub_sensor(config_name, init_function, new_sensor_function)
