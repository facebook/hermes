set(ADTSources
  ScopedHashTable.cpp
  WordBitSet.cpp
  )

add_hermes_unittest(HermesADTTests
  ${ADTSources}
  )

