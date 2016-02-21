cmake_minimum_required(VERSION 2.8.12.2)

project(Torsion)

# wxWidgets ------------------------------------------------------

set(wx_components core base adv aui stc net propgrid)

if(WIN32)
	list(APPEND wx_components scintilla)
	#TODO: if not static build then copy dll files to exe directory
endif(WIN32)

find_package(wxWidgets COMPONENTS ${wx_components} REQUIRED)
include(${wxWidgets_USE_FILE})

option(wxWidgets_USE_REL_AND_DBG "" false)
#TODO: fix debug level mismatch

# GTK+ 2 ---------------------------------------------------------

if(UNIX)
	find_package(GTK2 REQUIRED)
	include_directories(${GTK2_INCLUDE_DIRS})
endif(UNIX)

# Torsion App ----------------------------------------------------

file(GLOB torision_src "Code/Torsion/*.c??")
set(SRCS ${torision_src})

if(WIN32)
	set(SRCS ${SRCS} Code/Torsion/Torsion.rc)
endif(WIN32)

add_definitions(-DMARKUP_STL)

add_executable(torsion WIN32 ${SRCS})
target_link_libraries(torision ${GTK2_LIBRARIES} ${wxWidgets_LIBRARIES})