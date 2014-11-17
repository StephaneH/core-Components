set(ZipIncludeDir   ${ZipRoot}/Headers ${ZipRoot}/Interfaces CACHE STRING "ZipIncludeDir")


file(GLOB Sources ${ZipRoot}/Sources/*.cpp)


include_directories(${ComponentsRoot}
	${IcuIncludeDir}
	${Interfaces}
	${XBoxRoot}
	${ZLibIncludeDir}
	${ZipRoot}/Headers 
	${ZipRoot}/Interfaces)

add_library(Zip SHARED ${Sources})

target_link_libraries(Zip Kernel KernelIPC ${LibZipLibs} ${ZLibLibs})
