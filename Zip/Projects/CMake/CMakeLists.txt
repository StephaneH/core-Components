

set(ZipIncludeDir   ${ZipRoot}/Headers ${ZipRoot}/Interfaces PARENT_SCOPE)


file(GLOB Sources ${ZipRoot}/Sources/*.cpp)

add_library(Zip SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${IcuIncludeDir}
  ${Interfaces}
  ${XBoxRoot}
  ${ZLibIncludeDir}
  ${ZipRoot}/Headers 
  ${ZipRoot}/Interfaces)


target_link_libraries(Zip Kernel KernelIPC LibZip ZLib)
