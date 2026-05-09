# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\tangjiaqi_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tangjiaqi_autogen.dir\\ParseCache.txt"
  "tangjiaqi_autogen"
  )
endif()
