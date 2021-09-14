
add_library(reedgefs INTERFACE)
add_library(reliance-edge::fs ALIAS reedgefs)

target_include_directories(reedgefs
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/core/include>
)

target_sources( reedgefs
    INTERFACE
    src/core/driver/blockio.c
    src/core/driver/buffer.c
    src/core/driver/buffercmn.c
    src/core/driver/core.c
    src/core/driver/dir.c
    src/core/driver/format.c
    src/core/driver/imap.c
    src/core/driver/imapextern.c
    src/core/driver/imapinline.c
    src/core/driver/inode.c
    src/core/driver/inodedata.c
    src/core/driver/volume.c
    src/fse/fse.c
    src/posix/path.c
    src/posix/posix.c
    src/util/bitmap.c
    src/util/crc.c
    src/util/endian.c
    src/util/memory.c
    src/util/namelen.c
    src/util/sign.c
    src/util/string.c
)
