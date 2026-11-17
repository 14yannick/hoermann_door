import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_DISABLED_BY_DEFAULT,
    CONF_TYPE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_TOTAL_INCREASING,
)
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID
from ...uapbridge_esp import UAPBridge_esp

# Bus counters exist only in the ESP bridge (with PIC16 the bus is handled by the PIC).
DEPENDENCIES = ["uapbridge_esp"]

UAPBridgeCounterSensor = uapbridge_ns.class_(
    "UAPBridgeCounterSensor", sensor.Sensor, cg.PollingComponent
)
CounterType = uapbridge_ns.enum("CounterType", is_class=True)

COUNTER_TYPES = {
    "broadcasts_received": CounterType.BROADCASTS_RECEIVED,
    "polls_answered": CounterType.POLLS_ANSWERED,
    "commands_sent": CounterType.COMMANDS_SENT,
    "commands_dropped": CounterType.COMMANDS_DROPPED,
}

# One entry per counter, each with its own update_interval (default 300s).
# Diagnostic and hidden in Home Assistant unless enabled; RAM only, reset on reboot.
CONFIG_SCHEMA = (
    sensor.sensor_schema(
        UAPBridgeCounterSensor,
        icon="mdi:counter",
        accuracy_decimals=0,
        state_class=STATE_CLASS_TOTAL_INCREASING,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )
    .extend(
        {
            cv.Optional(CONF_DISABLED_BY_DEFAULT, default=True): cv.boolean,
            cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge_esp),
            cv.Required(CONF_TYPE): cv.enum(COUNTER_TYPES, lower=True),
        }
    )
    .extend(cv.polling_component_schema("300s"))
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    cg.add(var.set_uapbridge_parent(parent))
    cg.add(var.set_counter_type(config[CONF_TYPE]))
