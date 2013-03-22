#############
## basic FindQxt.cmake
## This is an *EXTREMELY BASIC* cmake find/config file for
## those times you have a cmake project and wish to use
## libQxt.
##
## It should be noted that at the time of writing, that
## I (mschnee) have an extremely limited understanding of the
## way Find*.cmake files work, but I have attempted to
## emulate what FindQt4.cmake and a few others do.
##
##  To enable a specific component, set your QXT_USE_${modname}:
##  SET(QXT_USE_QXTCORE TRUE)
##  SET(QXT_USE_QXTGUI FALSE)
##  Currently available components:
##  QxtCore, QxtGui, QxtNetwork, QxtWeb, QxtSql
##  Auto-including directories are enabled with INCLUDE_DIRECTORIES(), but
##  can be accessed if necessary via ${QXT_INCLUDE_DIRS}
##
## To add the libraries to your build, TARGET_LINK_LIBRARIES(), such as...
##  TARGET_LINK_LIBRARIES(YourTargetNameHere ${QXT_LIBRARIES})
## ...or..
##  TARGET_LINK_LIBRARIES(YourTargetNameHere ${QT_LIBRARIES} ${QXT_LIBRARIES})
################### TODO:
##      The purpose of this cmake file is to find what components
##  exist, regardless of how libQxt was build or configured, thus
##  it should search/find all possible options.  As I am not aware
##  that any module requires anything special to be used, adding all
##  modules to ${QXT_MODULES} below should be sufficient.
##      Eventually, there should be version numbers, but
##  I am still too unfamiliar with cmake to determine how to do
##  version checks and comparisons.
##      At the moment, this cmake returns a failure if you
##  try to use a component that doesn't exist.  I don't know how to
##  set up warnings.
##      It would be nice having a FindQxt.cmake and a UseQxt.cmake
##  file like done for Qt - one to check for everything in advance

##############

###### setup
SET(QXT_MODULES QxtGui QxtWeb QxtZeroConf QxtNetwork QxtSql QxtBerkeley QxtCore)
SET(QXT_FOUND_MODULES)
FOREACH(mod ${QXT_MODULES})
    STRING(TOUPPER ${mod} U_MOD)
    SET(QXT_${U_MOD}_INCLUDE_DIR NOTFOUND)
    SET(QXT_${U_MOD}_LIB_DEBUG NOTFOUND)
    SET(QXT_${U_MOD}_LIB_RELEASE NOTFOUND)
    SET(QXT_FOUND_${U_MOD} FALSE)
ENDFOREACH(mod)
SET(QXT_QXTGUI_DEPENDSON QxtCore)
SET(QXT_QXTWEB_DEPENDSON QxtCore QxtNetwork)
SET(QXT_QXTZEROCONF_DEPENDSON QxtCore QxtNetwork)
SET(QXT_QXTNETWORK_DEPENDSON QxtCore)
SET(QXT_QXTQSQL_DEPENDSON QxtCore)
SET(QXT_QXTBERKELEY_DEPENDSON QxtCore)

FOREACH(mod ${QXT_MODULES})
    STRING(TOUPPER ${mod} U_MOD)
    IF(NOT ${U_MOD}_INCLUDE_DIR)
	    FIND_PATH(QXT_${U_MOD}_INCLUDE_DIR NAME ${mod}
		PATH_SUFFIXES ${mod} include/${mod}
			qxt/include/${mod} include/qxt/${mod}
			Qxt/include/${mod} include/Qxt/${mod}
		PATHS
		~/Library/Frameworks/
		/Library/Frameworks/
		/sw/
		/usr/local/
		/usr
		/opt/local/
		/opt/csw
		/opt
		"C:/Program Files/libqxt/include"
		"C:/Program Files (x86)/libqxt/include"
		NO_DEFAULT_PATH
	    )
	    FIND_LIBRARY(QXT_${U_MOD}_LIB_RELEASE NAME ${mod}
		PATH_SUFFIXES Qxt/lib64 Qxt/lib lib64 lib lib/${CMAKE_LIBRARY_ARCHITECTURE} 
		PATHS
		/sw
		/usr/local
		/usr
		/opt/local
		/opt/csw
		/opt
		"C:/Program Files/libqxt"
		"C:/Program Files (x86)/libqxt"
		NO_DEFAULT_PATH
	    )
	    FIND_LIBRARY(QXT_${U_MOD}_LIB_DEBUG NAME ${mod}d
		PATH_SUFFIXES Qxt/lib64 Qxt/lib lib64 lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
		PATHS
		/sw
		/usr/local
		/usr
		/opt/local
		/opt/csw
		/opt
		"C:/Program Files/libqxt/"
		"C:/Program Files (x86)/libqxt/"
		NO_DEFAULT_PATH
	    )
	    IF (QXT_${U_MOD}_LIB_RELEASE)
		SET(QXT_FOUND_MODULES "${QXT_FOUND_MODULES} ${mod}")
	    ENDIF (QXT_${U_MOD}_LIB_RELEASE)

	    IF (QXT_${U_MOD}_LIB_DEBUG)
		SET(QXT_FOUND_MODULES "${QXT_FOUND_MODULES} ${mod}")
	    ENDIF (QXT_${U_MOD}_LIB_DEBUG)
    ENDIF()
ENDFOREACH(mod)

FOREACH(mod ${QXT_MODULES})
    STRING(TOUPPER ${mod} U_MOD)
    IF(QXT_${U_MOD}_INCLUDE_DIR AND QXT_${U_MOD}_LIB_RELEASE)
        SET(QXT_FOUND_${U_MOD} TRUE)
    ENDIF(QXT_${U_MOD}_INCLUDE_DIR AND QXT_${U_MOD}_LIB_RELEASE)
ENDFOREACH(mod)


##### find and include
# To use a Qxt Library....
#   SET(QXT_FIND_COMPONENTS QxtCore, QxtGui)
# ...and this will do the rest
IF( QXT_FIND_COMPONENTS )
    FOREACH( component ${QXT_FIND_COMPONENTS} )
        STRING( TOUPPER ${component} _COMPONENT )
        SET(QXT_USE_${_COMPONENT}_COMPONENT TRUE)
    ENDFOREACH( component )
ENDIF( QXT_FIND_COMPONENTS )

SET(QXT_LIBRARIES "")
SET(QXT_INCLUDE_DIRS "")

# like FindQt4.cmake, in order of dependence
FOREACH( module ${QXT_MODULES} )
    STRING(TOUPPER ${module} U_MOD)
    IF(QXT_USE_${U_MOD} OR QXT_DEPENDS_${U_MOD})
        IF(QXT_FOUND_${U_MOD})
            STRING(REPLACE "QXT" "" qxt_module_def "${U_MOD}")
            ADD_DEFINITIONS(-DQXT_${qxt_module_def}_LIB)
            SET(QXT_INCLUDE_DIRS ${QXT_INCLUDE_DIRS} ${QXT_${U_MOD}_INCLUDE_DIR})
            INCLUDE_DIRECTORIES(${QXT_${U_MOD}_INCLUDE_DIR})
            SET(QXT_LIBRARIES ${QXT_LIBRARIES} ${QXT_${U_MOD}_LIB_RELEASE})
        ELSE(QXT_FOUND_${U_MOD})
            MESSAGE("Could not find Qxt Module ${module}")
            RETURN()
        ENDIF(QXT_FOUND_${U_MOD})
        FOREACH(dep QXT_${U_MOD}_DEPENDSON)
            SET(QXT_DEPENDS_${dep} TRUE)
        ENDFOREACH(dep)
    ENDIF(QXT_USE_${U_MOD} OR QXT_DEPENDS_${U_MOD})
ENDFOREACH(module)
MESSAGE(STATUS "Found Qxt Libraries:${QXT_FOUND_MODULES}")
