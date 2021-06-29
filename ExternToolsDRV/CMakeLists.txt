# Cmake script for project AddInNative
# autor: 1C

CMAKE_MINIMUM_REQUIRED(VERSION 3.6 FATAL_ERROR)

foreach(policy CMP0068)
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

PROJECT(${Addn_Native} CXX)
IF(WIN32)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/conf_data_in/AddInNative.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/AddInNative.rc)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/conf_data_in/AddInNative.def.in ${CMAKE_CURRENT_SOURCE_DIR}/AddInNative.def)
    ENABLE_LANGUAGE(RC)
ENDIF()
#----------------------------------------------------------------------------------------------------------------#
#--source files-------------------
SET(AddInNative_SRC 
    AddInNative.cpp
    AddInNative.h
    dllmain.cpp
    stdafx.cpp
    stdafx.h
    )
if (WIN32)
    LIST(APPEND AddInNative_SRC AddInNative.rc AddInNative.def)
endif()
#--source files end---------------
#----------------------------------------------------------------------------------------------------------------#
include_directories(${CMAKE_SOURCE_DIR}/../include)

SET (CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_SOURCE_DIR})
SET(AddInDef_SRC)

if (UNIX)
    if (APPLE)
        ADD_DEFINITIONS(-D__APPLE__ )
    endif()
else()
    add_definitions(-DUNICODE -DWIN32 -D_USRDLL -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -DADDINCPP_EXPORTS)
    IF(TARGET_PLATFORM_32)
        add_definitions(-D_USE_32BIT_TIME_T )
    ENDIF()
endif()
if(APPLE)
    set(BNDL ${PROJECT_NAME}Bndl)

    add_library(${BNDL} MODULE ${AddInNative_SRC})

    set_target_properties(${BNDL} PROPERTIES BUNDLE TRUE)
    set_target_properties( ${BNDL} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    SET(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")

    SET(MACOSX_BUNDLE_COPYRIGHT "Copyright ${YEAR} ${MYCOMPANY}.")
    TARGET_LINK_LIBRARIES(${BNDL} "-framework CoreFoundation")
    set_target_properties(${BNDL} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in)
    
    #Safari pkg
    SET(PKG_NAME "AddInSafOSX")
    get_target_property(BUNDLE_NAME ${BNDL} OUTPUT_NAME)

    set(MIMETYPE "${Addn_MimeType}")
    file(COPY ${CMAKE_SOURCE_DIR}/../lib/1CEAdnWebNPAPISafOSX.bundle DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle/Contents/Info.plist)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle/Contents/PlugIns)
    
    #FIXME Get BUNDLE full name
    add_custom_command(TARGET ${BNDL} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIB_DIR}/${BUNDLE_NAME}.bundle ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle/Contents/PlugIns/${BUNDLE_NAME}.bundle
        COMMENT "Copy ${BNDL}"
        MAIN_DEPENDENCY ${BNDL}
    )
    add_custom_target(saf_productbuild ALL DEPENDS ${BNDL}
        COMMAND productbuild --component ${CMAKE_CURRENT_BINARY_DIR}/1CEAdnWebNPAPISafOSX.bundle "/Library/Internet Plug-Ins/" ${CMAKE_CURRENT_BINARY_DIR}/${PKG_NAME}.pkg
        COMMENT "Create package for Safari"
        MAIN_DEPENDENCY ${BNDL}
    )
    add_custom_target( saf_productsign ALL DEPENDS saf_productbuild
        COMMAND productsign --sign "Developer ID Installer: ${SIGNCERT}"  ${CMAKE_CURRENT_BINARY_DIR}/${PKG_NAME}.pkg ${PKG_OUTPUT_DIR}/${PKG_NAME}.pkg
        COMMENT "Signing package for Safari"
        MAIN_DEPENDENCY ${BNDL}
    )
endif()

add_library(${PROJECT_NAME} SHARED ${AddInNative_SRC})

set_target_properties( ${PROJECT_NAME} PROPERTIES
       OUTPUT_NAME ${PROJECT_NAME}${MySuffix1}${MySuffix2}_${Addn_VERSION}
       POSITION_INDEPENDENT_CODE ON 
       CXX_STANDARD 11
       CXX_STANDARD_REQUIRED ON
   )
IF (UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-undefined -Xlinker --version-script -Xlinker ${CMAKE_CURRENT_SOURCE_DIR}/version.script" )
ENDIF()

if(UNIX AND NOT APPLE)
    SET(STRIP_CMD)
    if (STRIP)
        SET(STRIP_CMD ${STRIP} $<TARGET_FILE:${PROJECT_NAME}> -f ${PDB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>.debug)
    endif()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
            COMMAND ${STRIP_CMD}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${TMP_DIR}/
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PKG_OUTPUT_DIR}/
            COMMENT "Copy ${PROJECT_NAME}"
            MAIN_DEPENDENCY ${PROJECT_NAME}
            )
elseif(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PKG_OUTPUT_DIR}/
            COMMENT "Copy ${PROJECT_NAME}"
            MAIN_DEPENDENCY ${PROJECT_NAME}
            )
elseif(WIN32)
    if (SIGN_BAT)
        SET(SIGN_CMD1 ${SIGN_BAT} $<TARGET_FILE:${PROJECT_NAME}>)
        SET(SIGN_CMD2 )
    elseif(SIGN_TOOL AND EXISTS ${SIGN_CERT})
        SET(SIGN_CMD1 ${SIGN_SHA1} $<TARGET_FILE:${PROJECT_NAME}>)
        SET(SIGN_CMD2 ${SIGN_SHA256} $<TARGET_FILE:${PROJECT_NAME}>)
    else()
        SET(SIGN_CMD1)
        SET(SIGN_CMD2)
    endif()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
            COMMAND ${SIGN_CMD1}
            COMMAND ${SIGN_CMD2}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PKG_OUTPUT_DIR}/
            COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:${PROJECT_NAME}> ${LIB_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>
            COMMENT "Copy ${PROJECT_NAME}"
            MAIN_DEPENDENCY ${PROJECT_NAME}
            )
endif()
