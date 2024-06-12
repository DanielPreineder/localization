cmake_minimum_required(VERSION 3.16)

project(EveLocalization VERSION 1.0.0)

if(NOT DEFINED ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "Missing required environment variable CCP_EVE_PERFORCE_BRANCH_PATH")
elseif(NOT IS_ABSOLUTE $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be an absolute path")
elseif(NOT EXISTS $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH is pointing at a non-existing location $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
elseif(NOT IS_DIRECTORY $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be a directory")
else()
    message(STATUS "Using CCP_EVE_PERFORCE_BRANCH_PATH located at $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
endif()

file(TO_CMAKE_PATH "$ENV{CCP_EVE_PERFORCE_BRANCH_PATH}" BRANCH_ROOT_DIR)
list(INSERT CMAKE_MODULE_PATH 0 ${BRANCH_ROOT_DIR}/cmake)
include(CcpGlobalSettings)

find_package(Blue CONFIG REQUIRED)
find_package(BlueExposure CONFIG REQUIRED)

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

# The following variable is defined only on DLL systems
if(CMAKE_IMPORT_LIBRARY_SUFFIX)
    add_custom_command(
            TARGET EveLocalization POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:EveLocalization> $<TARGET_FILE_DIR:EveLocalization>
            COMMAND_EXPAND_LISTS
    )
endif()

set(EXEFILE_NAME Exefile)
if(WIN32)
    set(EXEFILE_NAME ${EXEFILE_NAME}.exe)
endif()

# Exefile is used to run the Python tests.
add_custom_command(TARGET EveLocalization POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${Exefile_ROOT}/${CCP_VENDOR_BIN_PATH}/${EXEFILE_NAME} $<TARGET_FILE_DIR:EveLocalization>
        COMMAND_EXPAND_LISTS
        VERBATIM
)

# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTING "Build tests. Enabled by default." ON)
    include(CTest)
    if(BUILD_TESTING)
        # add the python tests
        list(APPEND PYTHON_TESTS test_eveLocalization.py)
        foreach(PYTHON_TEST ${PYTHON_TESTS})
            add_test(NAME ${PYTHON_TEST}
                    COMMAND $<TARGET_FILE_DIR:EveLocalization>/${EXEFILE_NAME} /inherit /buildflavor=$<LOWER_CASE:$<CONFIG>> /py ${PYTHON_TEST}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/eveLocalization/tests
            )
            if(WIN32)
                set_tests_properties(${PYTHON_TEST} PROPERTIES ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:EveLocalization>\;${CMAKE_CURRENT_SOURCE_DIR}/python\;${BRANCH_ROOT_DIR}/carbon/common/stdlib\;${Python_ROOT}/${CCP_VENDOR_BIN_PATH}\;${Greenlet_ROOT}/python\;${Greenlet_ROOT}/${CCP_VENDOR_BIN_PATH}\;${Scheduler_ROOT}/${CCP_VENDOR_BIN_PATH};BUILDFLAVOR=$<LOWER_CASE:$<CONFIG>>")
            elseif (APPLE)
                set_tests_properties(${PYTHON_TEST} PROPERTIES ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:EveLocalization>:${CMAKE_CURRENT_SOURCE_DIR}/python:${BRANCH_ROOT_DIR}/carbon/common/stdlib:${Python_ROOT}/${CCP_VENDOR_BIN_PATH}:${Greenlet_ROOT}/python:${Greenlet_ROOT}/${CCP_VENDOR_BIN_PATH}:${Scheduler_ROOT}/${CCP_VENDOR_LIB_PATH};BUILDFLAVOR=$<LOWER_CASE:$<CONFIG>>")
            else()
                message(FATAL_ERROR "Unsupported platform")
            endif ()
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
