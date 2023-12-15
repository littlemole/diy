include("out/build/gcc-release/CPackConfig.cmake")

set(CPACK_INSTALL_CMAKE_PROJECTS
    "out/build/gcc-release;${CPACK_PACKAGE_NAME};ALL;/"
    "out/build/gcc-debug;${CPACK_PACKAGE_NAME};ALL;/"
    )