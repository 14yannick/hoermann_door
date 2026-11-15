import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import ICON_FAN, ICON_LIGHTBULB
from .. import uapbridge_ns, CONF_UAPBRIDGE_ID, UAPBridge

DEPENDENCIES = ["uapbridge"]

UAPBridgeSwitchVent = uapbridge_ns.class_("UAPBridgeSwitchVent", switch.Switch, cg.Component)
UAPBridgeSwitchLight = uapbridge_ns.class_("UAPBridgeSwitchLight", switch.Switch, cg.Component)

CONF_SWITCH_VENT = "venting_switch"
CONF_SWITCH_LIGHT = "light_switch"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_UAPBRIDGE_ID): cv.use_id(UAPBridge),
        cv.Optional(CONF_SWITCH_VENT): switch.switch_schema(
            UAPBridgeSwitchVent, icon=ICON_FAN
        ),
        cv.Optional(CONF_SWITCH_LIGHT): switch.switch_schema(
            UAPBridgeSwitchLight, icon=ICON_LIGHTBULB
        ),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_UAPBRIDGE_ID])
    if conf := config.get(CONF_SWITCH_VENT):
        vent_sw = await switch.new_switch(config[CONF_SWITCH_VENT])
        await cg.register_component(vent_sw, config[CONF_SWITCH_VENT])
        cg.add(vent_sw.set_uapbridge_parent(parent))
    if conf := config.get(CONF_SWITCH_LIGHT):
        light_sw = await switch.new_switch(config[CONF_SWITCH_LIGHT])
        await cg.register_component(light_sw, config[CONF_SWITCH_LIGHT])
        cg.add(light_sw.set_uapbridge_parent(parent))