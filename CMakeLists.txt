cmake_minimum_required (VERSION 2.8)

set (BRANCH_ROOT ${CMAKE_SOURCE_DIR})
get_filename_component(BRANCH_ROOT ${BRANCH_ROOT} ABSOLUTE)

set (PROCESS_DIR ${CMAKE_SOURCE_DIR})
get_filename_component(PROCESS_DIR ${PROCESS_DIR} ABSOLUTE)

set (BRANCH_NAME "trafficInspector")

#set(CMAKE_C_COMPILER /usr/bin/gcc-7)
#set(CMAKE_CXX_COMPILER /usr/bin/g++-7)

#==============================================================================
#Common Directories
set (DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
list (APPEND SRC_DIRECTORIES ${DIRECTORIES}/src
                             ${DIRECTORIES}/src/Packets)

#Headers dir
set (INC_DIRECTORIES ${DIRECTORIES}
                        ${DIRECTORIES}/include
                        ${DIRECTORIES}/include/Packets
                        ${DIRECTORIES}/include/Utils)

#==============================================================================
#Compile flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g3 -fvar-tracking -MD -Wall -Wno-invalid-offsetof -std=c++1z -D__GXX_EXPERIMENTAL_CXX0X__ ")

#DEBUG option
list (APPEND DEFINES_ALL -DDEBUG)

#==============================================================================
# Libraries
find_library (LIB_PTHREAD pthread)
list (APPEND LIBS ${LIB_PTHREAD})
#==============================================================================

list(REMOVE_DUPLICATES INC_DIRECTORIES)
list(REMOVE_DUPLICATES SRC_DIRECTORIES)
list(REMOVE_DUPLICATES DEFINES_ALL)


add_definitions(${DEFINES_ALL})

foreach (_dir ${SRC_DIRECTORIES})
    file(GLOB _src ${_dir}/*.cpp)
    list(APPEND SRC_ALL ${_src})

    file(GLOB _src ${_dir}/*.c)
    list(APPEND SRC_ALL ${_src})
endforeach()

include_directories(${INC_DIRECTORIES})
include_directories(${CMAKE_SOURCE_DIR})

foreach (_lib ${LIBS})
    message(${_lib})
endforeach()


#==============================================================================
# Setup project
set (PROC_NAME "trafficInspector")
add_executable(${PROC_NAME} ${SRC_ALL})
set_target_properties(${PROC_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROC_NAME} ${LIBS})
