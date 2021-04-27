cmake_minimum_required(VERSION 3.10)
enable_testing()

option(ENABLE_ASAN "Enable ASan" OFF)

if(ENABLE_ASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-variable -Wno-initializer-overrides")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(NCURSES REQUIRED IMPORTED_TARGET ncursesw)
pkg_check_modules(FORM REQUIRED IMPORTED_TARGET formw)
pkg_check_modules(MENU REQUIRED IMPORTED_TARGET menuw)

find_program(SCDOC scdoc)

add_executable(lemon lemon.c)

add_custom_command(
    OUTPUT calculator.c calculator.h
    COMMAND lemon "-d${CMAKE_CURRENT_BINARY_DIR}"
                  "-T${CMAKE_SOURCE_DIR}/lempar.c"
                  "${CMAKE_SOURCE_DIR}/calculator.lemon"
)

add_executable(hexxed calculator.c main.c buffer.c buffer.h panes.c panes.h render.c render.h)
target_include_directories(hexxed PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(hexxed PkgConfig::NCURSES PkgConfig::FORM PkgConfig::MENU PkgConfig::GLIB)
install(TARGETS hexxed DESTINATION bin)

add_executable(calculator_test calculator_test.c calculator.c buffer.c)
target_include_directories(calculator_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(calculator_test PkgConfig::GLIB)
add_test(calculator calculator_test)

if(SCDOC)
  add_subdirectory(man)
endif()
