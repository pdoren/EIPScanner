include_directories(${CMAKE_SOURCE_DIR}/src)

function(cxx_library name)
  # Generate debug library name with a postfix.
  set_target_properties(${name}
    PROPERTIES
    DEBUG_POSTFIX "d")
  # Set the output directory for build artifacts
  set_target_properties(${name}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  # make PDBs match library name
  get_target_property(pdb_debug_postfix ${name} DEBUG_POSTFIX)
  set_target_properties(${name}
    PROPERTIES
    PDB_NAME "${name}"
    PDB_NAME_DEBUG "${name}${pdb_debug_postfix}"
    COMPILE_PDB_NAME "${name}"
    COMPILE_PDB_NAME_DEBUG "${name}${pdb_debug_postfix}")
endfunction()

set(SOURCE_FILES
        cip/connectionManager/ForwardCloseRequest.cpp
        cip/connectionManager/ForwardOpenRequest.cpp
        cip/connectionManager/LargeForwardOpenRequest.cpp
        cip/connectionManager/ForwardOpenResponse.cpp
        cip/connectionManager/NetworkConnectionParametersBuilder.cpp

        cip/CipRevision.cpp
        cip/EPath.cpp
        cip/MessageRouterRequest.cpp
        cip/MessageRouterResponse.cpp
        cip/CipString.cpp

        eip/CommonPacket.cpp
        eip/CommonPacketItem.cpp
        eip/CommonPacketItemFactory.cpp
        eip/EncapsPacket.cpp
        eip/EncapsPacketFactory.cpp

        fileObject/FileObjectEmptyState.cpp
        fileObject/FileObjectLoadedState.cpp
        fileObject/FileObjectNonExistentState.cpp
        fileObject/FileObjectState.cpp
        fileObject/FileObjectUploadInProgressState.cpp

        sockets/BaseSocket.cpp
        sockets/EndPoint.cpp
        sockets/TCPSocket.cpp
        sockets/UDPBoundSocket.cpp
        sockets/UDPSocket.cpp

        utils/Logger.cpp
        utils/Buffer.cpp

        vendor/ra/powerFlex525/DPIFaultManager.cpp
        vendor/ra/powerFlex525/DPIFaultObject.cpp
        vendor/ra/powerFlex525/DPIFaultCode.cpp
        vendor/ra/powerFlex525/DPIFaultParameter.cpp
	vendor/yaskawa/mp3300iec/Yaskawa_EPath.cpp
	vendor/yaskawa/mp3300iec/Yaskawa_MessageRouter.cpp
	vendor/yaskawa/mp3300iec/Yaskawa_MessageRouterRequest.cpp

        BaseObject.cpp
        ConnectionManager.cpp
        DiscoveryManager.cpp
        FileObject.cpp
        IdentityObject.cpp
        IOConnection.cpp
        MessageRouter.cpp
        ParameterObject.cpp
        SessionInfo.cpp)

add_library(EIPScanner SHARED ${SOURCE_FILES})
add_library(EIPScannerS STATIC ${SOURCE_FILES})

if(WIN32)
  target_link_libraries(EIPScanner wsock32 ws2_32)
  target_link_libraries(EIPScannerS wsock32 ws2_32)
endif()

cxx_library(EIPScanner)
cxx_library(EIPScannerS)

set_target_properties(
        EIPScanner
        PROPERTIES
        VERSION ${EIPSCANNER_FULL_VERSION}
        SOVERSION ${EIPSCANNER_MAJOR_VERSION})
		
if(WIN32)
  set_target_properties(EIPScanner PROPERTIES ENABLE_EXPORTS 1)
  set_target_properties(EIPScanner PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif()

target_compile_definitions(EIPScanner
    PUBLIC
    GTEST_LINKED_AS_SHARED_LIBRARY=1)

install(TARGETS EIPScanner EIPScannerS
		RUNTIME
			DESTINATION bin
        LIBRARY
            DESTINATION lib
        ARCHIVE
            DESTINATION lib)
			
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	# Install PDBs
	get_target_property(t_pdb_name EIPScanner COMPILE_PDB_NAME)
	get_target_property(t_pdb_name_debug EIPScanner COMPILE_PDB_NAME_DEBUG)
	get_target_property(t_pdb_output_directory EIPScanner PDB_OUTPUT_DIRECTORY)
	install(FILES
	  "${t_pdb_output_directory}/\${CMAKE_INSTALL_CONFIG_NAME}/$<$<CONFIG:Debug>:${t_pdb_name_debug}>$<$<NOT:$<CONFIG:Debug>>:${t_pdb_name}>.pdb"
	  DESTINATION bin
	  OPTIONAL)
endif()

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/
        DESTINATION include/EIPScanner
        FILES_MATCHING PATTERN "*.h*")
