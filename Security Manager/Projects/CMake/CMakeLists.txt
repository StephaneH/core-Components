

set(SecurityManagerIncludeDir	${SecurityManagerRoot}/Interfaces PARENT_SCOPE)

file(GLOB Sources ${SecurityManagerRoot}/Sources/*.cpp)

add_library(SecurityManager SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${IcuIncludeDir}
  ${JsCoreIncludeDir}
  ${SecurityManagerRoot}/Headers
  ${SecurityManagerRoot}/Interfaces
  ${XBoxRoot})


target_link_libraries(SecurityManager JavaScript Kernel KernelIPC)
