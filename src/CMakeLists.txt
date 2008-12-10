ADD_EXECUTABLE (ptfs allocate.c earley.c hashtab.c objstack.c ptfs.c vlobject.c)

INCLUDE_DIRECTORIES(/usr/include/fuse)
TARGET_LINK_LIBRARIES(ptfs fuse)
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
ADD_CUSTOM_TARGET(bison bison sgramm.y && mv sgramm.tab.c sgramm.c)
