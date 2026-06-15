from esphome.components import binary_sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID, UAPBridge
from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_SAFETY,
    ENTITY_CATEGORY_DIAGNOSTIC,
)

DEPENDENCIES = ["uapbridge"]

UAPBridgeCommunication = uapbridge_ns.class_("UAPBridgeCommunication", binary_sensor.BinarySensor, cg.Component)
UAPBridgeRelaySensor = uapbridge_ns.class_("UAPBridgeRelaySensor", binary_sensor.BinarySensor, cg.Component)
UAPBridgeErrorSensor = uapbridge_ns.class_("UAPBridgeErrorSensor", binary_sensor.BinarySensor, cg.Component)
UAPBridgePrewarnSensor = uapbridge_ns.class_("UAPBridgePrewarnSensor", binary_sensor.BinarySensor, cg.Component)
UAPBridgeIsConnected = uapbridge_ns.class_("UAPBridgeIsConnected", binary_sensor.BinarySensor, cg.Component)

CONF_PIC16_COM = "pic16_com"
CONF_RELAY_STATE = "relay_state"
CONF_ERROR_STATE = "error_state"
CONF_PREWARN_STATE = "prewarn_state"
CONF_IS_CONNECTED = "is_connected"


def _require_e3_protocol(value):
    """Validates that at least one E3 protocol component is loaded."""
    from esphome.core import CORE
    raw = CORE.raw_config or {}
    if "uapbridge_esp" not in raw and "uapbridge_pic16" not in raw:
        raise cv.Invalid(
            "This sensor requires 'uapbridge_esp' or 'uapbridge_pic16' (E3 protocol only)"
        )
    return value


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
        cv.Optional(CONF_PIC16_COM): binary_sensor.binary_sensor_schema(
            UAPBridgeCommunication,
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ).add_extra(cv.requires_component("uapbridge_pic16")),
        cv.Optional(CONF_RELAY_STATE): binary_sensor.binary_sensor_schema(
            UAPBridgeRelaySensor,
        ),
        cv.Optional(CONF_ERROR_STATE): binary_sensor.binary_sensor_schema(
            UAPBridgeErrorSensor,
            device_class=DEVICE_CLASS_PROBLEM,
        ).add_extra(_require_e3_protocol),
        cv.Optional(CONF_PREWARN_STATE): binary_sensor.binary_sensor_schema(
            UAPBridgePrewarnSensor,
            device_class=DEVICE_CLASS_SAFETY,
        ).add_extra(_require_e3_protocol),
        cv.Optional(CONF_IS_CONNECTED): binary_sensor.binary_sensor_schema(
            UAPBridgeIsConnected,
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])

    if conf := config.get(CONF_PIC16_COM):
        comm_sens = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(comm_sens, conf)
        cg.add(comm_sens.set_uapbridge_pic16_parent(parent))

    if conf := config.get(CONF_RELAY_STATE):
        relay_sens = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(relay_sens, conf)
        cg.add(relay_sens.set_uapbridge_parent(parent))

    if conf := config.get(CONF_ERROR_STATE):
        error_sens = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(error_sens, conf)
        cg.add(error_sens.set_uapbridge_parent(parent))

    if conf := config.get(CONF_PREWARN_STATE):
        prewarn_sens = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(prewarn_sens, conf)
        cg.add(prewarn_sens.set_uapbridge_parent(parent))

    if conf := config.get(CONF_IS_CONNECTED):
        is_conn_sens = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(is_conn_sens, conf)
        cg.add(is_conn_sens.set_uapbridge_parent(parent))
