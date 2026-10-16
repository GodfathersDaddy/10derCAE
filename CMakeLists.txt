cmake_minimum_required(VERSION 3.25)

project(10derCAE VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOGEN_PARALLEL 1)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg Test)
find_package(OpenCASCADE CONFIG REQUIRED)

add_library(tendercae_core
    src/AppTypes.h
    src/Document.cpp
    src/Document.h
    src/GeometryKernel.cpp
    src/GeometryKernel.h
    src/I18n.cpp
    src/I18n.h
    src/SketchModel.cpp
    src/SketchModel.h
    src/WorkspaceCatalog.cpp
    src/WorkspaceCatalog.h
)
target_include_directories(tendercae_core PUBLIC src ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(tendercae_core PUBLIC Qt6::Core ${OpenCASCADE_LIBRARIES})

add_library(tendercae_ui
    src/MainWindow.cpp
    src/MainWindow.h
    src/OcctViewport.cpp
    src/OcctViewport.h
    src/RibbonWidget.cpp
    src/RibbonWidget.h
    src/SketchCanvas.cpp
    src/SketchCanvas.h
)
target_link_libraries(tendercae_ui PUBLIC tendercae_core Qt6::Gui Qt6::Widgets Qt6::Svg)

file(GLOB V2_ICONS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/v2/light/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/v2/dark/*.svg"
)
set(APP_RESOURCES ${V2_ICONS}
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/i18n/de.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/i18n/en.json"
)
qt_add_resources(tendercae_core "tendercae_resources"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES ${APP_RESOURCES}
)

add_executable(10derCAE WIN32 src/main.cpp)
target_link_libraries(10derCAE PRIVATE tendercae_ui)

foreach(target tendercae_core tendercae_ui 10derCAE)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

include(CTest)
if(BUILD_TESTING)
    add_executable(document_tests tests/DocumentTests.cpp)
    target_link_libraries(document_tests PRIVATE tendercae_core)
    add_test(NAME document_tests COMMAND document_tests)

    add_executable(workspace_tests tests/WorkspaceTests.cpp)
    target_link_libraries(workspace_tests PRIVATE tendercae_core)
    add_test(NAME workspace_tests COMMAND workspace_tests)

    add_executable(geometry_tests tests/GeometryTests.cpp)
    target_link_libraries(geometry_tests PRIVATE tendercae_core)
    add_test(NAME geometry_tests COMMAND geometry_tests)

    add_executable(sketch_tests tests/SketchTests.cpp)
    target_link_libraries(sketch_tests PRIVATE tendercae_core)
    add_test(NAME sketch_tests COMMAND sketch_tests)

    add_executable(sketch_canvas_tests tests/SketchCanvasTests.cpp)
    target_link_libraries(sketch_canvas_tests PRIVATE tendercae_ui Qt6::Test)
    add_test(NAME sketch_canvas_tests COMMAND sketch_canvas_tests)
    set_tests_properties(sketch_canvas_tests PROPERTIES
        ENVIRONMENT "QT_PLUGIN_PATH=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
endif()

install(TARGETS 10derCAE RUNTIME DESTINATION .)
install(FILES LICENSE THIRD_PARTY_NOTICES.md
    resources/icons/v2/LICENSE.md
    resources/icons/v2/LICENSE-CC-BY-NC-SA-4.0.txt
    resources/icons/v2/NOTICE.txt
    resources/icons/v2/provenance.json
    DESTINATION licenses)

