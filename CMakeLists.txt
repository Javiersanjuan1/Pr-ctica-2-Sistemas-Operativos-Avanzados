cmake_minimum_required(VERSION 3.16)
project(P2_Computacion_Matricial C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Optimizaciones recomendadas para medir rendimiento realista
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-O2 -Wall -Wextra -Wpedantic)

# Hilos
find_package(Threads REQUIRED)

add_executable(procSecuencial procSecuencial.c)
add_executable(hebSecuencial  hebSecuencial.c)
add_executable(hebConcurrente hebConcurrente.c)
add_executable(procConcurrente procConcurrente.c)

# Enlaces
target_link_libraries(hebSecuencial  PRIVATE Threads::Threads)
target_link_libraries(hebConcurrente PRIVATE Threads::Threads)
# En muchos Linux modernos no hace falta -lrt, pero lo añadimos si existe
include(CheckLibraryExists)
check_library_exists(rt clock_gettime "" HAVE_LIBRT)
if(HAVE_LIBRT)
    target_link_libraries(procConcurrente PRIVATE rt)
endif()
