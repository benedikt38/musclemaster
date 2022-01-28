#
# Try to find Assimp library and include path.
# Once done this will define
#
# ASSIMP_FOUND
# ASSIMP_INCLUDE_PATH
# ASSIMP_LIBRARY
# 
# It will add the assimp DLL for windows builds to PROJECT_RUNTIME_LIBRARIES

set(ASSIMP_ROOT_ENV $ENV{ASSIMP_ROOT})

SET(ASSIMP_SEARCH_PATHS
	
	${DEPENDENCIES_ROOT}
	/usr			# APPLE
	/usr/local		# APPLE
	/opt/local		# APPLE
)
	
	

FIND_PATH(ASSIMP_INCLUDE_PATH 
    NAMES
       assimp/defs.h
    PATHS
        ${ASSIMP_SEARCH_PATHS}
    PATH_SUFFIXES
        include
    DOC
        "The directory where assimp/defs.h resides"
)

    FIND_LIBRARY( ASSIMP_LIBRARY
        NAMES
            assimp/assimp-vc140-mt.lib
        PATHS
            ${ASSIMP_SEARCH_PATHS}
    PATH_SUFFIXES
        lib
    DOC
        "The directory where assimp-vc140-mt.lib resides"
  )

SET(ASSIMP_FOUND "NO")
IF (ASSIMP_INCLUDE_PATH AND ASSIMP_LIBRARY)
	SET(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})
	SET(ASSIMP_FOUND "YES")
    message("EXTERNAL LIBRARY 'ASSIMP' FOUND")
ELSE()
    message("ERROR: EXTERNAL LIBRARY 'ASSIMP' NOT FOUND")
ENDIF (ASSIMP_INCLUDE_PATH AND ASSIMP_LIBRARY)
