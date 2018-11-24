aux_source_directory(./decoder DIR_UAVS2D_SRC)
aux_source_directory(./decore/x86 DIR_X86_SRC)
aux_source_directory(./decore/x86_256 DIR_X86_256_SRC)
aux_source_directory(./decore DIR_X86_CORE)
aux_source_directory(./decore/neon DIR_ARM_SRC)


list(APPEND DIR_UAVS2D_SRC ${DIR_X86_SRC})
list(APPEND DIR_UAVS2D_SRC ${DIR_X86_256_SRC})
list(APPEND DIR_UAVS2D_SRC ${DIR_X86_CORE})

include_directories("decore")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -mavx2 -march=native")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native")


add_library(uavs2d ${DIR_UAVS2D_SRC} )
set_property(TARGET uavs2d PROPERTY C_STANDARD 99)

install(TARGETS uavs2d LIBRARY DESTINATION lib ARCHIVE DESTINATION alib)
install(FILES decoder/uavs2d.h DESTINATION include)
