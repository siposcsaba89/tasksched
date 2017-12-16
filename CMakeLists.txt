cmake_minimum_required(VERSION 3.0)
project(tasksched)
set_property(GLOBAL PROPERTY USE_FOLDERS On) 

option(BUILD_EXAMPLES "Build tasksched examples" ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)



set(PUBLIC_HDRS
    include/tasksched/tasksched.h
    include/tasksched/timeline_creator.h
)

set(SRCS 
    src/tasksched.cpp
    src/timeline_creator.cpp
)

add_library(${PROJECT_NAME} ${PUBLIC_HDRS} ${SRCS})
add_library(::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
if (BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${PROJECT_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN 1)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER libs)
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()

source_group(${PROJECT_NAME} FILES ${PUBLIC_HDRS})

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/gen/${PROJECT_NAME}/${PROJECT_NAME}_export.h)


target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/gen>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread)
endif()

if (BUILD_EXAMPLES)
    add_executable(basics examples/basic.cpp)
    set_target_properties(basics PROPERTIES DEBUG_POSTFIX _d)
    set_target_properties(basics PROPERTIES FOLDER examples)
    target_link_libraries(basics PRIVATE ::${PROJECT_NAME})
endif()


include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}-config-version.cmake"
  VERSION 0.0.1
  COMPATIBILITY AnyNewerVersion
)

configure_file(cmake/${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets DESTINATION 
    ARCHIVE DESTINATION lib LIBRARY DESTINATION lib RUNTIME DESTINATION bin)
install(FILES 
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake 
        ${CMAKE_CURRENT_BINARY_DIR}/gen/${PROJECT_NAME}-config-version.cmake
    DESTINATION 
        lib/cmake/${PROJECT_NAME})
    
install(EXPORT ${PROJECT_NAME}-targets NAMESPACE :: DESTINATION lib/cmake/${PROJECT_NAME})

install(FILES
    ${CMAKE_BINARY_DIR}/gen/${PROJECT_NAME}/${PROJECT_NAME}_export.h
	${PUBLIC_HDRS}
    DESTINATION include/${PROJECT_NAME}/${PROJECT_NAME})


