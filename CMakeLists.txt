cmake_minimum_required(VERSION 3.16)

project(EveLocalization VERSION 1.0.0)

include(cmake/CcpGlobalSettings.cmake)
include(cmake/CcpTargetConfigurations.cmake)
include(cmake/CcpPackageConfigHelpers.cmake)

find_package(carbon-blueexposure REQUIRED CONFIG)
find_package(carbon-blue REQUIRED CONFIG)
find_package(carbon-exefile-interpreter REQUIRED CONFIG)

set(headers
    autoversion.h
    collator.h
    formatters.h
    localization.h
    parser.h
    version.h
    WrapPointList.h
)

set(sources
    collator.cpp
    collator.mm
    collator_blue.cpp
    formatters.cpp
    localization.cpp
    localization_blue.cpp
    parser.cpp
    parser_blue.cpp
    stdafx.cpp
    WrapPointList.cpp
    WrapPointList_Blue.cpp
)

ccp_add_library(EveLocalization SHARED
    ${headers}
    ${sources}
)
target_link_libraries(EveLocalization PRIVATE Blue BlueExposure)

target_precompile_headers(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)
target_include_directories(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(EveLocalization PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    OUTPUT_NAME "_evelocalization"
)

if(APPLE)
    enable_language(OBJCXX)
    set_source_files_properties(${sources} PROPERTIES LANGUAGE OBJCXX)

    set_target_properties(
        EveLocalization
        PROPERTIES 
            PREFIX ""
            SUFFIX ".so"
    )
    target_link_libraries(EveLocalization PRIVATE "-framework Foundation")

elseif(WIN32)

    target_link_libraries(EveLocalization PRIVATE usp10)

endif()

ccp_prepare_exefile(
        TARGET EveLocalization
        BUILD_TYPE ${CMAKE_BUILD_TYPE}
        COMMAND_OUT PYTHON_INTERPRETER
        PYTHONPATH_OUT PYTHONPATH
        TEST_DIR $<TARGET_FILE_DIR:EveLocalization>
)

if (WIN32)
    add_custom_command(
            TARGET EveLocalization POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:EveLocalization> $<TARGET_FILE_DIR:EveLocalization>
            COMMAND_EXPAND_LISTS
    )
endif()

# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTING "Build tests. Enabled by default." ON)
    include(CTest)
    if(BUILD_TESTING)
        # add the python tests
        list(APPEND PYTHON_TESTS test_eveLocalization.py)
        foreach(PYTHON_TEST ${PYTHON_TESTS})
            add_test(NAME ${PYTHON_TEST}
                    COMMAND ${PYTHON_INTERPRETER} /inherit /buildflavor=$<LOWER_CASE:$<CONFIG>> /py ${PYTHON_TEST}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/eveLocalization/tests
            )
            set_tests_properties(${PYTHON_TEST} PROPERTIES ENVIRONMENT "PYTHONPATH=${PYTHONPATH}\;${CMAKE_CURRENT_SOURCE_DIR}/python;BUILDFLAVOR=$<LOWER_CASE:$<CONFIG>>")
        endforeach()
    endif()

    # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
    install(
        TARGETS EveLocalization
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )

    # Install python package
    install(DIRECTORY python/ DESTINATION python
        FILES_MATCHING
        PATTERN "*.py"
        PATTERN "package.yaml"
    )
endif()
