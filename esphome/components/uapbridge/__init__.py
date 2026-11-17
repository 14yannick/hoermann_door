import inspect

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome import automation, pins
from esphome.const import CONF_ID, CONF_TRIGGER_ID

DEPENDENCIES = ["uart"]
MULTI_CONF = True
CONF_RTS_PIN = "rts_pin"
CONF_AUTO_CORRECTION = "auto_correction"

# Create UAPBridge namespace
uapbridge_ns = cg.esphome_ns.namespace("uapbridge")
UAPBridge = uapbridge_ns.class_("UAPBridge", cg.Component, uart.UARTDevice)
DoorEvent = uapbridge_ns.enum("DoorEvent", is_class=True)
DoorEventTrigger = uapbridge_ns.class_("DoorEventTrigger", automation.Trigger.template())

# Automation triggers on the bridge: config key -> door event.
# They only fire for real changes between two known door states (not at boot / bus recovery).
DOOR_EVENT_TRIGGERS = {
    "on_opening": DoorEvent.OPENING,
    "on_open": DoorEvent.OPENED,
    "on_closing": DoorEvent.CLOSING,
    "on_closed": DoorEvent.CLOSED,
    "on_stopped": DoorEvent.STOPPED,
    "on_venting": DoorEvent.VENTING,
    "on_error": DoorEvent.ERROR_ON,
    "on_prewarn": DoorEvent.PREWARN_ON,
}

CONF_UAPBRIDGE_ID = "uapbridge_id"

# Oldest ESPHome version this component is tested with; older versions get a clear error.
validate_min_esphome_version = cv.require_esphome_version(2025, 11, 0)

CONFIG_SCHEMA_BASE = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(UAPBridge),
        cv.Optional(CONF_RTS_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_AUTO_CORRECTION): cv.boolean,
        **{
            cv.Optional(key): automation.validate_automation(
                {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(DoorEventTrigger)}
            )
            for key in DOOR_EVENT_TRIGGERS
        },
    }
).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)

def CONFIG_SCHEMA(conf):
    if conf:
        raise cv.Invalid(
            "Invalid operation to use baseclass in config\n"
            "either use uapbridge_esp or uapbridge_pic16"
        )


async def to_code_base(var, config):
    await cg.register_component(var, config)

    if CONF_RTS_PIN in config:
        rts_pin = await cg.gpio_pin_expression(config[CONF_RTS_PIN])
        cg.add(var.set_rts_pin(rts_pin))

    if CONF_AUTO_CORRECTION in config:
        cg.add(var.set_auto_correction(config[CONF_AUTO_CORRECTION]))

    for key, event in DOOR_EVENT_TRIGGERS.items():
        for conf in config.get(key, []):
            trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var, event)
            await automation.build_automation(trigger, [], conf)


# --- Actions --------------------------------------------------------------------------------
# uapbridge.<name>: <bridge id>  (the id may be omitted if there is only one bridge)
UAPBRIDGE_ACTIONS = {
    "uapbridge.open": "OpenAction",
    "uapbridge.close": "CloseAction",
    "uapbridge.stop": "StopAction",
    "uapbridge.venting": "VentingAction",
    "uapbridge.impulse": "ImpulseAction",
    "uapbridge.toggle_light": "ToggleLightAction",
    "uapbridge.light_on": "LightOnAction",
    "uapbridge.light_off": "LightOffAction",
}

UAPBRIDGE_ACTION_SCHEMA = automation.maybe_simple_id(
    {cv.GenerateID(): cv.use_id(UAPBridge)}
)

# ESPHome >= 2026.3 expects synchronous= (warns otherwise); older versions reject the argument.
_REGISTER_ACTION_KWARGS = (
    {"synchronous": True}
    if "synchronous" in inspect.signature(automation.register_action).parameters
    else {}
)


async def uapbridge_action_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


for _name, _cls in UAPBRIDGE_ACTIONS.items():
    automation.register_action(
        _name,
        uapbridge_ns.class_(_cls, automation.Action),
        UAPBRIDGE_ACTION_SCHEMA,
        **_REGISTER_ACTION_KWARGS,
    )(uapbridge_action_to_code)
