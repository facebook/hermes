#if defined(__x86_64__)
    #include "make_x86_64_sysv_macho_gas.S"
#elif defined(__arm64__)
    #include "make_arm64_aapcs_macho_gas.S"
#else
    #error "Invalid Apple arch"
#endif
