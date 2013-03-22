#                    F I N D G L . C M A K E
# BRL-CAD
#
# Copyright (c) 2001-2013 United States Government as represented by
# the U.S. Army Research Laboratory.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following
# disclaimer in the documentation and/or other materials provided
# with the distribution.
#
# 3. The name of the author may not be used to endorse or promote
# products derived from this software without specific prior written
# permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
###
# - Try to find OpenGL
# Once done this will define
#
#  OPENGL_FOUND            - system has OpenGL
#  OPENGL_XMESA_FOUND      - system has XMESA
#  OPENGL_GLU_FOUND        - system has GLU
#  OPENGL_INCLUDE_DIR_GL   - the GL include directory
#  OPENGL_INCLUDE_DIR_GLX  - the GLX include directory
#  OPENGL_LIBRARIES        - Link these to use OpenGL and GLU
#
# If you want to use just GL you can use these values
#  OPENGL_gl_LIBRARY   - Path to OpenGL Library
#  OPENGL_glu_LIBRARY  - Path to GLU Library
#
# Define controlling option OPENGL_USE_AQUA if on Apple -
# if this is not true, look for the X11 OpenGL.  Defaults
# to true.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ------------------------------------------------------------------------------
#
# The above copyright and license notice applies to distributions of
# CMake in source and binary form.  Some source files contain additional
# notices of original copyright by their contributors; see each source
# for details.  Third-party software packages supplied with CMake under
# compatible licenses provide their own copyright notices documented in
# corresponding subdirectories.
#
# ------------------------------------------------------------------------------
#
# CMake was initially developed by Kitware with the following sponsorship:
#
#  * National Library of Medicine at the National Institutes of Health
#    as part of the Insight Segmentation and Registration Toolkit (ITK).
#
#  * US National Labs (Los Alamos, Livermore, Sandia) ASC Parallel
#    Visualization Initiative.
#
#  * National Alliance for Medical Image Computing (NAMIC) is funded by the
#    National Institutes of Health through the NIH Roadmap for Medical Research,
#    Grant U54 EB005149.
#
#  * Kitware, Inc.
#
#=============================================================================

if(WIN32)
      set(OPENGL_gl_LIBRARY opengl32 CACHE STRING "OpenGL library for win32")
      set(OPENGL_glu_LIBRARY glu32 CACHE STRING "GLU library for win32")
else(WIN32)

  # The first line below is to make sure that the proper headers
  # are used on a Linux machine with the NVidia drivers installed.
  # They replace Mesa with NVidia's own library but normally do not
  # install headers and that causes the linking to
  # fail since the compiler finds the Mesa headers but NVidia's library.
  # Make sure the NVIDIA directory comes BEFORE the others.
  #  - Atanas Georgiev <atanas@cs.columbia.edu>


  set(OPENGL_INC_SEARCH_PATH
    /usr/share/doc/NVIDIA_GLX-1.0/include
    /usr/X11/include
    /usr/X11R7/include
    /usr/X11R6/include
    /usr/include/X11
    /usr/local/include/X11
    /usr/local/include
    /usr/include
    /usr/openwin/share/include
    /usr/openwin/include
    /usr/pkg/xorg/include
    /opt/graphics/OpenGL/include
    )
  # Handle HP-UX cases where we only want to find OpenGL in either hpux64
  # or hpux32 depending on if we're doing a 64 bit build.
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(HPUX_IA_OPENGL_LIB_PATH /opt/graphics/OpenGL/lib/hpux32/)
  else(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(HPUX_IA_OPENGL_LIB_PATH
      /opt/graphics/OpenGL/lib/hpux64/
      /opt/graphics/OpenGL/lib/pa20_64)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

  get_property(SEARCH_64BIT GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
  if(SEARCH_64BIT)
    set(64BIT_DIRS "/usr/lib64/X11;/usr/lib64")
  else(SEARCH_64BIT)
    set(32BIT_DIRS "/usr/lib/X11;/usr/lib")
  endif(SEARCH_64BIT)

  set(OPENGL_LIB_SEARCH_PATH
    ${64BIT_DIRS}
    ${32BIT_DIRS}
    /usr/X11/lib
    /usr/X11R7/lib
    /usr/X11R6/lib
    /usr/shlib
    /usr/openwin/lib
    /opt/graphics/OpenGL/lib
    /usr/pkg/xorg/lib
    ${HPUX_IA_OPENGL_LIB_PATH}
    )

  if(APPLE)
    OPTION(OPENGL_USE_AQUA "Require native OSX Framework version of OpenGL." ON)
  endif(APPLE)

  if(OPENGL_USE_AQUA)
    find_library(OPENGL_gl_LIBRARY OpenGL DOC "OpenGL lib for OSX")
    find_library(OPENGL_glu_LIBRARY AGL DOC "AGL lib for OSX")
    find_path(OPENGL_INCLUDE_DIR_GL OpenGL/gl.h DOC "Include for OpenGL on OSX")
  else(OPENGL_USE_AQUA)
    # If we're on Apple and not using Aqua, we don't want frameworks
    set(CMAKE_FIND_FRAMEWORK "NEVER")

    find_path(OPENGL_INCLUDE_DIR_GL GL/gl.h        ${OPENGL_INC_SEARCH_PATH})
    find_path(OPENGL_INCLUDE_DIR_GLX GL/glx.h      ${OPENGL_INC_SEARCH_PATH})
    find_path(OPENGL_xmesa_INCLUDE_DIR GL/xmesa.h  ${OPENGL_INC_SEARCH_PATH})
    find_library(OPENGL_gl_LIBRARY NAMES GL MesaGL PATHS ${OPENGL_LIB_SEARCH_PATH})

    # On Unix OpenGL most certainly always requires X11.
    # Feel free to tighten up these conditions if you don't
    # think this is always true.
    # It's not true on OSX.

    if(OPENGL_gl_LIBRARY)
      if(NOT X11_FOUND)
	include(FindX11)
      endif(NOT X11_FOUND)
      if(X11_FOUND)
	set(OPENGL_LIBRARIES ${X11_LIBRARIES})
      endif(X11_FOUND)
    endif(OPENGL_gl_LIBRARY)

    find_library(OPENGL_glu_LIBRARY NAMES GLU MesaGLU PATHS ${OPENGL_LIB_SEARCH_PATH})
  endif(OPENGL_USE_AQUA)

endif(WIN32)

set( OPENGL_FOUND "NO" )

if(X11_FOUND)
  if(OPENGL_INCLUDE_DIR_GLX AND OPENGL_INCLUDE_DIR_GL AND OPENGL_gl_LIBRARY)
    set(OPENGL_FOUND "YES" )
  endif(OPENGL_INCLUDE_DIR_GLX AND OPENGL_INCLUDE_DIR_GL AND OPENGL_gl_LIBRARY)
else(X11_FOUND)
  if(MSVC OR MINGW)
    if(OPENGL_gl_LIBRARY)
      set(OPENGL_FOUND "YES" )
    endif(OPENGL_gl_LIBRARY)
  else(MSVC OR MINGW)
    if(OPENGL_INCLUDE_DIR_GL AND OPENGL_gl_LIBRARY)
      set(OPENGL_FOUND "YES" )
    endif(OPENGL_INCLUDE_DIR_GL AND OPENGL_gl_LIBRARY)
  endif(MSVC OR MINGW)
endif(X11_FOUND)


if(OPENGL_FOUND)
  if(OPENGL_xmesa_INCLUDE_DIR)
    set( OPENGL_XMESA_FOUND "YES" )
  else(OPENGL_xmesa_INCLUDE_DIR)
    set( OPENGL_XMESA_FOUND "NO" )
  endif(OPENGL_xmesa_INCLUDE_DIR)

  set(OPENGL_LIBRARIES  ${OPENGL_gl_LIBRARY} ${OPENGL_LIBRARIES})
  if(OPENGL_glu_LIBRARY)
    set(OPENGL_GLU_FOUND "YES" )
    set(OPENGL_LIBRARIES ${OPENGL_glu_LIBRARY} ${OPENGL_LIBRARIES} )
  else(OPENGL_glu_LIBRARY)
    set( OPENGL_GLU_FOUND "NO" )
  endif(OPENGL_glu_LIBRARY)

  # This deprecated setting is for backward compatibility with CMake1.4
  set(OPENGL_LIBRARY ${OPENGL_LIBRARIES})
else(OPENGL_FOUND)
  # We don't have enough, so no partials - clean up
  set(OPENGL_INCLUDE_DIR_GL "" CACHE STRING "OpenGL not found" FORCE)
  set(OPENGL_INCLUDE_DIR_GLX "" CACHE STRING "OpenGL not found" FORCE)
  set(OPENGL_xmesa_INCLUDE_DIR "" CACHE STRING "OpenGL not found" FORCE)
  set(OPENGL_glu_LIBRARY "" CACHE STRING "OpenGL not found" FORCE)
  set(OPENGL_gl_LIBRARY "" CACHE STRING "OpenGL not found" FORCE)
endif(OPENGL_FOUND)

mark_as_advanced(
  OPENGL_INCLUDE_DIR_GL
  OPENGL_INCLUDE_DIR_GLX
  OPENGL_xmesa_INCLUDE_DIR
  OPENGL_glu_LIBRARY
  OPENGL_gl_LIBRARY
  )

# Local Variables:
# tab-width: 8
# mode: cmake
# indent-tabs-mode: t
# End:
# ex: shiftwidth=2 tabstop=8
