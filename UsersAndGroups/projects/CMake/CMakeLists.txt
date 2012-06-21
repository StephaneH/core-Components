cmake_minimum_required(VERSION 2.8)
project(USERSANDGROUPS)


set(UsersAndGroupsIncludeDir	${UsersAndGroupsRoot}/Sources PARENT_SCOPE)


file(GLOB Sources ${UsersAndGroupsRoot}/Sources/*.cpp)

add_library(UsersAndGroups SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${IcuIncludeDir}
  ${JsCoreIncludeDir}
  ${XBoxRoot})


#add_dependencies(UsersAndGroups Icu Kernel KernelIPC Xml)
