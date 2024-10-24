cmake_minimum_required(VERSION 3.12)

# Ausgabe der CMake-Version
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "CMake executable path: ${CMAKE_COMMAND}")


project(Time2Stand VERSION 0.1 LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets SerialPort)

#Instruct CMake to run uic, moc, rcc automatically when needed.
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") # Hide console window in release build

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# target_link_libraries(foo PRIVATE nlohmann_json::nlohmann_json)

# This will find the Qt5 files. Equal to Qt += widgets, ...
# If you get an error message by a static kit here, the CMAKE_PREFIX_PATH is not up-to-date.
# Please remove your CMakeLists.txt.user in this case and add the Kits again.
find_package(Qt5 COMPONENTS Core  REQUIRED)
find_package(Qt5 COMPONENTS Gui  REQUIRED)
find_package(Qt5 COMPONENTS Widgets  REQUIRED)

# Check for static Kit
find_package(Qt5 COMPONENTS Core REQUIRED)
get_target_property(APP_TARGET_TYPE Qt5::Core TYPE)
if(APP_TARGET_TYPE STREQUAL STATIC_LIBRARY)
    message("Using static Kit")
else ()
    message("Using dynamic Kit")
    option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
endif()


# QMake defined this implicity
add_compile_definitions(UNICODE)
add_compile_definitions(_UNICODE)

set(HEADERS
    device.h
    device_errorcode.h
    observer.h
    positiontracker.h
    window.h
)

set(SOURCES
    device.cpp
    device_errorcode.cpp
    main.cpp
    positiontracker.cpp
    window.cpp
)

set(RESOURCES
    time2stand.qrc
)

# Füge die Ressourcen hinzu
qt5_add_resources(RESOURCES_RCC ${RESOURCES})

# Erstelle die ausführbare Datei
add_executable(Time2Stand ${SOURCES} ${HEADERS} ${RESOURCES_RCC}
    appsettings.h appsettings.cpp)

# Link static if using a static kit
if(APP_TARGET_TYPE STREQUAL STATIC_LIBRARY)
    #Static start
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_START_STATIC 1)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_SEARCH_END_STATIC 1)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})

    #Set Linker flags
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# Verlinke die Qt-Bibliotheken
target_link_libraries(Time2Stand Qt5::Core Qt5::Widgets nlohmann_json::nlohmann_json)

# Installationsanweisungen (optional)
install(TARGETS Time2Stand
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/widgets/desktop/systray
)
