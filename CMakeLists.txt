cmake_minimum_required(VERSION 2.8)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if (DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else ()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif ()
endif ()

project(vita2npsc)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "VITA2NPSC")
set(VITA_TITLEID "VITA2NPSC")
set(VITA_VERSION "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti -std=c++11")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

link_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
)

FUNCTION(ADD_RESOURCES out_var)
    SET(result)
    FOREACH(in_f ${ARGN})
        SET(out_f "${CMAKE_CURRENT_BINARY_DIR}/${in_f}.o")
        GET_FILENAME_COMPONENT(out_dir ${out_f} DIRECTORY)
        ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
                COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${in_f}
                DEPENDS ${in_f}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Building resource ${out_f}"
                VERBATIM
                )
        LIST(APPEND result ${out_f})
    ENDFOREACH()
    SET(${out_var} "${result}" PARENT_SCOPE)
ENDFUNCTION()

add_subdirectory(third/libb64)
add_subdirectory(third/PkgDecrypt)
add_subdirectory(third/restclient-cpp)

file(GLOB res_files RELATIVE
        ${CMAKE_SOURCE_DIR}
        data/english.txt)

add_resources(res ${res_files})

add_executable(${PROJECT_NAME}
        src/modules/screen.cpp
        src/modules/reader.cpp
        src/modules/uploader.cpp
        src/vita2npsc.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE third)
target_include_directories(${PROJECT_NAME} PRIVATE third/restclient-cpp/include)

link_libraries(b64)

target_link_libraries(${PROJECT_NAME}
        b64
        PkgDecrypt
        restclient-cpp
        curl
        ssl
        crypto
        z
        m
        c
        vita2d
        SceCtrl_stub
        SceSysmodule_stub
        SceDisplay_stub
        ScePgf_stub
        SceGxm_stub
        SceCommonDialog_stub
        SceNet_stub
        )

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        FILE sce_sys/icon0.png sce_sys/icon0.png
        FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        )
