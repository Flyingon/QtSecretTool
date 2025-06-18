cmake_minimum_required(VERSION 3.16)

project(QtSecretTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找所需的Qt组件
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Quick 
    Sql 
    Widgets
    Concurrent
)

qt_standard_project_setup(REQUIRES 6.8)

# 源文件列表
set(SOURCES
    main.cpp
    src/core/Application.cpp
    src/PasswordManager.cpp
    src/models/PasswordItem.cpp
    src/models/PasswordListModel.cpp
    src/database/DatabaseManager.cpp
    src/crypto/CryptoManager.cpp
)

# 头文件列表
set(HEADERS
    src/core/Application.h
    src/PasswordManager.h
    src/models/PasswordItem.h
    src/models/PasswordListModel.h
    src/database/DatabaseManager.h
    src/crypto/CryptoManager.h
)

qt_add_executable(appQtSecretTool
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(appQtSecretTool
    URI QtSecretTool
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/pages/AddEditPasswordPage.qml
        qml/pages/PasswordListPage.qml
        qml/pages/SettingsPage.qml
        qml/components/PasswordCard.qml
        qml/components/SearchBar.qml
        qml/components/CustomButton.qml
        qml/components/PasswordGeneratorDialog.qml
        qml/components/MasterPasswordDialog.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQtSecretTool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.flyingontemp.QtSecretTool
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQtSecretTool
    PRIVATE 
    Qt6::Core
    Qt6::Quick
    Qt6::Sql
    Qt6::Widgets
    Qt6::Concurrent
)

# 设置包含目录
target_include_directories(appQtSecretTool PRIVATE src)

include(GNUInstallDirs)
install(TARGETS appQtSecretTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
