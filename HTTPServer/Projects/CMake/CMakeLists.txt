cmake_minimum_required(VERSION 2.8)
project(HTTPSERVER)


set(HTTPServerIncludeDir    ${HTTPServerRoot}/Headers ${HTTPServerRoot}/Interfaces PARENT_SCOPE)


file(GLOB Sources ${HTTPServerRoot}/Sources/*.cpp)

add_library(HTTPServer SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${HTTPServerRoot}/Headers
  ${HTTPServerRoot}/Interfaces
  ${IcuIncludeDir}
  ${ServerNetIncludeDir}
  ${XBoxRoot}
  ${XmlIncludeDir})


target_link_libraries(HTTPServer Kernel KernelIPC ServerNet Xml)
