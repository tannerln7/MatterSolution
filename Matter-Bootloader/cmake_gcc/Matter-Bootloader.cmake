####################################################################
# Automatically-generated file. Do not edit!                       #
####################################################################

set(SDK_PATH "/home/tannerln7/.silabs/slt/installs/conan/p/simpleb526998f4a4d/p")
set(COPIED_SDK_PATH "simplicity_sdk_2025.6.2")
set(PKG_PATH "/home/tannerln7/.silabs/slt/installs")

add_library(slc OBJECT
    "${SDK_PATH}/../../matte7710ee5c0d1f2/p/third_party/matter_sdk/examples/platform/silabs/syscalls_stubs.cpp"
    "${SDK_PATH}/platform/bootloader/core/btl_bootload.c"
    "${SDK_PATH}/platform/bootloader/core/btl_core.c"
    "${SDK_PATH}/platform/bootloader/core/btl_main.c"
    "${SDK_PATH}/platform/bootloader/core/btl_parse.c"
    "${SDK_PATH}/platform/bootloader/core/btl_reset.c"
    "${SDK_PATH}/platform/bootloader/core/flash/btl_internal_flash.c"
    "${SDK_PATH}/platform/bootloader/debug/btl_debug.c"
    "${SDK_PATH}/platform/bootloader/debug/btl_debug_swo.c"
    "${SDK_PATH}/platform/bootloader/driver/btl_driver_util.c"
    "${SDK_PATH}/platform/bootloader/parser/compression/btl_decompress_lz4.c"
    "${SDK_PATH}/platform/bootloader/parser/compression/btl_decompress_lzma.c"
    "${SDK_PATH}/platform/bootloader/parser/compression/lzma/LzmaDec.c"
    "${SDK_PATH}/platform/bootloader/parser/gbl/btl_gbl_custom_tags.c"
    "${SDK_PATH}/platform/bootloader/parser/gbl/btl_gbl_format.c"
    "${SDK_PATH}/platform/bootloader/parser/gbl/btl_gbl_parser.c"
    "${SDK_PATH}/platform/bootloader/security/btl_crc16.c"
    "${SDK_PATH}/platform/bootloader/security/btl_crc32.c"
    "${SDK_PATH}/platform/bootloader/security/btl_security_aes.c"
    "${SDK_PATH}/platform/bootloader/security/btl_security_ecdsa.c"
    "${SDK_PATH}/platform/bootloader/security/btl_security_sha256.c"
    "${SDK_PATH}/platform/bootloader/security/btl_security_tokens.c"
    "${SDK_PATH}/platform/bootloader/security/ecc/ecc.c"
    "${SDK_PATH}/platform/bootloader/security/sha/btl_sha256.c"
    "${SDK_PATH}/platform/bootloader/security/sha/crypto_sha.c"
    "${SDK_PATH}/platform/bootloader/security/sha/cryptoacc_sha.c"
    "${SDK_PATH}/platform/bootloader/security/sha/se_sha.c"
    "${SDK_PATH}/platform/bootloader/storage/bootloadinfo/btl_storage_bootloadinfo.c"
    "${SDK_PATH}/platform/bootloader/storage/btl_storage.c"
    "${SDK_PATH}/platform/bootloader/storage/btl_storage_library.c"
    "${SDK_PATH}/platform/bootloader/storage/internal_flash/btl_storage_internal_flash.c"
    "${SDK_PATH}/platform/common/src/sl_assert.c"
    "${SDK_PATH}/platform/common/src/sl_core_cortexm.c"
    "${SDK_PATH}/platform/common/src/sl_syscalls.c"
    "${SDK_PATH}/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c"
    "${SDK_PATH}/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c"
    "${SDK_PATH}/platform/emlib/src/em_acmp.c"
    "${SDK_PATH}/platform/emlib/src/em_burtc.c"
    "${SDK_PATH}/platform/emlib/src/em_cmu.c"
    "${SDK_PATH}/platform/emlib/src/em_dbg.c"
    "${SDK_PATH}/platform/emlib/src/em_emu.c"
    "${SDK_PATH}/platform/emlib/src/em_eusart.c"
    "${SDK_PATH}/platform/emlib/src/em_gpcrc.c"
    "${SDK_PATH}/platform/emlib/src/em_gpio.c"
    "${SDK_PATH}/platform/emlib/src/em_i2c.c"
    "${SDK_PATH}/platform/emlib/src/em_iadc.c"
    "${SDK_PATH}/platform/emlib/src/em_ldma.c"
    "${SDK_PATH}/platform/emlib/src/em_letimer.c"
    "${SDK_PATH}/platform/emlib/src/em_msc.c"
    "${SDK_PATH}/platform/emlib/src/em_opamp.c"
    "${SDK_PATH}/platform/emlib/src/em_pcnt.c"
    "${SDK_PATH}/platform/emlib/src/em_prs.c"
    "${SDK_PATH}/platform/emlib/src/em_rmu.c"
    "${SDK_PATH}/platform/emlib/src/em_system.c"
    "${SDK_PATH}/platform/emlib/src/em_timer.c"
    "${SDK_PATH}/platform/emlib/src/em_usart.c"
    "${SDK_PATH}/platform/emlib/src/em_vdac.c"
    "${SDK_PATH}/platform/emlib/src/em_wdog.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_attestation.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_cipher.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_entropy.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_hash.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_key_derivation.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_key_handling.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_signature.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sl_se_manager_util.c"
    "${SDK_PATH}/platform/security/sl_component/se_manager/src/sli_se_manager_mailbox.c"
    "${SDK_PATH}/platform/security/sl_component/sl_mbedtls_support/src/se_aes.c"
    "${SDK_PATH}/platform/security/sl_component/sl_mbedtls_support/src/sl_mbedtls.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_common.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_psa_driver_init.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_aead.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_builtin_keys.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_cipher.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_derivation.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_key_management.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_mac.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_driver_signature.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_aead.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_cipher.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_driver_mac.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_opaque_key_derivation.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_aead.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_cipher.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_hash.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_driver_mac.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_transparent_key_derivation.c"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/src/sli_se_version_dependencies.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sl_memory_manager.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sl_memory_manager_dynamic_reservation.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sl_memory_manager_pool.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sl_memory_manager_pool_common.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sl_memory_manager_region.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sl_memory_manager_retarget.c"
    "${SDK_PATH}/platform/service/memory_manager/src/sli_memory_manager_common.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/aes.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/constant_time.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/platform.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/platform_util.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/psa_crypto_client.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/psa_util.c"
    "${SDK_PATH}/util/third_party/mbedtls/library/threading.c"
    "${SDK_PATH}/util/third_party/trusted-firmware-m/lib/fih/src/fih.c"
    "${SDK_PATH}/util/third_party/trusted-firmware-m/platform/ext/target/siliconlabs/hse/sli_se.c"
)

target_include_directories(slc PUBLIC
   "../config"
   "../autogen"
    "${SDK_PATH}/platform/Device/SiliconLabs/EFR32MG24/Include"
    "${SDK_PATH}/platform/common/inc"
    "${SDK_PATH}/platform/bootloader"
    "${SDK_PATH}/platform/bootloader/parser/compression"
    "${SDK_PATH}/platform/bootloader/debug"
    "${SDK_PATH}/platform/bootloader/parser"
    "${SDK_PATH}/platform/bootloader/api"
    "${SDK_PATH}/platform/bootloader/core/flash"
    "${SDK_PATH}/platform/bootloader/security"
    "${SDK_PATH}/platform/CMSIS/Core/Include"
    "${SDK_PATH}/platform/emlib/inc"
    "${SDK_PATH}/platform/security/sl_component/sl_mbedtls_support/config"
    "${SDK_PATH}/platform/security/sl_component/sl_mbedtls_support/config/preset"
    "${SDK_PATH}/platform/security/sl_component/sl_mbedtls_support/inc"
    "${SDK_PATH}/util/third_party/mbedtls/include"
    "${SDK_PATH}/util/third_party/mbedtls/library"
    "${SDK_PATH}/platform/service/memory_manager/inc"
    "${SDK_PATH}/platform/service/memory_manager/src"
    "${SDK_PATH}/platform/security/sl_component/sl_psa_driver/inc"
    "${SDK_PATH}/platform/security/sl_component/se_manager/inc"
    "${SDK_PATH}/util/third_party/trusted-firmware-m/lib/fih/inc"
    "${SDK_PATH}/util/third_party/trusted-firmware-m/platform/include"
    "${SDK_PATH}/platform/security/sl_component/sli_psec_osal/inc"
)

target_compile_definitions(slc PUBLIC
    "EFR32MG24B220F1536IM48=1"
    "SL_CODE_COMPONENT_SYSTEM=system"
    "SE_MANAGER_CONFIG_FILE=\"btl_aes_ctr_stream_block_cfg.h\""
    "BTL_PARSER_SUPPORT_CUSTOM_TAGS=1"
    "BTL_PARSER_SUPPORT_LZMA=1"
    "_LZMA_SIZE_OPT=1"
    "BOOTLOADER_ENABLE=1"
    "BOOTLOADER_SECOND_STAGE=1"
    "SL_RAMFUNC_DISABLE=1"
    "__START=main"
    "__STARTUP_CLEAR_BSS=1"
    "SYSTEM_NO_STATIC_MEMORY=1"
    "BTL_PARSER_SUPPORT_LZ4=1"
    "BOOTLOADER_SUPPORT_INTERNAL_STORAGE=1"
    "BOOTLOADER_SUPPORT_STORAGE=1"
    "SL_COMPONENT_CATALOG_PRESENT=1"
    "MBEDTLS_CONFIG_FILE=<sl_mbedtls_trustzone_config.h>"
    "MBEDTLS_PSA_CRYPTO_CLIENT=1"
    "MBEDTLS_PSA_CRYPTO_CONFIG_FILE=<psa_crypto_config.h>"
    "SL_CODE_COMPONENT_SE_MANAGER=se_manager"
    "SL_CODE_COMPONENT_CORE=core"
    "SL_CODE_COMPONENT_PSEC_OSAL=psec_osal"
    "SL_TRUSTZONE_SECURE=1"
)

target_link_libraries(slc PUBLIC
    "-Wl,--start-group"
    "gcc"
    "c"
    "m"
    "nosys"
    "-Wl,--end-group"
)
target_compile_options(slc PUBLIC
    $<$<COMPILE_LANGUAGE:C>:-mcpu=cortex-m33>
    $<$<COMPILE_LANGUAGE:C>:-mthumb>
    $<$<COMPILE_LANGUAGE:C>:-mfpu=fpv5-sp-d16>
    $<$<COMPILE_LANGUAGE:C>:-mfloat-abi=hard>
    $<$<COMPILE_LANGUAGE:C>:-mcmse>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-Os>
    $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:C>:-fomit-frame-pointer>
    $<$<COMPILE_LANGUAGE:C>:-g>
    $<$<COMPILE_LANGUAGE:C>:-Wno-ignored-qualifiers>
    $<$<COMPILE_LANGUAGE:C>:-Wno-sign-compare>
    $<$<COMPILE_LANGUAGE:C>:-fno-lto>
    $<$<COMPILE_LANGUAGE:C>:--specs=nano.specs>
    $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-m33>
    $<$<COMPILE_LANGUAGE:CXX>:-mthumb>
    $<$<COMPILE_LANGUAGE:CXX>:-mfpu=fpv5-sp-d16>
    $<$<COMPILE_LANGUAGE:CXX>:-mfloat-abi=hard>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-mcmse>
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Os>
    $<$<COMPILE_LANGUAGE:CXX>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-fomit-frame-pointer>
    $<$<COMPILE_LANGUAGE:CXX>:-g>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-ignored-qualifiers>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-sign-compare>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-lto>
    $<$<COMPILE_LANGUAGE:CXX>:--specs=nano.specs>
    $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-m33>
    $<$<COMPILE_LANGUAGE:ASM>:-mthumb>
    $<$<COMPILE_LANGUAGE:ASM>:-mfpu=fpv5-sp-d16>
    $<$<COMPILE_LANGUAGE:ASM>:-mfloat-abi=hard>
    "$<$<COMPILE_LANGUAGE:ASM>:SHELL:-x assembler-with-cpp>"
)

set(post_build_command ${POST_BUILD_EXE} postbuild "./Matter-Bootloader.slpb" --parameter build_dir:"$<TARGET_FILE_DIR:Matter-Bootloader>")
set_property(TARGET slc PROPERTY C_STANDARD 17)
set_property(TARGET slc PROPERTY CXX_STANDARD 17)
set_property(TARGET slc PROPERTY CXX_EXTENSIONS OFF)

target_link_options(slc INTERFACE
    -mcpu=cortex-m33
    -mthumb
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
    -T${CMAKE_CURRENT_LIST_DIR}/../autogen/linkerfile.ld
    --specs=nano.specs
    "SHELL:-Xlinker -Map=$<TARGET_FILE_DIR:Matter-Bootloader>/Matter-Bootloader.map"
    -Wl,--no-warn-rwx-segment
    "SHELL:-Wl,--wrap=_free_r -Wl,--wrap=_malloc_r -Wl,--wrap=_calloc_r -Wl,--wrap=_realloc_r"
    -fno-lto
    -Wl,--gc-sections
)

# BEGIN_SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA