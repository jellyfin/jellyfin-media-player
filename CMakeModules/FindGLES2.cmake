# Copyright (c) 2012, Guillermo A. Amaral B. (gamaral) <g@maral.me>.
# All rights reserved.
#
# This file is part of Marshmallow Game Engine.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are
# those of the authors and should not be interpreted as representing official
# policies, either expressed or implied, of the project as a whole.
#
###############################################################################
# Find GLES2
###############################################################################
#
#  GLES2_FOUND
#  GLES2_INCLUDE_DIR
#  GLES2_LIBRARY
#
###############################################################################

find_path(GLES2_INCLUDE_DIR GLES2/gl2.h
	HINTS $ENV{GLES2DIR}
	PATH_SUFFIXES include
	PATHS ~/Library/Frameworks
	      /Library/Frameworks
	      /usr/local
	      /usr
	      /usr/X11R6
	      /opt/local
	      /opt/vc
	      /opt
)

find_library(GLES2_LIBRARY
	GLESv2
	HINTS $ENV{GLES2DIR}
	PATH_SUFFIXES lib64 lib
	PATHS ~/Library/Frameworks
	      /Library/Frameworks
	      /usr/local
	      /usr
	      /usr/X11R6
	      /opt/local
	      /opt/vc
	      /opt
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLES2 DEFAULT_MSG GLES2_LIBRARY)

mark_as_advanced(GLES2_LIBRARY GLES2_INCLUDE_DIR)

set(OPENGL_FOUND ${GLES2_FOUND})
