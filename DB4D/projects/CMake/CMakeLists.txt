

set(DB4DIncludeDir  ${DB4DRoot}/Headers ${DB4DRoot}/Sources PARENT_SCOPE)


file(GLOB Sources ${DB4DRoot}/Sources/*.cpp
                  ${DB4DRoot}/Sources/Enterprise/*.cpp)

list(REMOVE_ITEM Sources ${DB4DRoot}/Sources/JS4D_oldPart.cpp)



include_directories(${ComponentsRoot}
	${DB4DRoot}/Headers
	${DB4DRoot}/Sources
	${HttpServerIncludeDir}
	${IcuIncludeDir}
	${JsCoreIncludeDir}
	${KernelIPCIncludeDir}
	${KernelIncludeDir}
	${SecurityManagerIncludeDir}
	${ServerNetIncludeDir}
	${XBoxRoot})


#add_library(DB4DStatic STATIC EXCLUDE_FROM_ALL ${Sources})

#jmo - Temporary stuff !
add_definitions(-DDB4D_AS_DLL=1)

add_library(DB4D SHARED ${Sources})

target_link_libraries(DB4D
	TextCore JavaScript Kernel KernelIPC ServerNet Xml)

#target_link_libraries(DB4DStatic
#	TextCoreStatic JavaScriptStatic KernelStatic KernelIPCStatic ServerNetStatic XmlStatic)

