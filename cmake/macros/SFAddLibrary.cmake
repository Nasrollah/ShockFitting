##############################################################################
# macro for adding a library in the project
##############################################################################
MACRO( SF_ADD_LIBRARY LIBNAME )

  # option to build it or not
  OPTION ( SF_BUILD_${LIBNAME} "Build the ${LIBNAME} library" ON )

  # by default libraries are not part of the kernel
  IF ( NOT DEFINED ${LIBNAME}_kernellib )
    SET ( ${LIBNAME}_kernellib OFF )
  ELSE()
  	# add the library to the list of kernel libs
		IF ( ${LIBNAME}_kernellib )
  		SF_CACHE_LIST_APPEND ( SF_KERNEL_LIBS ${LIBNAME} )
		ENDIF()
		# and mark the option advanced
		MARK_AS_ADVANCED ( SF_BUILD_${LIBNAME} )
  ENDIF()

  # should we install these headers ?
  # default for kernel libs is to install
  # default for plugin libs is not to install
  OPTION ( SF_BUILD_${LIBNAME}_API "Publish the ${LIBNAME} library API" ${LIBNAME}_kernellib )
  MARK_AS_ADVANCED( SF_BUILD_${LIBNAME}_API )
  # set this library on
  # SET ( SF_BUILD_${LIBNAME}_API ON )

  # library is shared or static?
  IF ( BUILD_SHARED_LIBS )
    SET ( ${LIBNAME}_buildtype SHARED )
  ELSE ()
    SET ( ${LIBNAME}_buildtype STATIC )
  ENDIF ()

  # add to list of local libs
  LIST ( APPEND SF_LOCAL_LIBNAMES ${LIBNAME} )

#   SF_DEBUG_VAR(SF_MODULES_LIST)

  # check if all required modules are present
  SET ( ${LIBNAME}_all_mods_pres ON )
  FOREACH ( reqmod ${${LIBNAME}_requires_mods} )
    LIST ( FIND SF_MODULES_LIST ${reqmod} pos )
    IF ( ${pos} EQUAL -1 )
      SET ( ${LIBNAME}_all_mods_pres OFF )
      IF ( SF_BUILD_${LIBNAME} )
          LOGVERBOSE ( "\# lib [${LIBNAME}] requires module [${reqmod}] which is not present")
      ENDIF()
    ENDIF ()
  ENDFOREACH ( reqmod ${${LIBNAME}_requires_mods} )

  IF (SF_BUILD_${LIBNAME} AND ${LIBNAME}_all_mods_pres)
    SET ( ${LIBNAME}_will_compile ON )
  ELSE()
    SET ( ${LIBNAME}_will_compile OFF )
  ENDIF()

  SET ( ${LIBNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )
  SET ( SF_COMPILES_${LIBNAME} ${${LIBNAME}_will_compile} CACHE INTERNAL "" FORCE )

  LOGVERBOSE ("lib_${LIBNAME} = ${${LIBNAME}_will_compile}")

  # compile if selected and all required modules are present
  IF (${LIBNAME}_will_compile)

    # add include dirs if defined
    IF( DEFINED ${LIBNAME}_includedirs )
      INCLUDE_DIRECTORIES(${${LIBNAME}_includedirs})
    ENDIF()

    SF_SEPARATE_SOURCES("${${LIBNAME}_files}" ${LIBNAME})

    SOURCE_GROUP ( Headers FILES ${${LIBNAME}_headers} )
    SOURCE_GROUP ( Sources FILES ${${LIBNAME}_sources} )

    LOG ( "    LIB : [${LIBNAME}]" )

    ADD_LIBRARY(${LIBNAME} ${${LIBNAME}_buildtype} ${${LIBNAME}_sources} ${${LIBNAME}_headers})

    SET_TARGET_PROPERTIES ( ${LIBNAME} PROPERTIES LINK_FLAGS "${SF_LIBRARY_LINK_FLAGS}" )
    SET_TARGET_PROPERTIES ( ${LIBNAME} PROPERTIES DEFINE_SYMBOL ${LIBNAME}_EXPORTS )

    # add installation paths
    INSTALL( TARGETS ${LIBNAME}
      RUNTIME DESTINATION ${SF_INSTALL_BIN_DIR}
      LIBRARY DESTINATION ${SF_INSTALL_LIB_DIR}
      ARCHIVE DESTINATION ${SF_INSTALL_LIB_DIR}
    )

    # install headers for the libraries but
    # check if this library headers should be installed with the API
    IF(SF_BUILD_${LIBNAME}_API)
      # replace the current directory with target
      STRING ( REPLACE ${CMAKE_BINARY_DIR} ${SF_INSTALL_INCLUDE_DIR} LIBNAME_INSTALL_HEADERS ${CMAKE_CURRENT_BINARY_DIR} )
      STRING ( REPLACE ShockFitting/src     ShockFitting LIBNAME_INSTALL_HEADERS ${LIBNAME_INSTALL_HEADERS} )
      # STRING ( REPLACE ShockFitting/plugins ShockFitting LIBNAME_INSTALL_HEADERS ${LIBNAME_INSTALL_HEADERS} )
      # STRING ( REPLACE ShockFitting/extra   ShockFitting LIBNAME_INSTALL_HEADERS ${LIBNAME_INSTALL_HEADERS} )
      INSTALL(FILES ${${LIBNAME}_headers} DESTINATION ${LIBNAME_INSTALL_HEADERS})
    ENDIF()

    # if mpi was found add it to the libraries
    IF   (SF_HAVE_MPI AND NOT SF_HAVE_MPI_COMPILER)
    #           MESSAGE ( STATUS "${APPNAME} links to ${MPI_LIBRARIES}" )
              TARGET_LINK_LIBRARIES ( ${LIBNAME} ${MPI_LIBRARIES} )
    ENDIF()

    # add ShockFitting internal dependency libraries if defined
    IF( DEFINED ${LIBNAME}_libs )
		  IF ( SF_ENABLE_INTERNAL_DEPS )
				# MESSAGE ( STATUS "${LIBNAME} has ${${LIBNAME}_libs}}" )
				TARGET_LINK_LIBRARIES ( ${LIBNAME} ${${LIBNAME}_libs} )
		  ENDIF()
    ENDIF()

    # add external dependency libraries if defined
    IF( DEFINED ${LIBNAME}_libs )
			#	MESSAGE ( STATUS "${LIBNAME} has ${${LIBNAME}_libs}}" )
      TARGET_LINK_LIBRARIES ( ${LIBNAME} ${${LIBNAME}_libs} )
    ENDIF()

    # only add link in dso library if building shared libs
    IF (BUILD_SHARED_LIBS)
      GET_TARGET_PROPERTY(LIB_LOCNAME ${LIBNAME} LOCATION)
      SET(DSO_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}${LIBNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}${LIB_SUFFIX})
      IF ( UNIX )
        ADD_CUSTOM_COMMAND(
          TARGET ${LIBNAME}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E remove ${ShockFitting_DSO_DIR}/${DSO_LIB_NAME}
          COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIB_LOCNAME} ${ShockFitting_DSO_DIR}/${DSO_LIB_NAME}
        )
      ELSE()
        ADD_CUSTOM_COMMAND(
          TARGET ${LIBNAME}
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E remove ${ShockFitting_DSO_DIR}/${DSO_LIB_NAME}
          COMMAND ${CMAKE_COMMAND} -E copy ${LIB_LOCNAME} ${ShockFitting_DSO_DIR}/${DSO_LIB_NAME}
        )
      ENDIF()
    ENDIF()

	  # if not kernel lib and static is set 
	  # then this lib will be added to the list of kernel libs
	  IF ( NOT ${LIBNAME}_kernellib AND SF_ENABLE_STATIC )
	      SF_CACHE_LIST_APPEND ( SF_KERNEL_STATIC_LIBS ${LIBNAME} )
	  ENDIF ()

  ENDIF ()

  GET_TARGET_PROPERTY ( ${LIBNAME}_LINK_LIBRARIES  ${LIBNAME} LINK_LIBRARIES )
  
  # log some info about the library
  LOGFILE("${LIBNAME} : [${SF_BUILD_${LIBNAME}}]")
  LOGFILE("${LIBNAME} : [${${LIBNAME}_will_compile}]")
  LOGFILE("${LIBNAME} installdir      : [${LIBNAME_INSTALL_HEADERS}]")
  LOGFILE("${LIBNAME}_dir             : [${${LIBNAME}_dir}]")
  LOGFILE("${LIBNAME}_kernellib       : [${${LIBNAME}_kernellib}]")
  LOGFILE("${LIBNAME}_includedirs     : [${${LIBNAME}_includedirs}]")
  LOGFILE("${LIBNAME}_libs            : [${${LIBNAME}_libs}]")
  LOGFILE("${LIBNAME}_all_mods_pres   : [${${LIBNAME}_all_mods_pres}]")
  LOGFILE("${LIBNAME}_requires_mods   : [${${LIBNAME}_requires_mods}]")
  LOGFILE("${LIBNAME}_sources         : [${${LIBNAME}_sources}]")
  LOGFILE("${LIBNAME}_LINK_LIBRARIES  : [${${LIBNAME}_LINK_LIBRARIES}]")

ENDMACRO( SF_ADD_LIBRARY )
##############################################################################
