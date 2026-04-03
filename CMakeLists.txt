cmake_minimum_required(VERSION 3.16)

project(TypeMaster VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Find Qt modules ──────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia LinguistTools)

# ── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/gamecard.cpp
    src/applewindow.cpp
    src/spacewindow.cpp
    src/practicewindow.cpp
)

set(HEADERS
    src/mainwindow.h
    src/gamecard.h
    src/applewindow.h
    src/spacewindow.h
    src/practicewindow.h
)

# ── Translation files ─────────────────────────────────────────────────────────
set(TS_FILES
    translations/typemaster_zh_CN.ts
    translations/typemaster_en.ts
)

# ── Executable ────────────────────────────────────────────────────────────────
qt_add_executable(TypeMaster
    ${SOURCES}
    ${HEADERS}
    resources.qrc
)

# ── Translations ──────────────────────────────────────────────────────────────
qt_add_translations(TypeMaster
    TS_FILES ${TS_FILES}
    RESOURCE_PREFIX "/i18n"
)

# ── Link libraries ────────────────────────────────────────────────────────────
target_link_libraries(TypeMaster PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
)

# ── Windows: hide console window ──────────────────────────────────────────────
if(WIN32)
    set_target_properties(TypeMaster PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS TypeMaster
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
