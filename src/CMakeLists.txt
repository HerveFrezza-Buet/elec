# Generate the list of source files
# using GLOB does not RECURSE

file(
	GLOB
	headers_files
	*.hpp
)

file(
	GLOB
	py_files
	elec-*.py
)


# Installation targets
install (FILES ${headers_files} 
	 DESTINATION ${INCLUDE_INSTALL_DIR}
	 COMPONENT devel)

install (FILES ${py_files} 
	 DESTINATION bin
	 PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
	 COMPONENT binary)
