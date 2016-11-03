aux_source_directory(./decoder DIR_UAVS2D_SRC)
aux_source_directory(./decore/x86 DIR_X86_SRC)
aux_source_directory(./decore/x86_256 DIR_X86_256_SRC)
aux_source_directory(./decore DIR_X86_CORE)

list(APPEND DIR_UAVS2D_SRC ${DIR_X86_SRC})
list(APPEND DIR_UAVS2D_SRC ${DIR_X86_256_SRC})
list(APPEND DIR_UAVS2D_SRC ${DIR_X86_CORE})

include_directories("decore")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -mavx2 -march=native")

add_library(uavs2d ${DIR_UAVS2D_SRC})

install(TARGETS uavs2d LIBRARY DESTINATION /usr/lib ARCHIVE DESTINATION /usr/lib)
install(FILES decoder/uavs2d.h DESTINATION /usr/include)
