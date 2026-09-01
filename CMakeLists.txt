cmake_minimum_required(VERSION 3.16)

project(naite
    VERSION 0.1.0
    DESCRIPTION "A simple timer application"
    HOMEPAGE_URL "https://github.com/Fosca709/naite"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Multimedia Widgets)

include(GNUInstallDirs)

add_executable(${PROJECT_NAME}
    main.cpp
    resources.qrc
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Multimedia Qt6::Widgets)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES packaging/naite.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES assets/icons/icon.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME naite.svg
)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_CONTACT "Fosca709")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Fosca709")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

include(CPack)
