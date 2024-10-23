# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\MediaPlayer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\MediaPlayer_autogen.dir\\ParseCache.txt"
  "MediaPlayer_autogen"
  )
endif()
