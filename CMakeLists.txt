project(gl)

set(HEADER_FILES
    GlBuffers.h
    GlDebug.h
    GlFrameBuffer.h
    GlGeometry.h
    GlLighting.h
    GlMethods.h
    GlShaders.h
    GlVertexArrays.h
)

add_library(gl STATIC ${HEADER_FILES})
set_target_properties(gl PROPERTIES LINKER_LANGUAGE CXX)