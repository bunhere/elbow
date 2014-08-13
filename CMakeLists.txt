cmake_minimum_required(VERSION 2.8)
project(Elbow)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

add_definitions(-DDEFAULT_WIDTH=800)
add_definitions(-DDEFAULT_HEIGHT=600)
add_definitions(-DTHEME_BUILD_PATH=\"${CMAKE_BINARY_DIR}/data/themes\") 
add_definitions(-DDEVELOPER_MODE=1)

set(EXEC_INSTALL_DIR "bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${EXEC_INSTALL_DIR})

# Check package dependency.
find_package(Ecore REQUIRED COMPONENTS Input File)
find_package(Edje REQUIRED)
find_package(Eina REQUIRED)
find_package(Evas REQUIRED)
find_package(Elementary REQUIRED)

find_package(Sqlite REQUIRED)

if ("${WEBKIT_BACKEND}" STREQUAL "ewebkit2")
    find_package(EWebKit2 REQUIRED)
    add_definitions(-DUSE_EWEBKIT2=1)
else ()
    set(WEBKIT_BACKEND "elm_web")
    add_definitions(-DUSE_ELM_WEB=1)

    find_package(EWebKit2 REQUIRED)
endif ()

message("Selected web backend is ${WEBKIT_BACKEND}")

set(ELBOW_SOURCES
    src/database/bookmark.c

    src/app.c
    src/browser.c
    src/homescreen.c
    src/main.c
    src/webview.c
    src/webview_elm.c
    src/webview_ewk2.c
)

set(ELBOW_INCLUDE_DIRS
    ${ECORE_FILE_INCLUDE_DIRS}
    ${EDJE_INCLUDE_DIRS}
    ${ELEMENTARY_INCLUDE_DIRS}
    ${EWEBKIT2_INCLUDE_DIR}
    ${SQLITE_INCLUDE_DIR}
)

set(ELBOW_LIBRARIES
    ${ECORE_INPUT_LIBRARIES}
    ${ECORE_FILE_LIBRARIES}
    ${ECORE_LIBRARIES}
    ${EDJE_LIBRARIES}
    ${EINA_LIBRARIES}
    ${ELEMENTARY_LIBRARIES}
    ${EWEBKIT2_LIBRARY}
    ${EVAS_LIBRARIES}
    ${SQLITE_LIBRARIES}
)

add_subdirectory(data/themes)
add_subdirectory(src/tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ELBOW_CFLAGS}")
set(CMAKE_C_FLAGS "-Wall ${CMAKE_C_FLAGS}")

add_executable(elbow ${ELBOW_SOURCES})
include_directories(${ELBOW_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)
target_link_libraries(elbow ${ELBOW_LIBRARIES})

install(TARGETS elbow DESTINATION ${EXEC_INSTALL_DIR})
