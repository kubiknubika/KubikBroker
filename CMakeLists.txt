cmake_minimum_required(VERSION 3.10)
project(KubikBroker)

# Стандарт C++20 (Сеньорский выбор)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Где искать заголовки
include_directories(include)

# Собираем исполняемый файл из всех .cpp в папке src
file(GLOB SOURCES "src/*.cpp")
add_executable(KubikBroker ${SOURCES})