cmake_minimum_required(VERSION 3.3)

project(uavs2d)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")
aux_source_directory(./decoder DIR_UAVS2D_SRC)
aux_source_directory(./decore/x86 DIR_X86_SRC)
aux_source_directory(./decore/x86_256 DIR_X86_256_SRC)
aux_source_directory(./decore DIR_X86_CORE)
aux_source_directory(./decore/neon DIR_ARM_SRC)

set(TARGET_ARCH_REGEX "^.*-march[= ]([^ ]+).*$")
string(REGEX MATCH "${TARGET_ARCH_REGEX}" TARGET_ARCH_MATCH "${CMAKE_C_FLAGS}" "${CMAKE_CXX_FLAGS}")
if (TARGET_ARCH_MATCH)
    string(REGEX REPLACE "${TARGET_ARCH_REGEX}" "1" TARGET_ARCH ${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS})
else ()
    set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif ()

message("Build For:${TARGET_ARCH}")

include_directories("decore")

if (TARGET_ARCH STREQUAL "x86_64")
    list(APPEND DIR_UAVS2D_SRC ${DIR_X86_SRC})
    list(APPEND DIR_UAVS2D_SRC ${DIR_X86_256_SRC})
    list(APPEND DIR_UAVS2D_SRC ${DIR_X86_CORE})
endif ()

if (TARGET_ARCH STREQUAL "aarch64")
    enable_language(ASM)
    set(CORE_SRC
            decoder/AEC.c
            decoder/block.c
            decoder/codingUnit.c
            decoder/DecAdaptiveLoopFilter.c
            decoder/header.c
            decoder/image.c
            decoder/loopfilter.c
            decoder/wquant.c
            decoder/uavs2d.c
            decore/biaridecod.c
            decore/bitstream.c
            decore/block_info.c
            decore/ComAdaptiveLoopFilter.c
            decore/common_tables.c
            decore/common.c
            decore/inter-prediction.c
            decore/intra-prediction.c
            decore/loop-filter.c
            decore/pixel.c
            decore/threadpool.c
            decore/transform.c
            )

    set(DIR_UAVS2D_SRC  ${CORE_SRC})
endif ()

if (TARGET_ARCH STREQUAL "armv7a")
    set(CODE_ARM_SRC
            decore/neon/android_sao_neon.s
            decore/neon/android_sao.c
            decore/neon/android.c
            decore/neon/android_pixel.s
            decore/neon/asm.S
            decore/neon/android_inter_pred.s
            decore/neon/android_idct.s
            decore/neon/android_alf.s
            decore/neon/android_deblock.s
            decore/neon/android_intra_pred.s
            )
    set(DIR_UAVS2D_SRC ${CODE_ARM_SRC})
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ")
else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -mavx2 -march=native")

endif ()

add_library(uavs2d ${DIR_UAVS2D_SRC})
if (TARGET_ARCH STREQUAL "aarch64")
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_sao_neon.s PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_pixel.s PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/asm.S PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_inter_pred.s PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_idct.s PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_alf.s PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_deblock.s PROPERTY LANGUAGE C)
#    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decore/neon/android_intra_pred.s PROPERTY LANGUAGE C)
endif()
set_property(TARGET uavs2d PROPERTY C_STANDARD 99)

install(TARGETS uavs2d LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(FILES decoder/uavs2d.h DESTINATION include)
