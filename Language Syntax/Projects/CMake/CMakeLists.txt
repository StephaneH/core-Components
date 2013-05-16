cmake_minimum_required(VERSION 2.8)
project(LANGUAGESYNTAX)


set(LanguageSyntaxIncludeDir	${LanguageSyntaxRoot}/Sources PARENT_SCOPE)


file(GLOB Sources ${LanguageSyntaxRoot}/Sources/*.cpp)

add_library(LanguageSyntax SHARED ${Sources})


include_directories(${ComponentsRoot}
  ${DB4DIncludeDir}
  ${IcuIncludeDir}
  ${JsCoreIncludeDir}
  ${LanguageSyntaxRoot}
  ${SrcDir}
  ${XBoxRoot}
  ${XmlIncludeDir})


target_link_libraries(LanguageSyntax JavaScript Kernel KernelIPC)
