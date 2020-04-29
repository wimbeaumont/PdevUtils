cmake_minimum_required(VERSION 2.8)

project( PdevUtils )

# cmake file to  generate the make files to build the executables in this project 


# usefull for debug cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON


#generate the scpi lib 
# the path could be simple ../scpi-parser/*.c but in this way it is easy to copy it for other projects 
file(GLOB_RECURSE scpiliblist  ../../PdevUtils/scpi-parser/*.c  ../../PdevUtils/scpi-parser/*.h )
# use static libs , but this is not a restriction , just easier to port , so you can set it to SHARED
add_library(scpiparser STATIC  ${scpiliblist})
target_include_directories(scpiparser  PUBLIC  ../../PdevUtils/scpi-parser/inc )


#generate (all) application specific lib 
file(GLOB_RECURSE applliblist  ../../PdevUtils/*/lib/*.cpp  ../../PdevUtils/*/lib/*.h )
#message("${applliblist}")
add_library(applib STATIC  ${applliblist})
target_include_directories(applib  PUBLIC  ../../PdevUtils/scpi-parser/inc )



# collect all main programs 
file(GLOB_RECURSE ALLPROGS  ../*/src/*.cpp)

#message("${ALLPROGS}")

## build the appl include directories , 

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list ../../PdevUtils/*/lib/*.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES ( incl_dirs)
message("${incl_dirs}")

## add scpiparser include files  but seems already in so skip
#set ( incl_dirs ../scpiparser/inc ${incl_dirs}) 


LIST(REMOVE_DUPLICATES incl_dirs)

#message("${incl_dirs}")

foreach ( testprog ${ALLPROGS}) 
	#string( REPLACE ".cpp" "" testname ${testprog} )
	get_filename_component(testname  ${testprog} NAME_WE )
	add_executable( ${testname}  ${testprog}  )
	#message("${testname}")
	# just add some compiler flags
	#target_compile_options(${testname}  PUBLIC -std=c++1y -Wall )
	target_compile_options(${testname}  PUBLIC -g -Wall )
	target_include_directories(${testname} PUBLIC  ${incl_dirs}  )
	target_link_libraries( ${testname} scpiparser applib)
endforeach ( testprog ${allprogs}) 
###############################################################################
## testing ####################################################################
###############################################################################



