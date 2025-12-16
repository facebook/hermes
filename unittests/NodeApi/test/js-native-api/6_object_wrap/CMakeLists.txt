add_node_api_module(myobject
  SOURCES
    myobject.cc
    myobject.h
)

add_node_api_module(myobject_basic_finalizer
  SOURCES
    myobject.cc
    myobject.h
  DEFINES
    NAPI_EXPERIMENTAL
)

add_node_api_module(nested_wrap
  SOURCES
    nested_wrap.cc
    nested_wrap.h
  DEFINES
    "NAPI_VERSION=10"
)
