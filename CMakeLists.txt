cmake_minimum_required(VERSION 3.16)
set(PROJECT text-highlight-plugin)
project(${PROJECT} VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

set(KF_DEP_VERSION "${QT_MAJOR_VERSION}")
find_package(ECM ${KF_DEP_VERSION} QUIET REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt${QT_MAJOR_VERSION}Widgets CONFIG REQUIRED)

find_package(KF${QT_MAJOR_VERSION}TextEditor ${KF_DEP_VERSION} QUIET REQUIRED)

find_package(KF${QT_MAJOR_VERSION}
        REQUIRED COMPONENTS
        CoreAddons # Core addons on top of QtCore
        I18n # For localization
        TextEditor # The editor component
)

# This line defines the actual target
if (QT_MAJOR_VERSION EQUAL 6)
    kcoreaddons_add_plugin(${PROJECT}
            INSTALL_NAMESPACE "kf6/ktexteditor")
else ()
    kcoreaddons_add_plugin(${PROJECT}
            INSTALL_NAMESPACE "ktexteditor")
endif ()

include(ECMQtDeclareLoggingCategory)
include(ECMOptionalAddSubdirectory)

target_sources(
        ${PROJECT}
        PRIVATE
        text_highlight_plugin.cpp
        text_highlight_plugin_view.cpp
        text_highlight_plugin.json

        plugin.qrc
)

add_compile_options(-O3 -Wall -Wextra -Wpedantic -Wno-dev)

# This makes the plugin translatable
target_compile_definitions(${PROJECT} PRIVATE TRANSLATION_DOMAIN="${PROJECT}")

target_link_libraries(${PROJECT}
        PRIVATE
        KF${QT_MAJOR_VERSION}::CoreAddons KF${QT_MAJOR_VERSION}::I18n KF${QT_MAJOR_VERSION}::TextEditor
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
