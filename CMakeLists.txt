cmake_minimum_required(VERSION 3.21)

project(MergeQt VERSION 0.1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(QT_NO_MISSING_CATALOG_LANGUAGE_WARNING ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Test)

if (WIN32)
    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /EHa")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()
    set(VERSIONINFO_RC "${CMAKE_BINARY_DIR}/VersionInfo.rc")
    configure_file("${CMAKE_SOURCE_DIR}/VersionInfo.rc.in" "${VERSIONINFO_RC}")
elseif (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
endif()

qt_standard_project_setup(
    REQUIRES 6.5
    I18N_TRANSLATED_LANGUAGES
        en_US
        zh_CN
        zh_TW
        ja_JP
        ko_KR
        de_DE
        fr_FR
        es_ES
        ru_RU
    I18N_SOURCE_LANGUAGE en_US
)

function(mergeqt_configure_target target)
    target_include_directories(${target}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_compile_definitions(${target}
        PUBLIC
            PROJECT_NAME="${PROJECT_NAME}"
        PRIVATE
            QT_MESSAGELOGCONTEXT
    )
endfunction()

function(mergeqt_configure_test_target target)
    mergeqt_configure_target(${target})
    set_target_properties(${target} PROPERTIES
        FOLDER "Test"
        MACOSX_BUNDLE FALSE
        WIN32_EXECUTABLE FALSE
    )
endfunction()

set(MERGEQT_PLATFORM_SOURCES
    src/platform/path/pathservice.cpp
    src/platform/path/pathservice.h
)

set(MERGEQT_CORE_SOURCES
    src/core/binary/binarycomparetypes.h
    src/core/folder/foldercompareengine.cpp
    src/core/folder/foldercompareengine.h
    src/core/folder/foldercomparetypes.h
    src/core/folder/folderoperationexecutor.cpp
    src/core/folder/folderoperationexecutor.h
    src/core/folder/folderoperationplanner.cpp
    src/core/folder/folderoperationplanner.h
    src/core/image/imagecomparetypes.h
    src/core/project/projecttypes.h
    src/core/text/lineardiffengine.cpp
    src/core/text/lineardiffengine.h
    src/core/text/textcomparetypes.h
    src/core/text/textdocumentloader.cpp
    src/core/text/textdocumentloader.h
    src/core/text/textnormalizer.cpp
    src/core/text/textnormalizer.h
)

set(MERGEQT_APP_SOURCES
    src/app/application/applicationbootstrap.cpp
    src/app/application/applicationbootstrap.h
    src/app/application/applicationlaunchrequest.cpp
    src/app/application/applicationlaunchrequest.h
    src/app/application/appcontext.cpp
    src/app/application/appcontext.h
    src/app/compare/binarycomparesessionservice.cpp
    src/app/compare/binarycomparesessionservice.h
    src/app/compare/comparetypes.cpp
    src/app/compare/comparetypes.h
    src/app/compare/comparesessionservice.cpp
    src/app/compare/comparesessionservice.h
    src/app/compare/foldercomparesessionservice.cpp
    src/app/compare/foldercomparesessionservice.h
    src/app/compare/foldercomparereportformatter.cpp
    src/app/compare/foldercomparereportformatter.h
    src/app/compare/imagecomparesessionservice.cpp
    src/app/compare/imagecomparesessionservice.h
    src/app/logging/logger.cpp
    src/app/logging/logger.h
    src/app/settings/propertyhelper.h
    src/app/settings/settingsservice.cpp
    src/app/settings/settingsservice.h
)

set(MERGEQT_UI_SOURCES
    src/ui/widgets/applicationappearance.cpp
    src/ui/widgets/applicationappearance.h
    src/ui/frontend/frontendfactory.cpp
    src/ui/frontend/frontendfactory.h
    src/ui/widgets/binarycomparetabpage.cpp
    src/ui/widgets/binarycomparetabpage.h
    src/ui/widgets/compareinputheader.cpp
    src/ui/widgets/compareinputheader.h
    src/ui/frontend/uifrontend.h
    src/ui/widgets/comparetabpage.h
    src/ui/widgets/comparepagefactory.cpp
    src/ui/widgets/comparepagefactory.h
    src/ui/widgets/foldercomparetabpage.cpp
    src/ui/widgets/foldercomparetabpage.h
    src/ui/widgets/imagecomparetabpage.cpp
    src/ui/widgets/imagecomparetabpage.h
    src/ui/widgets/linenumbertextedit.cpp
    src/ui/widgets/linenumbertextedit.h
    src/ui/widgets/mainwindow.cpp
    src/ui/widgets/mainwindow.h
    src/ui/widgets/opencomparedialog.cpp
    src/ui/widgets/opencomparedialog.h
    src/ui/widgets/placeholdercomparetabpage.cpp
    src/ui/widgets/placeholdercomparetabpage.h
    src/ui/widgets/settingsdialog.cpp
    src/ui/widgets/settingsdialog.h
    src/ui/widgets/textcomparetabpage.cpp
    src/ui/widgets/textcomparetabpage.h
    src/ui/widgets/textdiffoverviewbar.cpp
    src/ui/widgets/textdiffoverviewbar.h
    src/ui/widgets/widgetsfrontend.cpp
    src/ui/widgets/widgetsfrontend.h
    src/ui/widgets/workspacestartpage.cpp
    src/ui/widgets/workspacestartpage.h
)

add_library(MergeQtPlatform STATIC
    ${MERGEQT_PLATFORM_SOURCES}
)
mergeqt_configure_target(MergeQtPlatform)
target_link_libraries(MergeQtPlatform
    PUBLIC
        Qt6::Core
)

add_library(MergeQtCore STATIC
    ${MERGEQT_CORE_SOURCES}
)
mergeqt_configure_target(MergeQtCore)
target_link_libraries(MergeQtCore
    PUBLIC
        Qt6::Core
)

add_library(MergeQtApp STATIC
    ${MERGEQT_APP_SOURCES}
)
mergeqt_configure_target(MergeQtApp)
target_link_libraries(MergeQtApp
    PUBLIC
        MergeQtCore
        MergeQtPlatform
        Qt6::Core
    PRIVATE
        Qt6::Gui
)

add_library(MergeQtUi STATIC
    ${MERGEQT_UI_SOURCES}
)
mergeqt_configure_target(MergeQtUi)
target_link_libraries(MergeQtUi
    PUBLIC
        MergeQtApp
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

qt_add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    main.cpp
    ${VERSIONINFO_RC}
)
mergeqt_configure_target(${PROJECT_NAME})

qt_add_translations(TARGETS ${PROJECT_NAME}
    TS_OUTPUT_DIRECTORY translations
    MERGE_QT_TRANSLATIONS
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        MergeQtUi
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.mergeqt.${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

option(BUILD_TESTING "Build MergeQt tests" ON)
if (BUILD_TESTING)
    enable_testing()
endif()

if (BUILD_TESTING)
    add_custom_target(Test)
    set_target_properties(Test PROPERTIES
        FOLDER "Test"
    )

    add_executable(MergeQtAppApplicationLaunchRequestTest
        tests/app/application/app_application_launch_request_test.cpp
    )
    mergeqt_configure_test_target(MergeQtAppApplicationLaunchRequestTest)
    target_link_libraries(MergeQtAppApplicationLaunchRequestTest
        PRIVATE
            MergeQtApp
            Qt6::Test
    )
    add_dependencies(Test MergeQtAppApplicationLaunchRequestTest)
    add_test(NAME MergeQtAppApplicationLaunchRequestTest COMMAND MergeQtAppApplicationLaunchRequestTest)

    add_executable(MergeQtAppRecentCompareTest
        tests/app/compare/app_recent_compare_test.cpp
    )
    mergeqt_configure_test_target(MergeQtAppRecentCompareTest)
    target_link_libraries(MergeQtAppRecentCompareTest
        PRIVATE
            MergeQtApp
            Qt6::Test
    )
    add_dependencies(Test MergeQtAppRecentCompareTest)
    add_test(NAME MergeQtAppRecentCompareTest COMMAND MergeQtAppRecentCompareTest)

    add_executable(MergeQtAppBinaryCompareSessionTest
        tests/app/compare/app_binary_compare_session_test.cpp
    )
    mergeqt_configure_test_target(MergeQtAppBinaryCompareSessionTest)
    target_link_libraries(MergeQtAppBinaryCompareSessionTest
        PRIVATE
            MergeQtApp
            Qt6::Gui
            Qt6::Test
    )
    add_dependencies(Test MergeQtAppBinaryCompareSessionTest)
    add_test(NAME MergeQtAppBinaryCompareSessionTest COMMAND MergeQtAppBinaryCompareSessionTest)

    add_executable(MergeQtAppImageCompareSessionTest
        tests/app/compare/app_image_compare_session_test.cpp
    )
    mergeqt_configure_test_target(MergeQtAppImageCompareSessionTest)
    target_link_libraries(MergeQtAppImageCompareSessionTest
        PRIVATE
            MergeQtApp
            Qt6::Gui
            Qt6::Test
    )
    add_dependencies(Test MergeQtAppImageCompareSessionTest)
    add_test(NAME MergeQtAppImageCompareSessionTest COMMAND MergeQtAppImageCompareSessionTest)

    add_executable(MergeQtCoreTextTest
        tests/core/text/core_text_compare_test.cpp
    )
    mergeqt_configure_test_target(MergeQtCoreTextTest)
    target_link_libraries(MergeQtCoreTextTest
        PRIVATE
            MergeQtCore
            Qt6::Test
    )
    add_dependencies(Test MergeQtCoreTextTest)
    add_test(NAME MergeQtCoreTextTest COMMAND MergeQtCoreTextTest)

    add_executable(MergeQtCoreFolderTest
        tests/core/folder/core_folder_compare_test.cpp
    )
    mergeqt_configure_test_target(MergeQtCoreFolderTest)
    target_link_libraries(MergeQtCoreFolderTest
        PRIVATE
            MergeQtCore
            Qt6::Test
    )
    add_dependencies(Test MergeQtCoreFolderTest)
    add_test(NAME MergeQtCoreFolderTest COMMAND MergeQtCoreFolderTest)

    add_executable(MergeQtCoreFolderOperationPlannerTest
        tests/core/folder/core_folder_operation_planner_test.cpp
    )
    mergeqt_configure_test_target(MergeQtCoreFolderOperationPlannerTest)
    target_link_libraries(MergeQtCoreFolderOperationPlannerTest
        PRIVATE
            MergeQtCore
            Qt6::Test
    )
    add_dependencies(Test MergeQtCoreFolderOperationPlannerTest)
    add_test(NAME MergeQtCoreFolderOperationPlannerTest COMMAND MergeQtCoreFolderOperationPlannerTest)

    add_executable(MergeQtUiLineNumberTextEditSyncTest
        tests/ui/linenumbertextedit_sync_test.cpp
    )
    mergeqt_configure_test_target(MergeQtUiLineNumberTextEditSyncTest)
    target_link_libraries(MergeQtUiLineNumberTextEditSyncTest
        PRIVATE
            MergeQtUi
            Qt6::Test
            Qt6::Widgets
    )
    add_dependencies(Test MergeQtUiLineNumberTextEditSyncTest)
    add_test(NAME MergeQtUiLineNumberTextEditSyncTest COMMAND MergeQtUiLineNumberTextEditSyncTest)

    add_executable(MergeQtUiTextCompareAlignmentTest
        tests/ui/textcompare_alignment_test.cpp
    )
    mergeqt_configure_test_target(MergeQtUiTextCompareAlignmentTest)
    target_link_libraries(MergeQtUiTextCompareAlignmentTest
        PRIVATE
            MergeQtUi
            Qt6::Test
            Qt6::Widgets
    )
    add_dependencies(Test MergeQtUiTextCompareAlignmentTest)
    add_test(NAME MergeQtUiTextCompareAlignmentTest COMMAND MergeQtUiTextCompareAlignmentTest)

endif()
