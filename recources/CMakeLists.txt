project(RECOURCES)

file(GLOB_RECURSE OBJ *.obj)

SOURCE_GROUP(OBJ-Files FILES ${OBJ})

add_custom_target(RECOURCES SOURCES ${OBJ})