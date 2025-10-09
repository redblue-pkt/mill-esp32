import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, i2c, network, sensor, switch, text_sensor, globals, button
from esphome.const import CONF_ID, CONF_SENSOR

DEPENDENCIES = ["i2c", "climate", "network" , "sensor", "switch", "text_sensor", "globals", "button"]

cms_ns = cg.esphome_ns.namespace("mill")
MillClimate = cms_ns.class_("MillClimate", climate.Climate, cg.PollingComponent, i2c.I2CDevice)

CONF_HEAT_SWITCH = "heat_switch"
CONF_STATUS_TEXT = "status_text"
CONF_TARGET_TEMP_SENSOR = "target_temperature_sensor"
CONF_CURRENT_TEMP_SENSOR = "current_temperature_sensor"
# to del ?
CONF_TGT_TEMP_GLOBAL = "target_temperature_global"
CONF_CUR_TEMP_GLOBAL = "current_temperature_global"
CONF_RESET_BUTTON   = "reset_button"
CONF_FACTORY_BUTTON = "factory_button"
CONF_STATUS_VALUE_GLOBAL = "status_value_global"

#CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
BASE_SCHEMA = climate.climate_schema(MillClimate)

CONFIG_SCHEMA = (
    BASE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MillClimate),
    cv.Required(CONF_SENSOR): cv.use_id(sensor.Sensor),              # aktualna temp (NTC)
    cv.Required(CONF_HEAT_SWITCH): cv.use_id(switch.Switch),         # przekaźnik GPIO4
    cv.Optional(CONF_STATUS_TEXT): cv.use_id(text_sensor.TextSensor),# opcjonalny text_sensor z HEAT/IDLE/OFF
    cv.Optional(CONF_TARGET_TEMP_SENSOR): cv.use_id(sensor.Sensor),  # opcjonalny „wanted_temperature”
    cv.Optional(CONF_CURRENT_TEMP_SENSOR): cv.use_id(sensor.Sensor), # opcjonalny „now_temperature”

    # to del ?
    cv.Optional(CONF_TGT_TEMP_GLOBAL): cv.use_id(globals.GlobalsComponent),
    cv.Optional(CONF_CUR_TEMP_GLOBAL): cv.use_id(globals.GlobalsComponent),
    cv.Optional(CONF_RESET_BUTTON): cv.use_id(button.Button),
    cv.Optional(CONF_FACTORY_BUTTON): cv.use_id(button.Button),
    cv.Optional(CONF_STATUS_VALUE_GLOBAL): cv.use_id(globals.GlobalsComponent),
    })
    .extend(cv.polling_component_schema("50ms"))
    .extend(i2c.i2c_device_schema(0x50))
)
#}).extend(cv.polling_component_schema("1s")).extend(i2c.i2c_device_schema(0x50))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await i2c.register_i2c_device(var, config)

    sens = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_temperature_sensor(sens))

    hs = await cg.get_variable(config[CONF_HEAT_SWITCH])
    cg.add(var.set_heat_switch(hs))

    if CONF_STATUS_TEXT in config:
        st = await cg.get_variable(config[CONF_STATUS_TEXT])
        cg.add(var.set_status_text(st))
    if CONF_TARGET_TEMP_SENSOR in config:
        tts = await cg.get_variable(config[CONF_TARGET_TEMP_SENSOR])
        cg.add(var.set_target_temperature_sensor(tts))
    if CONF_CURRENT_TEMP_SENSOR in config:
        cts = await cg.get_variable(config[CONF_CURRENT_TEMP_SENSOR])
        cg.add(var.set_current_temperature_sensor(cts))

    # to del ?
    if CONF_TGT_TEMP_GLOBAL in config:
        tg = await cg.get_variable(config[CONF_TGT_TEMP_GLOBAL])
        cg.add(var.set_target_temp_global(tg))
    if CONF_CUR_TEMP_GLOBAL in config:
        cg_ = await cg.get_variable(config[CONF_CUR_TEMP_GLOBAL])
        cg.add(var.set_current_temp_global(cg_))

    if CONF_RESET_BUTTON in config:
        rb = await cg.get_variable(config[CONF_RESET_BUTTON])
        cg.add(var.set_reset_button(rb))

    if CONF_FACTORY_BUTTON in config:
        fb = await cg.get_variable(config[CONF_FACTORY_BUTTON])
        cg.add(var.set_factory_button(fb))

    if CONF_STATUS_VALUE_GLOBAL in config:
        cg_ = await cg.get_variable(config[CONF_STATUS_VALUE_GLOBAL])
        cg.add(var.set_status_value_global(cg_))
