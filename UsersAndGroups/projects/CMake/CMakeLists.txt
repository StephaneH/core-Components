

set(UsersAndGroupsIncludeDir	${UsersAndGroupsRoot}/Sources PARENT_SCOPE)


file(GLOB Sources ${UsersAndGroupsRoot}/Sources/*.cpp)

add_library(UsersAndGroups SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${IcuIncludeDir}
  ${JsCoreIncludeDir}
  ${XBoxRoot})


target_link_libraries(UsersAndGroups JavaScript Kernel KernelIPC Xml)
