if (${DISABLE_NAUTY})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISABLE_NAUTY")
endif()
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wno-unused-function")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISABLE_GLUCOSE")
endif()

file(GLOB FILENAMES *.cpp)
set(lt1name ${CMAKE_CURRENT_SOURCE_DIR}/link_test1.cpp)
set(lt2name ${CMAKE_CURRENT_SOURCE_DIR}/link_test2.cpp)
list(REMOVE_ITEM FILENAMES ${lt1name} ${lt2name})

foreach(FILENAME ${FILENAMES})
    get_filename_component(BASENAME ${FILENAME} NAME_WE)

    set(TARGET_NAME ${BASENAME})

    add_executable(${TARGET_NAME} ${FILENAME})
    if (WIN32)
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/cryptominisat/cmsat5-src/Debug")
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/cryptominisat/cmsat5-src/Release")
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/test/Debug")
        file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/test/Release")
        set(DEBUG_DLL "${PROJECT_BINARY_DIR}/cryptominisat/cmsat5-src/Debug/cryptominisat5win.dll")
        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} 
                -E touch
                ${DEBUG_DLL}
        )
        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} 
                -E copy_if_different
                ${DEBUG_DLL}
                "${PROJECT_BINARY_DIR}/test/Debug/"
        )
        set(RELEASE_DLL "${PROJECT_BINARY_DIR}/cryptominisat/cmsat5-src/Release/cryptominisat5win.dll")
        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} 
            -E touch
            ${RELEASE_DLL}
        )
        add_custom_command(
            TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} 
            -E copy_if_different
            ${RELEASE_DLL}
            "${PROJECT_BINARY_DIR}/test/Release/"
        )
    endif()
    if (PERCY_BUILD_CMS)
        target_link_libraries(${TARGET_NAME} percy libcryptominisat5)
    else()
        target_link_libraries(${TARGET_NAME} percy)
    endif()
    if (UNIX)
        target_link_libraries(${TARGET_NAME} ${ZLIB_LIBRARIES})
    endif()
    add_test(${TARGET_NAME} ${TARGET_NAME})
endforeach()

# Make sure that there are no linking issues
add_executable(link_test link_test1.cpp link_test2.cpp)
target_link_libraries(link_test percy)
