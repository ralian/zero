cmake_minimum_required(VERSION 3.22)
project(zero VERSION 0.0 LANGUAGES C)

file(GLOB_RECURSE ZERO_SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

add_executable(zero ${ZERO_SOURCES})
