# Make sure the compiler can find include files from our library.
include_directories(${CMAKE_SOURCE_DIR}/src)

file(
	GLOB
	USAGE_TESTS
	*.cpp
)

# loop over the list of tests
foreach(f ${USAGE_TESTS})
    get_filename_component(testName ${f} NAME_WE) 
    add_executable (${testName} ${f}) 
    set_target_properties(${testName} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${testName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS}" LINK_FLAGS "${PROJECT_ALL_LDFLAGS}")

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${testName} 
            DESTINATION bin
            RENAME ${CMAKE_PROJECT_NAME}-${testName}
	    COMPONENT binary)
endforeach(f)
