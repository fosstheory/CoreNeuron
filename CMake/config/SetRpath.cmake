# enable @rpath in the install name for any shared library being built
set(CMAKE_MACOSX_RPATH 1)

# On platforms like bgq, xlc didn't like rpath with static build
# Similar issue was seen on Cray
IF(NOT CRAY_SYSTEM)
    # use, i.e. don't skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

    # when building, don't use the install RPATH already # (but later on when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")

    # the RPATH to be used when installing, but only if it's not a system directory
    LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${LIB_INSTALL_DIR}" isSystemDir)
    IF("${isSystemDir}" STREQUAL "-1")
        SET(CMAKE_INSTALL_RPATH "${LIB_INSTALL_DIR}")
    ENDIF("${isSystemDir}" STREQUAL "-1")
ENDIF()
