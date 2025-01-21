#if defined(__x86_64__)
    #include "ontop_x86_64_sysv_macho_gas.S"
#elif defined(__arm64__)
    #include "ontop_arm64_aapcs_macho_gas.S"
#else
    #error "Invalid Apple arch"
#endif
