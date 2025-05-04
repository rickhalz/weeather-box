idf_component_register(SRCS "wifi_sta.c"
	INCLUDE_DIRS "include" REQUIRES esp_event esp_netif mqtt nvs_flash esp_wifi)
