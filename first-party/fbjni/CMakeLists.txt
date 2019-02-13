project(hermesfbjni CXX)

set(fbjni_SOURCES
  cxx/fbjni/ByteBuffer.cpp
  cxx/fbjni/OnLoad.cpp
  cxx/fbjni/ReadableByteChannel.cpp
  cxx/fbjni/fbjni.cpp
  cxx/fbjni/detail/Environment.cpp
  cxx/fbjni/detail/Exceptions.cpp
  cxx/fbjni/detail/Hybrid.cpp
  cxx/fbjni/detail/References.cpp
  cxx/fbjni/detail/utf8.cpp
  cxx/lyra/cxa_throw.cpp
  cxx/lyra/lyra.cpp
  cxx/lyra/lyra_breakpad.cpp
  cxx/lyra/lyra_exceptions.cpp
)

add_library(hermesfbjni STATIC
  ${fbjni_SOURCES}
)

target_compile_options(hermesfbjni PRIVATE
  -fno-omit-frame-pointer
  -fexceptions
  -O3
  -Wall
  -std=c++11
  -frtti
  -ffunction-sections
  -DDISABLE_CPUCAP
  -DDISABLE_XPLAT
  -DNDEBUG
  -Wno-error
)

target_include_directories(hermesfbjni PUBLIC
  cxx
)

target_link_libraries(hermesfbjni
  android
  log
)
