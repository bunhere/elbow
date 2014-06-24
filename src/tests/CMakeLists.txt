set(BOOKMARK_SOURCES
    ../database/bookmark.c

    test_bookmark.c
)

set(TEST_INCLUDE_DIRS
    ${EINA_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIR}
)
set(TEST_LIBRARIES
    ${EINA_LIBRARIES}
    ${SQLITE_LIBRARIES}
)

include_directories(
    ../database
    ..
    ${TEST_INCLUDE_DIRS}
)

add_executable(bookmark_test ${BOOKMARK_SOURCES})
target_link_libraries(bookmark_test ${TEST_LIBRARIES})
