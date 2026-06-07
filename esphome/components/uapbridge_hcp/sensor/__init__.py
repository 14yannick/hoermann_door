from esphome.components import sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import UNIT_PERCENT, ICON_PERCENT, STATE_CLASS_MEASUREMENT

from .. import uapbridge_hcp_ns, UAPBridge_hcp, CONF_UAPBRIDGE_HCP_ID

DEPENDENCIES = ["uapbridge_hcp"]

UAPBridgeHCPSensor = uapbridge_hcp_ns.class_(
    "UAPBridgeHCPSensor", sensor.Sensor, cg.Component
)

CONFIG_SCHEMA = sensor.sensor_schema(
    UAPBridgeHCPSensor,
    unit_of_measurement=UNIT_PERCENT,
    icon=ICON_PERCENT,
    state_class=STATE_CLASS_MEASUREMENT,
    accuracy_decimals=0,
).extend(
    {
        cv.GenerateID(CONF_UAPBRIDGE_HCP_ID): cv.use_id(UAPBridge_hcp),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[sensor.CONF_ID])
    parent = await cg.get_variable(config[CONF_UAPBRIDGE_HCP_ID])
    cg.add(var.set_parent(parent))
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)