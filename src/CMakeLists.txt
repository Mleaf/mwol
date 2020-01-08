set(src_mwol
	main.c
	common.c
	config.c
	json.c
	mqtt_client.c
	wake.c
	cJSON.c
)

set(libs 
	pthread 
	m
	mosquitto)

ADD_DEFINITIONS(-O2 -Wall --std=gnu99 -D_GNU_SOURCE)

add_executable(mwol ${src_mwol})
target_link_libraries(mwol ${libs})

install(TARGETS mwol
		RUNTIME DESTINATION bin
)

