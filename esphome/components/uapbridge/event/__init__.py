import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import event
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID, UAPBridge

DEPENDENCIES = ["uapbridge"]

UAPBridgeEvent = uapbridge_ns.class_("UAPBridgeEvent", event.Event, cg.Component)

# Fired only for real changes between two known door states (not at boot / bus recovery).
EVENT_TYPES = [
    "opening",
    "opened",
    "closing",
    "closed",
    "stopped",
    "venting",
    "error",
    "prewarn",
]

CONFIG_SCHEMA = (
    event.event_schema(UAPBridgeEvent, icon="mdi:garage")
    .extend(
        {
            cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await event.new_event(config, event_types=EVENT_TYPES)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    cg.add(var.set_uapbridge_parent(parent))
