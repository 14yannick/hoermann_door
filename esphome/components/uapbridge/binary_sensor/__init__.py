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
UAPBridgeGotValidBroadcast = uapbridge_ns.class_("UAPBridgeGotValidBroadcast", binary_sensor.BinarySensor, cg.Component)

CONF_PIC16_COM = "pic16_com"
CONF_RELAY_STATE = "relay_state"
CONF_ERROR_STATE = "error_state"
CONF_PREWARN_STATE = "prewarn_state"
CONF_GOT_VALID_BROADCAST = "got_valid_broadcast"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
        cv.Optional(CONF_PIC16_COM): binary_sensor.binary_sensor_schema(
            UAPBridgeCommunication
        ).extend({
            cv.Optional("device_class", default=DEVICE_CLASS_CONNECTIVITY): cv.string,
            cv.Optional("entity_category", default=ENTITY_CATEGORY_DIAGNOSTIC): cv.entity_category,
        }).add_extra(cv.requires_component("uapbridge_pic16")),
        cv.Optional(CONF_RELAY_STATE): binary_sensor.binary_sensor_schema(
            UAPBridgeRelaySensor
        ),
        cv.Optional(CONF_ERROR_STATE): binary_sensor.binary_sensor_schema(
            UAPBridgeErrorSensor
        ).extend({
            cv.Optional("device_class", default=DEVICE_CLASS_PROBLEM): cv.string,
        }),
        cv.Optional(CONF_PREWARN_STATE): binary_sensor.binary_sensor_schema(
            UAPBridgePrewarnSensor
        ).extend({
            cv.Optional("device_class", default=DEVICE_CLASS_SAFETY): cv.string,
        }),
        cv.Optional(CONF_GOT_VALID_BROADCAST): binary_sensor.binary_sensor_schema(
            UAPBridgeGotValidBroadcast
        ).extend({
            cv.Optional("device_class", default=DEVICE_CLASS_CONNECTIVITY): cv.string,
            cv.Optional("entity_category", default=ENTITY_CATEGORY_DIAGNOSTIC): cv.entity_category,
        }),
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

    if conf := config.get(CONF_GOT_VALID_BROADCAST):
        got_valid_broadcast_sens = await binary_sensor.new_binary_sensor(conf)
        await cg.register_component(got_valid_broadcast_sens, conf)
        cg.add(got_valid_broadcast_sens.set_uapbridge_parent(parent))
