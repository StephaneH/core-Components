

set(LanguageSyntaxIncludeDir	${LanguageSyntaxRoot}/Sources PARENT_SCOPE)


file(GLOB Sources ${LanguageSyntaxRoot}/Sources/*.cpp)



include_directories(${ComponentsRoot}
  ${DB4DIncludeDir}
  ${JsCoreIncludeDir}
  ${LanguageSyntaxRoot}
  ${SrcDir}
  ${XBoxRoot}
  ${XmlIncludeDir})

add_library(LanguageSyntax SHARED ${Sources})

target_link_libraries(LanguageSyntax JavaScript Kernel KernelIPC)
