set(source_files
   PlatformUnicodeJava.cpp
   PlatformUnicodeCF.cpp
   PlatformUnicodeICU.cpp
   CharacterProperties.cpp
)

add_llvm_library(hermesPlatformUnicode STATIC ${source_files}
    LINK_LIBS
    ${CORE_FOUNDATION}
)

hermes_link_icu(hermesPlatformUnicode)
