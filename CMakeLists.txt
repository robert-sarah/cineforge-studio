cmake_minimum_required(VERSION 3.20)
project(CineForgeStudio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CINEFORGE_BUILD_GUI "Build the optional Qt 6 desktop interface" ON)

add_library(cineforge_core
    src/Project.cpp
    src/FfmpegRenderer.cpp
    src/LocalAgent.cpp
    src/LocalServices.cpp
    src/ModelCatalog.cpp
)

target_include_directories(cineforge_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(cineforge_core PRIVATE OVA_VERSION=\"${PROJECT_VERSION}\")

if (WIN32)
    target_compile_definitions(cineforge_core PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

add_executable(cineforge-cli src/main.cpp)
target_link_libraries(cineforge-cli PRIVATE cineforge_core)

if (CINEFORGE_BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets Multimedia MultimediaWidgets QUIET)
    if (Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
        add_executable(cineforge-studio
            src/gui/MainWindow.cpp
            src/gui/TimelineWidget.cpp
            src/gui/main_gui.cpp
        )
        target_link_libraries(cineforge-studio PRIVATE cineforge_core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
        target_include_directories(cineforge-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_compile_definitions(cineforge-studio PRIVATE OVA_HAS_QT=1)
    else()
        message(STATUS "Qt6 not found: only cineforge-cli will be built")
    endif()
endif()

install(TARGETS cineforge-cli RUNTIME DESTINATION bin)
if (TARGET cineforge-studio)
    install(TARGETS cineforge-studio RUNTIME DESTINATION bin)
endif()

# Les outils et la documentation sont installés sans embarquer les poids IA,
# qui restent optionnels et soumis à leurs licences respectives.
install(DIRECTORY tools/ DESTINATION share/cineforge/tools
        PATTERN "__pycache__" EXCLUDE
        PATTERN "*.pyc" EXCLUDE)
install(DIRECTORY models/ DESTINATION share/cineforge/models
        PATTERN "*.bin" EXCLUDE
        PATTERN "*.onnx" EXCLUDE
        PATTERN "*.gguf" EXCLUDE)
install(FILES README.md INSTALL.md ROADMAP.md DESTINATION share/cineforge)
install(DIRECTORY doc/ DESTINATION share/cineforge/doc)
