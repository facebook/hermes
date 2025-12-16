add_node_api_module(test_cannot_run_js
  SOURCES
    test_cannot_run_js.c
  DEFINES
    "NAPI_VERSION=10"
)

add_node_api_module(test_pending_exception
  SOURCES
    test_cannot_run_js.c
  DEFINES
    "NAPI_VERSION=9"
)
