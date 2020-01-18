project(plasma-webqq)

find_package(KDE4 REQUIRED 4.6)
find_package(KActivities REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-Wall -Wextra)

################## plasma-containment-webqq ##################

set(webqq_SRCS webqq.cpp)

kde4_add_ui_files(webqq_SRCS webqqconfig.ui)

kde4_add_plugin(plasma_containment_webqq ${webqq_SRCS})

target_link_libraries(plasma_containment_webqq ${KDE4_PLASMA_LIBS} ${KDE4_KDEWEBKIT_LIBRARY} ${KACTIVITIES_LIBRARY})

install(TARGETS plasma_containment_webqq DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-webqq.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma_webqq.notifyrc DESTINATION ${DATA_INSTALL_DIR}/plasma_webqq)

add_subdirectory(icons)
add_subdirectory(po)
