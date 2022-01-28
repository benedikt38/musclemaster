#
# Try to find GLFW3 library and include path.
# Once done this will define
#
# GLFW3_FOUND
# GLFW3_INCLUDE_PATH
# GLFW3_LIBRARY
# 

SET(OPENCL_SEARCH_PATHS
	$ENV{OpenCL_ROOT}
	${DEPENDENCIES_ROOT}
	/usr			# APPLE
	/usr/local		# APPLE
	/opt/local		# APPLE
)


FIND_PATH(OpenCL_INCLUDE_PATH
    NAMES
       CL/cl_gl.h
    PATHS
        ${OPENCL_SEARCH_PATHS}
    PATH_SUFFIXES
        include
    DOC
        "The directory where CL/cl_gl.h resides"
)

FIND_LIBRARY( OpenCL_LIBRARY
    NAMES 
	openCL/openCL.lib
      PATHS
        ${OPENCL_SEARCH_PATHS}
    PATH_SUFFIXES
        lib
    DOC
        "The directory where glew32s.lib resides"
  )

SET(OpenCL_FOUND "NO")
IF (OpenCL_INCLUDE_PATH AND OpenCL_LIBRARY)
	SET(OpenCL_LIBRARIES ${OpenCL_LIBRARY})
	SET(OpenCL_FOUND "YES")
    message("EXTERNAL LIBRARY 'OpenCL' FOUND")
ELSE()
    message("ERROR: EXTERNAL LIBRARY 'OpenCL' NOT FOUND")
ENDIF (OpenCL_INCLUDE_PATH AND OpenCL_LIBRARY)
