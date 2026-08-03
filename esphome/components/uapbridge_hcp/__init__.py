import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID
from ..uapbridge import to_code_base, CONFIG_SCHEMA_BASE, UAPBridge

AUTO_LOAD = ["uapbridge"]
MULTI_CONF = True

CONF_UAPBRIDGE_HCP_ID = "uapbridge_hcp_id"
CONF_HIGH_FREQ_LOOP = "high_frequency_loop"

uapbridge_hcp_ns = cg.esphome_ns.namespace("uapbridge_hcp")
UAPBridge_hcp = uapbridge_hcp_ns.class_(
    "UAPBridge_hcp", UAPBridge
)

CONFIG_SCHEMA = cv.All(
    CONFIG_SCHEMA_BASE.extend(
        {
            cv.GenerateID(): cv.declare_id(UAPBridge_hcp),
            cv.Optional(CONF_HIGH_FREQ_LOOP, default=True): cv.boolean,
        }
    )
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "uapbridge_hcp",
    baud_rate=57600,
    require_rx=True,
    require_tx=True,
    parity="EVEN",
    stop_bits=1,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await to_code_base(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_high_frequency_loop(config[CONF_HIGH_FREQ_LOOP]))
