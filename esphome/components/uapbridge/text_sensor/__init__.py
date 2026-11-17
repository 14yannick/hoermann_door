from esphome.components import text_sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_DISABLED_BY_DEFAULT, CONF_TYPE, ENTITY_CATEGORY_DIAGNOSTIC
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID, UAPBridge

DEPENDENCIES = ["uapbridge"]

UAPBridgeTextSensor = uapbridge_ns.class_("UAPBridgeTextSensor", text_sensor.TextSensor, cg.Component)
UAPBridgeRawStatusTextSensor = uapbridge_ns.class_(
    "UAPBridgeRawStatusTextSensor", text_sensor.TextSensor, cg.Component
)

TYPE_STATE = "state"
TYPE_RAW_STATUS = "raw_status"

PARENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
    }
)

# Door state text (default, unchanged for existing configs)
STATE_SCHEMA = (
    text_sensor.text_sensor_schema(UAPBridgeTextSensor, icon="mdi:garage")
    .extend(PARENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

# Diagnostic: raw status word as hex; hidden in Home Assistant unless enabled
RAW_STATUS_SCHEMA = (
    text_sensor.text_sensor_schema(
        UAPBridgeRawStatusTextSensor,
        icon="mdi:hexadecimal",
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )
    .extend({cv.Optional(CONF_DISABLED_BY_DEFAULT, default=True): cv.boolean})
    .extend(PARENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


def CONFIG_SCHEMA(config):
    # Without `type` the sensor stays the door state text; configs that don't set `type`
    # validate exactly as before (no `type` key is added to them).
    if not isinstance(config, dict) or CONF_TYPE not in config:
        return STATE_SCHEMA(config)
    config = dict(config)
    sensor_type = cv.one_of(TYPE_STATE, TYPE_RAW_STATUS, lower=True)(config.pop(CONF_TYPE))
    schema = RAW_STATUS_SCHEMA if sensor_type == TYPE_RAW_STATUS else STATE_SCHEMA
    config = schema(config)
    config[CONF_TYPE] = sensor_type
    return config


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    cg.add(var.set_uapbridge_parent(parent))
