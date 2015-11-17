# Get the current date.
include(GetDate)
include(WebClientVariables)
today(CURRENT_DATE)

# Get git revision version
include(GetGitRevisionDescription)
get_git_head_revision(REFSPEC FULL_GIT_REVISION)
if(FULL_GIT_REVISION STREQUAL "GITDIR-NOTFOUND")
  set(GIT_REVISION "git")
else(FULL_GIT_REVISION STREQUAL "GITDIR-NOTFOUND")
  string(SUBSTRING ${FULL_GIT_REVISION} 0 8 GIT_REVISION)
endif(FULL_GIT_REVISION STREQUAL "GITDIR-NOTFOUND")

# Get the build number if available
if(DEFINED ENV{BUILD_NUMBER})
  set(VERSION_BUILD "$ENV{BUILD_NUMBER}")
  set(VERSION_BUILD_NR "$ENV{BUILD_NUMBER}")
else()
  set(VERSION_BUILD "dev")
  set(VERSION_BUILD_NR "0")
endif()

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_NANO 3)

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_NANO}.${VERSION_BUILD}-${GIT_REVISION}")
set(CANONICAL_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_NANO}-${GIT_REVISION}")
configure_file(src/Version.cpp.in src/Version.cpp)