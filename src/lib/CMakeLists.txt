add_library(digiham_util OBJECT dumphex.c hamming_distance.c version.c decoder.cpp meta.cpp phase.cpp cli.cpp charset.cpp coordinate.cpp)
target_compile_options(digiham_util PRIVATE "-fPIC")
