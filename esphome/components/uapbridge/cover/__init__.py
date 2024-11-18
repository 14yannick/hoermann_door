from esphome.components import cover
import esphome.config_validation as cv
import esphome.codegen as cg
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID, UAPBridge
from esphome.const import (
  CONF_ID
)

DEPENDENCIES = ["uapbridge"]

UAPBridgeCover = uapbridge_ns.class_("UAPBridgeCover", cover.Cover, cg.Component)

# Use COVER_SCHEMA instead of cover_schema
CONFIG_SCHEMA = (
   cover.COVER_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(UAPBridgeCover),
            cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    # Create a new Pvariable for UAPBridgeCover
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    # Retrieve the parent variable
    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    cg.add(var.set_uapbridge_parent(parent))

