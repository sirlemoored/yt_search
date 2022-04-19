cmake_minimum_required(VERSION 3.19)
project(yt)

set(PROJECT_EXEC_NAME yt)

if(MSVC)
    add_compile_options(/bigobj)
endif()

set(OPENSSL_USE_STATIC_LIBS FALSE)

if(${OPENSSL_ROOT_DIR})
    message(FATAL_ERROR "OpenSSL root not specified.")
else()
    find_package(OpenSSL REQUIRED)
endif()

if(${BOOST_ROOT}})
    message(FATAL_ERROR "Boost root not specified.")
else()
    find_package(Boost REQUIRED)
endif()

set(FILES_SOURCE
    src/main.cpp
)

add_executable(${PROJECT_EXEC_NAME} ${FILES_SOURCE})
set_property(TARGET ${PROJECT_EXEC_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${PROJECT_EXEC_NAME} 
                            PUBLIC 
                            include
                            ${Boost_INCLUDE_DIRS}
                            ${OPENSSL_INCLUDE_DIRS}
                            extern/nlohmann/include)

target_link_libraries(${PROJECT_EXEC_NAME}
                       PUBLIC
                       ws2_32
                       OpenSSL::SSL
                       OpenSSL::Crypto
                       )

