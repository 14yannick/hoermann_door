import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from ..uapbridge import to_code_base, CONFIG_SCHEMA_BASE, UAPBridge
from esphome.const import CONF_ID

AUTO_LOAD = ["uapbridge"]
MULTI_CONF = True
CONF_PIC16_VERSION = "pic16_version"

# Create UAPBridge_pic16 namespace
uapbridge_pic16_ns = cg.esphome_ns.namespace("uapbridge_pic16")
UAPBridge_pic16 = uapbridge_pic16_ns.class_("UAPBridge_pic16", UAPBridge)

CONFIG_SCHEMA = cv.All(
    CONFIG_SCHEMA_BASE.extend(
        {
            cv.GenerateID(): cv.declare_id(UAPBridge_pic16),
            cv.Optional(CONF_PIC16_VERSION, default=1): cv.int_,
        }
    )
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "uapbridge_uart",
    require_tx=True,
    require_rx=True,
    baud_rate=19200
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await to_code_base(var, config)
    await uart.register_uart_device(var, config)
    if CONF_PIC16_VERSION in config:
        cg.add(var.set_pic16_version(config[CONF_PIC16_VERSION]))
