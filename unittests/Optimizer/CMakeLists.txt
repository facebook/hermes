set(OptimizerSources
  Scalar/InstructionEscapeAnalysisTest.cpp
  Scalar/InstructionNumberingTest.cpp
  )

add_hermes_unittest(HermesOptimizerTests
  ${OptimizerSources}
  )

target_link_libraries(HermesOptimizerTests
 hermesOptimizer
 hermesFrontend
 )
