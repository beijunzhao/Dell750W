idf_component_register(
    SRCS
        "main.cpp"
        "power_control.cpp"
        "adc_sampler.cpp"
        "pmbus.cpp"
        "ble_server.cpp"
    PRIV_REQUIRES
        nvs_flash
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_ledc
        esp_adc
        bt
    INCLUDE_DIRS
        "."
)

# ble_server.cpp 使用 designated initializers 初始化 NimBLE 结构体，
# 编译器 -Wmissing-field-initializers 会误报，此处禁用
idf_component_get_property(main_component main COMPONENT_LIB)
target_compile_options(${main_component} PRIVATE -Wno-missing-field-initializers)
