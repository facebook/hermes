set(OptimizerSources
  Scalar/InstructionEscapeAnalysisTest.cpp
  )

add_hermes_unittest(HermesOptimizerTests
  ${OptimizerSources}
  )

target_link_libraries(HermesOptimizerTests
 hermesOptimizer
 hermesFrontend
 )
