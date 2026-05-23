cmake_minimum_required(VERSION 3.20)
project(BlackHoleSimulator) # Możesz zmienić nazwę projektu

# Ustawiamy standard C++ (w terminalu użyłeś c++20, więc tu też go ustawiamy)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. NAPRAWA BŁĘDU Z <Metal/Metal.hpp>
# Mówimy kompilatorowi, żeby szukał plików nagłówkowych w folderze metal-cpp
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp)

# 2. PLIKI ŹRÓDŁOWE
# Musimy podać główny plik oraz nasz tajny plik z implementacją!
set(SOURCE_FILES
        main.cpp
        metal_impl.cpp
        metal_impl.cpp
)

# 3. TWORZENIE PLIKU WYKONYWALNEGO
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# 4. PODŁĄCZENIE FRAMEWORKÓW APPLE
# Zastępuje to Twoje -framework Metal -framework Foundation z terminala
target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
)

# 5. MAGIA CLIONA: Automatyczna kompilacja shadera!
# Zawsze po zbudowaniu main.cpp, CLion sam skompiluje shader.metal do shader.metallib
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND xcrun -sdk macosx metal -c ${CMAKE_CURRENT_SOURCE_DIR}/shader.metal -o ${CMAKE_CURRENT_BINARY_DIR}/shader.air
        COMMAND xcrun -sdk macosx metallib ${CMAKE_CURRENT_BINARY_DIR}/shader.air -o ${CMAKE_CURRENT_BINARY_DIR}/shader.metallib
        COMMENT "Kompilowanie Apple Metal Shader..."
)