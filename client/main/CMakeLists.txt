idf_build_get_property(TARGET_ID IDF_TARGET)

# Pick required components
if(TARGET_ID STREQUAL "linux")
    set(requires
        esp_event
        esp_stubs
        protocol_examples_common
        nvs_flash
        driver
    )
else()
    set(requires
        protocol_examples_common
        nvs_flash
        driver
    )
endif()

# Choose IPv4 vs IPv6 source (you can ignore this if you only use v4)
if(CONFIG_EXAMPLE_IPV4)
    set(tcp_client_src tcp_client_v4.c)
else()
    set(tcp_client_src tcp_client_v6.c)
endif()

idf_component_register(
    SRCS
        tcp_client_main.c
        tcp_client_v4.c
        lcd20x4_driver.c
    INCLUDE_DIRS "."
    REQUIRES
        protocol_examples_common
        nvs_flash
        driver
        esp_netif          # <<-- add this
)

