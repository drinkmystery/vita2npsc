cmake_minimum_required(VERSION 2.8.1)

project(PkgDecrypt)
include_directories(.)
add_library(PkgDecrypt STATIC keyflate.c sfo.c)
