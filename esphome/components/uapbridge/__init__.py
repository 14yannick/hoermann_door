import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome import pins

DEPENDENCIES = ["uart"]
MULTI_CONF = True
CONF_RTS_PIN = "rts_pin"

# Create UAPBridge namespace
uapbridge_ns = cg.esphome_ns.namespace("uapbridge")
UAPBridge = uapbridge_ns.class_("UAPBridge", cg.Component, uart.UARTDevice)

CONF_UAPBRIDGE_ID = "uapbridge_id"

CONFIG_SCHEMA_BASE = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(UAPBridge),
        cv.Optional(CONF_RTS_PIN): pins.gpio_output_pin_schema,
    }
).extend(uart.UART_DEVICE_SCHEMA).extend(cv.COMPONENT_SCHEMA)

def CONFIG_SCHEMA(conf):
    if conf:
        raise cv.Invalid(
            "Invalid operation to use baseclass in config\n"
            "either use uapbridge_esp, uapbridge_pic16, or uapbridge_hcp"
        )


async def to_code_base(var, config):
    await cg.register_component(var, config)

    if CONF_RTS_PIN in config:
        rts_pin = await cg.gpio_pin_expression(config[CONF_RTS_PIN])
        cg.add(var.set_rts_pin(rts_pin))

