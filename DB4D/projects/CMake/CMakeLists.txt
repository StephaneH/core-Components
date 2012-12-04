cmake_minimum_required(VERSION 2.8)
project(DB4D)


#Todo : Find the best optimisation level and understand...
# - why we have undefined symbols at run time with -O2 and above (solved with -fno-inline)
# - why we have corrupted files with -O3


set(CMAKE_CXX_FLAGS_BETA    "-g -O2 -fno-inline")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -fno-inline")


set(DB4DIncludeDir  ${DB4DRoot}/Headers ${DB4DRoot}/Sources PARENT_SCOPE)


file(GLOB Sources ${DB4DRoot}/Sources/*.cpp)

list(REMOVE_ITEM Sources ${DB4DRoot}/Sources/JS4D_oldPart.cpp)

set_source_files_properties(${DB4DRoot}/Sources/4DDBHeaders.h PROPERTIES LANGUAGE "CXX")

list(INSERT Sources 0 ${DB4DRoot}/Sources/4DDBHeaders.h)


add_library(DB4D SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${HttpServerIncludeDir}
  ${IcuIncludeDir}
  ${JsCoreIncludeDir}
  ${KernelIPCIncludeDir}
  ${KernelIncludeDir}
  ${DB4DRoot}/Headers
  ${DB4DRoot}/Sources
  ${SecurityManagerIncludeDir}
  ${ServerNetIncludeDir}
  ${XBoxRoot})



add_dependencies(DB4D Javascript JsCore Kernel KernelIPC ServerNet Xerces Xml)
