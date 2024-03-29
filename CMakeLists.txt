# set the project name
project("FLEXER" CXX)
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)

### COMPILER FLAGS ###
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
        message(FATAL_ERROR "clang version must be >= 9")
    endif()
    add_compile_options("-fPIC")
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8")
        message(FATAL_ERROR "gcc version must be >= 8")
    endif()
    add_compile_options("-fPIC")
    add_compile_options("-std=c++1z")
    add_compile_options("-Wno-psabi")
endif()


### BUILD TYPE ###
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "${Green}Release mode${Reset}")
    add_compile_options("-O3")
    add_compile_options("-DNDEBUG")

elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ##add define DEBUG
    add_compile_definitions(DEBUG)
    message(STATUS "${Green}Debug mode${Reset}")
    if (CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options("-ggdb3")
        add_compile_options("-Wall")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options("-ggdb")
        add_compile_options("-Wall")
    else()
        add_compile_options("-g")
        add_compile_options("-Wall")
    endif()

    #   Usually too restrictive
    #    add_compile_options("-Wextra")
    #    add_compile_options("-Wpedantic")

endif()

message("-- Building for ${CMAKE_SYSTEM_NAME}")

################CUSTOM TARGETS##################### 
find_program(DOXYGEN_FOUND doxygen)
if (DOXYGEN_FOUND)
    set(current_targets ${current_targets} "docFlexer\n")
    message(STATUS "${Blue}doxygen found${Reset}")
    add_custom_target(docFlexer COMMAND doxygen ${PROJECT_SOURCE_DIR}/doc/doxygen.cfg)
endif()

find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    set(current_targets ${current_targets} "format\n")
    message(STATUS "${Blue}clang_format found${Reset}")
    add_custom_target(format COMMAND find ${PROJECT_SOURCE_DIR}/src/ -iname *.h -o -iname *.hh -o -iname *.cc -o -iname *.cpp | xargs clang-format -style=file -i)
endif()


#Basic configuration
include(FindPackageHandleStandardArgs)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

#gtests
add_subdirectory(src/googletest EXCLUDE_FROM_ALL)

#boost
find_package(BOOST1_83) 

### LIBRARIES LINKED TO EVERYONE ######################################

include_directories(
    src/globals/include/
    src/logging/include/
    src/utils/include/
    src/progressbar/include/progresscpp/
    )


#globals
add_subdirectory(src/globals)
link_libraries(globals)

#Logging
add_subdirectory(src/logging)
link_libraries(logging)

#All utils
add_subdirectory(src/utils)
link_libraries(all_utils)

#Progress bar
add_subdirectory(src/progressbar)
link_libraries(progresscpp)


### MAIN EXECUTABLE ###############################################

#Command line parser
add_subdirectory(src/commandLineParser)

SET(NAME flexer)
add_executable(${NAME} src/main.cc)
target_link_libraries(${NAME} PUBLIC text stdc++fs commandLineParser)


########Text#########################################
add_subdirectory(src/text)

### TESTS & EXAMPLES ##################################
enable_testing()
include (CTest)
add_subdirectory(tests)
add_subdirectory(examples)

########## INSTALLATION #################################
#set(CMAKE_INSTALL_MESSAGE NEVER)
#
#INSTALL(TARGETS flexer 
#    RUNTIME DESTINATION flexer/bin
#    LIBRARY DESTINATION flexer/lib
#    ARCHIVE DESTINATION flexer/lib
#    )

