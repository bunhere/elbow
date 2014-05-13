cmake_minimum_required(VERSION 2.8)
project(Elbow)
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

if ("${WEBKIT_BACKEND}" STREQUAL "ewebkit")
    find_package(EWebKit REQUIRED)
    add_definitions(-DUSE_EWEBKIT=1)

    include(FindPkgConfig)
    pkg_check_modules(PKGS REQUIRED cairo libsoup-2.4)

    foreach(flag ${PKGS_CFLAGS})
        set(ELBOW_CFLAGS "${ELBOW_CFLAGS} ${flag}")
    endforeach(flag)

    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ELBOW_CFLAGS}")
elseif ("${WEBKIT_BACKEND}" STREQUAL "ewebkit2")
    find_package(EWebKit2 REQUIRED)
    add_definitions(-DUSE_EWEBKIT2=1)
else ()
    set(WEBKIT_BACKEND "elm_web")
    add_definitions(-DUSE_ELM_WEB=1)

    # FIXME: ElementaryConfig.cmake should provide this
    if ("${USE_WEBKIT}" STREQUAL "ewebkit")
        find_package(EWebKit REQUIRED)
    else ()
        find_package(EWebKit2 REQUIRED)
    endif ()

endif ()

message("Selected web backend is ${WEBKIT_BACKEND}/${USE_WEBKIT}")

set(ELBOW_SOURCES
    src/app.c
    src/bookmark.c
    src/browser.c
    src/main.c
    src/webview.c
    src/webview_elm.c
    src/webview_ewk.c
    src/webview_ewk2.c
)

set(ELBOW_INCLUDE_DIRS
    ${ECORE_FILE_INCLUDE_DIRS}
    ${EDJE_INCLUDE_DIRS}
    ${ELEMENTARY_INCLUDE_DIRS}
    ${EWEBKIT_INCLUDE_DIR}
    ${EWEBKIT2_INCLUDE_DIR}
)

set(ELBOW_LIBRARIES
    ${ECORE_INPUT_LIBRARIES}
    ${ECORE_FILE_LIBRARIES}
    ${ECORE_LIBRARIES}
    ${EDJE_LIBRARIES}
    ${EINA_LIBRARIES}
    ${ELEMENTARY_LIBRARIES}
    ${EWEBKIT_LIBRARY}
    ${EWEBKIT2_LIBRARY}
    ${EVAS_LIBRARIES}
)

message("::: ${ELBOW_LIBRARIES}")
add_subdirectory(data/themes)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ELBOW_CFLAGS}")

add_executable(elbow ${ELBOW_SOURCES})
include_directories(${ELBOW_INCLUDE_DIRS})
target_link_libraries(elbow ${ELBOW_LIBRARIES})

install(TARGETS elbow DESTINATION ${EXEC_INSTALL_DIR})
