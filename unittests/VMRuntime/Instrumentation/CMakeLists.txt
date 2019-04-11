set(ISources
  ApproxIntegralTest.cpp
  ProcessStatsTest.cpp
)

add_hermes_unittest(HermesInstrumentationTests
  ${ISources}
)

target_link_libraries(HermesInstrumentationTests
  hermesInstrumentation
  )
