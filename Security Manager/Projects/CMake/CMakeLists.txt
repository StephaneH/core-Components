cmake_minimum_required(VERSION 2.8)
project(SECURITYMANAGER)


set(SecurityManagerIncludeDir	${SecurityManagerRoot}/Interfaces PARENT_SCOPE)

file(GLOB Sources ${SecurityManagerRoot}/Sources/*.cpp)

add_library(SecurityManager SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${IcuIncludeDir}
  ${JsCoreIncludeDir}
  ${SecurityManagerRoot}/Headers
  ${SecurityManagerRoot}/Interfaces
  ${XBoxRoot})

#add_dependencies(SecurityManager Icu Kernel KernelIPC )
