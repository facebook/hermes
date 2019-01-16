set(DtoaSources
  DtoaTest.cpp
  )

add_hermes_unittest(DtoaTests
  ${DtoaSources}
  )

target_link_libraries(DtoaTests
 dtoa
 )
