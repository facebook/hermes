# Some code is not compatible with unity builds, so turn them off for this target.
set(HERMES_ENABLE_UNITY_BUILD OFF)

add_hermes_library(LLVHDemangle
  ItaniumDemangle.cpp
  MicrosoftDemangle.cpp
  MicrosoftDemangleNodes.cpp
)
