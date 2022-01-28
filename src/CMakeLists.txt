file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}
	*.cpp
	*.c
	*.h)

add_executable(Source ${SOURCE_FILES})
target_compile_features(Source PUBLIC cxx_std_17)

