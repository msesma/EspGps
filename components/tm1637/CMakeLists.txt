set(component_srcs "tm1637.c")

idf_component_register(SRCS "${component_srcs}"
                       REQUIRES esp_driver_gpio
                       INCLUDE_DIRS ".")
