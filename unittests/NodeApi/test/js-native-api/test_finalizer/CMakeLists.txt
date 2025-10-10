add_node_api_module(test_finalizer
  SOURCES
    test_finalizer.c
  DEFINES
    NAPI_EXPERIMENTAL
    NODE_API_EXPERIMENTAL_NO_WARNING
)
