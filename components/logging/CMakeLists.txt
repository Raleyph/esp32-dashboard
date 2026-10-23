idf_component_register(
	SRCS
		"src/LogCapture.cpp"

	INCLUDE_DIRS
		"include"

	REQUIRES
		log
		freertos
)