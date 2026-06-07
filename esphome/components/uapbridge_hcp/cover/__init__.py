from esphome.components import cover
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID

from .. import uapbridge_hcp_ns, UAPBridge_hcp, CONF_UAPBRIDGE_HCP_ID

DEPENDENCIES = ["uapbridge_hcp"]

UAPBridgeHCPCover = uapbridge_hcp_ns.class_(
    "UAPBridgeHCPCover", cover.Cover, cg.Component
)

CONFIG_SCHEMA = cover.cover_schema(UAPBridgeHCPCover).extend(
    {
        cv.GenerateID(CONF_UAPBRIDGE_HCP_ID): cv.use_id(UAPBridge_hcp),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)

    parent = await cg.get_variable(config[CONF_UAPBRIDGE_HCP_ID])
    cg.add(var.set_parent(parent))