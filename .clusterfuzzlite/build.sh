mkdir build
cd build
cmake ../
make

# Copy all fuzzer executables to $OUT/
$CXX $CFLAGS $LIB_FUZZING_ENGINE \
  $SRC/tipa/.clusterfuzzlite/fuzz_parse.cpp \
  -o $OUT/fuzz_parse \
  -I$SRC/tipa/src/ \
  $SRC/tipa/build/src/libtipa.a
