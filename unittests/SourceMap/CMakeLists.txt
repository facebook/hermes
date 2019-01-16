set(SourceMapSources
  SourceMapTest.cpp
  )

add_hermes_unittest(HermesSourceMapTests
  ${SourceMapSources}
  )

target_link_libraries(HermesSourceMapTests
 hermesSourceMap
 )
