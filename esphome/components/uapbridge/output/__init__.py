from esphome.components import output
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from .. import uapbridge_ns, UAPBridge, CONF_UAPBRIDGE_ID

DEPENDENCIES = ["uapbridge"]

UAPBridgeBinaryOutput = uapbridge_ns.class_("UAPBridgeBinaryOutput", output.BinaryOutput, cg.Component)

CONFIG_SCHEMA = output.BINARY_OUTPUT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(UAPBridgeBinaryOutput),
        cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)

    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    cg.add(var.set_uapbridge_parent(parent))
