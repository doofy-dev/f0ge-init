project(rendertest C)
set(ASSET_FOLDER "assets")
set(FIRMWARE_PATH ~/.ufbt/current/sdk_headers/f7_sdk)

cmake_minimum_required(VERSION 3.16)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")


# Flipper include paths
include_directories(${FIRMWARE_PATH}/furi/)
include_directories(${FIRMWARE_PATH}/lib/)
include_directories(${FIRMWARE_PATH}/applications/services/)
include_directories(${FIRMWARE_PATH}/targets/f7/furi_hal/)


# Function to define and redefine macros globally
function(define_macro_if_necessary MACRO_NAME)
    # Undefine the macro if it is already defined
    add_definitions(-U${MACRO_NAME})
    # Define the macro with the new value
    add_definitions(-D"${MACRO_NAME}\(tag, format, ...\) = printf\(tag format, \#\#__VA_ARGS__\)")
endfunction()


# Generate icon template file
set(GENERATED_IN_FILE "${CMAKE_BINARY_DIR}/icons.h.in")
if (NOT EXISTS ${GENERATED_IN_FILE})
    file(WRITE ${GENERATED_IN_FILE} "#pragma once \n\n")
    file(APPEND ${GENERATED_IN_FILE} "const void p;\n\n")
    file(APPEND ${GENERATED_IN_FILE} "\/\/ Generated file constants\n")
    file(APPEND ${GENERATED_IN_FILE} "\@FILE_CONSTANTS\@\n")
endif ()


# Load the images and generate the mock variables so you can have autocomplete
set(DIRECTORY_TO_SCAN "${CMAKE_SOURCE_DIR}/${ASSET_FOLDER}")
file(GLOB FILES_IN_DIRECTORY "${DIRECTORY_TO_SCAN}/*")
set(FILE_CONSTANTS "")
foreach (FILE ${FILES_IN_DIRECTORY})
    get_filename_component(FILE_NAME ${FILE} NAME)
    get_filename_component(FILE_NAME_WE ${FILE} NAME_WE)

    set(FILE_CONSTANTS "${FILE_CONSTANTS}#define I_${FILE_NAME_WE} p\n")
endforeach ()
# include the folder where the generated icon fil will be created
include_directories(${CMAKE_BINARY_DIR})
#generate the file
configure_file(${CMAKE_BINARY_DIR}/icons.h.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_icons.h @ONLY)


# Override the log functions so they not show up as error
define_macro_if_necessary("FURI_LOG_W")
define_macro_if_necessary("FURI_LOG_D")
define_macro_if_necessary("FURI_LOG_E")
define_macro_if_necessary("FURI_LOG_I")

include_directories("${CMAKE_SOURCE_DIR}/../../")
# loads the source files and ads them to the project
FILE(GLOB_RECURSE SOURCES "*.c" "*.h")
add_executable(${PROJECT_NAME} ${SOURCES}
        main.c)