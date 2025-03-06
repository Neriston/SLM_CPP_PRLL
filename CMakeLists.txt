# CMakeList.txt : CMake project for slm_cpp_mesh_divided, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
# if (POLICY CMP0141)
#   cmake_policy(SET CMP0141 NEW)
#   set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
# endif()

set(CMAKE_CXX_FLAGS "-g")
project ("slm_cpp_mesh_divided")

set(SOURCES
	"main.cpp"

	"lib/source/lean_vtk.cpp"
	"lib/source/Process.cpp"
	"lib/source/Timer.cpp"

	"code/source/BodyData.cpp"
	"code/source/Config.cpp"
	"code/source/DataContainer.cpp"
	"code/source/DataWriter.cpp"
	"code/source/Element.cpp"
	"code/source/functions.cpp"
	"code/source/Laser.cpp"
	"code/source/Mesh.cpp"
	"code/source/MeshSector.cpp"
	"code/source/Neighbours.cpp"
	"code/source/Node.cpp"
	"code/source/Processor.cpp"
	"code/source/TimeFlow.cpp"
)

# Add source to this project's executable.
add_executable (slm_cpp_mesh_divided ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET slm_cpp_mesh_divided PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
