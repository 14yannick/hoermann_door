from esphome.components import cover
import esphome.config_validation as cv
import esphome.codegen as cg
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID, UAPBridge
from esphome.const import (
  CONF_ID,
  DEVICE_CLASS_GARAGE,
)

DEPENDENCIES = ["uapbridge"]

UAPBridgeCover = uapbridge_ns.class_("UAPBridgeCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cv.All(
  cover.cover_schema(UAPBridgeCover, device_class=DEVICE_CLASS_GARAGE).extend({
    cv.GenerateID(): cv.declare_id(UAPBridgeCover),
    cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
  }),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    cg.add(var.set_uapbridge_parent(parent))

