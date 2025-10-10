add_node_api_module(test_reference
  SOURCES
    test_reference.c
  DEFINES
    "NAPI_VERSION=9"
)

add_node_api_module(test_finalizer
  SOURCES
    test_finalizer.c
)
