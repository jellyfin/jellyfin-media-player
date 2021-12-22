# Get the current date.
# include(WebClientVariables)
string(TIMESTAMP CURRENT_DATE "%Y-%m-%d")

# Get git revision version
#include(GetGitRevisionDescription)
#get_git_head_revision(REFSPEC FULL_GIT_REVISION)
#if(FULL_GIT_REVISION STREQUAL "GITDIR-NOTFOUND")
#  set(GIT_REVISION "git")
#else(FULL_GIT_REVISION STREQUAL "GITDIR-NOTFOUND")
#  string(SUBSTRING ${FULL_GIT_REVISION} 0 8 GIT_REVISION)
#endif(FULL_GIT_REVISION STREQUAL "GITDIR-NOTFOUND")

# Get the build number if available
if(DEFINED ENV{BUILD_NUMBER})
  set(VERSION_BUILD "$ENV{BUILD_NUMBER}")
  set(VERSION_BUILD_NR "$ENV{BUILD_NUMBER}")
else()
  set(VERSION_BUILD "dev")
  set(VERSION_BUILD_NR "0")
endif()

set(VERSION_MAJOR 1)
set(VERSION_MINOR 7)
set(VERSION_NANO 0)

option(UPGRADE_DEBUG "" OFF)

set(VERSION_STRING "1.7.0-pre2")
set(VERSION_STRING_SHORT "1.7.0")
set(CANONICAL_VERSION_STRING "1.7.0-pre2")

configure_file(src/core/Version.cpp.in src/core/Version.cpp)
