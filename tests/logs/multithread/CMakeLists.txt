cmake_minimum_required(VERSION 3.5)

project(test_logs_multithread LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(SU_LOGS_NOSINGLETON)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "../../../log.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE "../../..")

#install(TARGETS smallrender
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
