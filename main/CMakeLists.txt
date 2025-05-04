idf_component_register(SRCS "weather-box.c"
	INCLUDE_DIRS "." REQUIRES sh1106 mq135 shtc3 wifi_sta nvs_flash)
