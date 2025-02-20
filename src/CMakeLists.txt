cmake_minimum_required(VERSION 3.16)
project(ngPost VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC, UIC, and RCC processing by CMake
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the required Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# Enable Qt's standard project setup
qt_standard_project_setup()

# Option to enable HMI functionality
option(use_hmi "Enable HMI functionality" ON)
if(DEFINED ENV{USE_HMI})
    set(use_hmi "$ENV{USE_HMI}" CACHE BOOL "Enable HMI functionality.")
endif()

# Check for QT_ROOT_DIR environment variable
if (DEFINED ENV{QT_ROOT_DIR})
    set(QT_ROOT_DIR "$ENV{QT_ROOT_DIR}")
else()
    message(FATAL_ERROR "QT_ROOT_DIR environment variable is not set. Please set it in your environment.")
endif()

# Specify the source files
set(SOURCES
    ArticleBuilder.cpp
    FileUploader.cpp
    FoldersMonitorForNewFiles.cpp
    NgPost.cpp
    NntpCheckCon.cpp
    NntpConnection.cpp
    NzbCheck.cpp
    Poster.cpp
    PostingJob.cpp
    hmi/PathSettingsWidget.cpp
    hmi/SettingsWidget.cpp
    hmi/MainWindow.cpp
    hmi/PostingWidget.cpp
    hmi/AutoPostWidget.cpp
    main.cpp
    nntp/Nntp.cpp
    nntp/NntpArticle.cpp
    nntp/NntpFile.cpp
    utils/CmdOrGuiApp.cpp
    utils/Yenc.cpp
    utils/ConfigUtility.cpp
    utils/ConfigKeys.cpp
)

# Specify the header files
set(HEADERS
    ArticleBuilder.h
    FileUploader.h
    FoldersMonitorForNewFiles.h
    NgPost.h
    NntpCheckCon.h
    NntpConnection.h
    NzbCheck.h
    Poster.h
    PostingJob.h
    hmi/PathSettingsWidget.h
    hmi/SettingsWidget.h
    hmi/MainWindow.h
    hmi/PostingWidget.h
    hmi/AutoPostWidget.h
    nntp/Nntp.h
    nntp/NntpArticle.h
    nntp/NntpFile.h
    nntp/NntpServerParams.h
    utils/CmdOrGuiApp.h
    utils/Macros.h
    utils/PureStaticClass.h
    utils/Yenc.h
    utils/ConfigUtility.h
    utils/ConfigKeys.h
)

# Specify the UI files
set(UI_FILES
    hmi/PathSettingsWidget.ui
    hmi/SettingsWidget.ui
    hmi/PostingWidget.ui
    hmi/AutoPostWidget.ui
    hmi/MainWindow.ui
)

# Add the executable target and include the source, header, and UI files
qt_add_executable(ngPost WIN32
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    resources/version.rc
)

# Include the directories for headers and UI files
target_include_directories(ngPost PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compile definitions
target_compile_definitions(ngPost PRIVATE
    APP_VERSION="5.0"
    QT_DEPRECATED_WARNINGS
    __COMPUTE_IMMEDIATE_SPEED__
    __RELEASE_ARTICLES_WHEN_CON_FAILS__
    __USE_CONNECTION_TIMEOUT__
    __USE_TMP_RAM__
)

# Link the Qt6 libraries to your executable
target_link_libraries(ngPost PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Widgets
)

# Resources
set(resources_resource_files
    "resources/icons/reset-password.png"
    "resources/icons/about.png"
    "resources/icons/auto.png"
    "resources/icons/clear.png"
    "resources/icons/close.jpg"
    "resources/icons/cmd.png"
    "resources/icons/file.png"
    "resources/icons/flag_DE.png"
    "resources/icons/flag_EN.png"
    "resources/icons/flag_ES.png"
    "resources/icons/flag_FR.png"
    "resources/icons/flag_NL.png"
    "resources/icons/flag_PT.png"
    "resources/icons/flag_ZH.png"
    "resources/icons/folder.png"
    "resources/icons/genKey.png"
    "resources/icons/ko.png"
    "resources/icons/left.png"
    "resources/icons/monitor.png"
    "resources/icons/ngPost.png"
    "resources/icons/ok.png"
    "resources/icons/pause.png"
    "resources/icons/pending.png"
    "resources/icons/play.png"
    "resources/icons/plus.png"
    "resources/icons/quick.png"
    "resources/icons/right.png"
    "resources/icons/save.png"
    "resources/icons/scanFolder.png"
    "resources/icons/uploading.png"
    "resources/lang/ngPost_de.qm"
    "resources/lang/ngPost_en.qm"
    "resources/lang/ngPost_es.qm"
    "resources/lang/ngPost_fr.qm"
    "resources/lang/ngPost_nl.qm"
    "resources/lang/ngPost_pt.qm"
    "resources/lang/ngPost_zh.qm"
)

qt_add_resources(ngPost "resources"
    PREFIX "/"
    BASE "resources"
    FILES ${resources_resource_files}
)

set(qmake_immediate_resource_files
    "resources/version.rc"
)

qt_add_resources(ngPost "qmake_immediate"
    PREFIX "/"
    FILES ${qmake_immediate_resource_files}
)

# HMI specific settings
if(use_hmi)
    set(UI_FILES
        hmi/MainWindow.ui
        hmi/AutoPostWidget.ui
        hmi/PostingWidget.ui
        hmi/SettingsWidget.ui
    )

    target_sources(ngPost PUBLIC
        hmi/AutoPostWidget.cpp hmi/AutoPostWidget.h hmi/AutoPostWidget.ui
        hmi/CheckBoxCenterWidget.cpp hmi/CheckBoxCenterWidget.h
        hmi/MainWindow.cpp hmi/MainWindow.h hmi/MainWindow.ui
        hmi/PostingWidget.cpp hmi/PostingWidget.h hmi/PostingWidget.ui
        hmi/SettingsWidget.cpp hmi/SettingsWidget.h hmi/SettingsWidget.ui
        hmi/PathSettingsWidget.cpp hmi/PathSettingsWidget.h hmi/PathSettingsWidget.ui
        hmi/SignedListWidget.cpp hmi/SignedListWidget.h
    )

    target_compile_definitions(ngPost PRIVATE
        __USE_HMI__
    )

    target_link_libraries(ngPost PRIVATE
        Qt6::Gui
    )
endif()

if((use_hmi) AND ((QT_VERSION_MAJOR GREATER 4)))
    target_link_libraries(ngPost PRIVATE
        Qt6::Widgets
    )
endif()

if(NOT use_hmi)
    set_target_properties(ngPost PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_link_libraries(ngPost PRIVATE
        # Remove: gui
    )
endif()

# Windows specific settings
if(WIN32)
    set_target_properties(ngPost PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_link_libraries(ngPost PRIVATE
        advapi32
        user32
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
        COMMAND "${QT_ROOT_DIR}/bin/windeployqt.exe" --verbose 0 "$<TARGET_FILE:${PROJECT_NAME}>"
    )
endif()

# Debug specific settings
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(ngPost PRIVATE
        -LOG_CONNECTION_ERRORS_BEFORE_EMIT_SIGNALS
        -LOG_NEWS_DATA
        -__SAVE_ARTICLES__
        LOG_CONNECTION_STEPS
        LOG_CONSTRUCTORS
        LOG_NEWS_AUTH
        **DEBUG**
    )
endif()

# Release specific settings
if(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(ngPost PRIVATE
        QT_NO_DEBUG_OUTPUT
    )
endif()

# Install targets
install(TARGETS ngPost
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Generate deploy script
qt_generate_deploy_app_script(
    TARGET ngPost
    FILENAME_VARIABLE deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

# Install deploy script
install(SCRIPT ${deploy_script})