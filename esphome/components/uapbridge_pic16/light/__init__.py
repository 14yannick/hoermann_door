from esphome.components import light, output
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_OUTPUT_ID, CONF_OUTPUT 
from .. import uapbridge_pic16_ns, CONF_UAPBRIDGE_PIC16_ID, UAPBridge_pic16

DEPENDENCIES = ["uapbridge_pic16"]

UAPBridge_pic16Light = uapbridge_pic16_ns.class_("UAPBridge_pic16Light", light.LightOutput, cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_UAPBRIDGE_PIC16_ID): cv.use_id(UAPBridge_pic16),
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(UAPBridge_pic16Light),
            cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
        }
    )
    .extend(cv.ENTITY_BASE_SCHEMA)
    .extend(light.BINARY_LIGHT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    light_output_var = cg.new_Pvariable(config[CONF_OUTPUT_ID])

    out = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(light_output_var.set_output(out))

    parent = await cg.get_variable(config[CONF_UAPBRIDGE_PIC16_ID])
    cg.add(light_output_var.set_uapbridge_pic16_parent(parent))

    await cg.register_component(light_output_var, config)
    await light.register_light(light_output_var, config)