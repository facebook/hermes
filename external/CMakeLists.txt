add_subdirectory(dtoa)
if(HERMES_ENABLE_NAPI)
  add_subdirectory(node-api-cts/implementors/hermes)
  add_subdirectory(node-api-tests/hermes)

  # Combined target to run all NAPI tests (CTS + Node.js).
  add_custom_target(check-napi
    DEPENDS check-napi-cts check-napi-node-tests
    COMMENT "Running all Node-API tests under Hermes"
  )
endif()

add_subdirectory(zip)
if(HERMESVM_ALLOW_JIT)
  add_subdirectory(asmjit)
endif()
add_subdirectory(llvh)
if (HERMES_USE_BOOST_CONTEXT)
  add_subdirectory(boost)
endif()
