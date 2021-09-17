add_library(digiham_util OBJECT dumphex.c hamming_distance.c decoder.cpp meta.cpp phase.cpp cli.cpp charset.cpp coordinate.cpp version.cpp)
target_compile_options(digiham_util PRIVATE "-fPIC")
