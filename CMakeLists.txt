project(EveLocalization VERSION 1.0.0)

add_library(EveLocalization SHARED
    autoversion.h
    collator.cpp
    collator.h
    collator_blue.cpp
    formatters.cpp
    formatters.h
    localization.cpp
    localization.h
    localization_blue.cpp
    parser.cpp
    parser.h
    parser_blue.cpp
    stdafx.cpp
    version.h
    WrapPointList.cpp
    WrapPointList.h
    WrapPointList_Blue.cpp
)
target_link_libraries(EveLocalization PRIVATE Blue BlueExposure)

target_precompile_headers(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)
target_include_directories(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../carbon/src)
set_target_properties(EveLocalization PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    OUTPUT_NAME "_evelocalization"
)

if(APPLE)
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

install(TARGETS EveLocalization)
