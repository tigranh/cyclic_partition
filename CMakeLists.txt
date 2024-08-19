
cmake_minimum_required (VERSION 3.23)

project (cyclic_partition_demo)

set(CMAKE_CXX_STANDARD 14)

set (HEADER_FILES
	partition_cyclic.hpp 
	partition_hoare.hpp 
	large_object.hpp 
	)
	
set (SOURCE_FILES
	main.cpp
	)
	
add_executable ( cyclic_partition_demo ${HEADER_FILES} ${SOURCE_FILES} )
