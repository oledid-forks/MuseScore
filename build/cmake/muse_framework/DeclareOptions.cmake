
macro(declare_muse_module_opt name def)
    option(MUSE_MODULE_${name} "Build ${name} module" ${def})
    option(MUSE_MODULE_${name}_TESTS "Build ${name} tests" ${def})
    option(MUSE_MODULE_${name}_API "Build ${name} api" ${def})
endmacro()

# Modules framework (alphabetical order please)
declare_muse_module_opt(ACCESSIBILITY ON)
declare_muse_module_opt(ACTIONS ON)
declare_muse_module_opt(AUDIO ON)
option(MUSE_MODULE_AUDIO_JACK "Enable jack support" OFF)
option(MUSE_MODULE_AUDIO_EXPORT "Enable audio export" ON)

declare_muse_module_opt(CLOUD ON)
declare_muse_module_opt(DRAW ON)
declare_muse_module_opt(EXTENSIONS ON)
declare_muse_module_opt(GLOBAL ON)
declare_muse_module_opt(LANGUAGES ON)
declare_muse_module_opt(LEARN ON)
declare_muse_module_opt(MIDI ON)
declare_muse_module_opt(MPE ON)
declare_muse_module_opt(MULTIINSTANCES ON)
declare_muse_module_opt(MUSESAMPLER ON)
declare_muse_module_opt(NETWORK ON)
declare_muse_module_opt(SHORTCUTS ON)
declare_muse_module_opt(UI ON)

set(VST3_SDK_VERSION "3.7")
declare_muse_module_opt(VST OFF)
set(MUSE_MODULE_VST_VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")
# backwards compatible, will be removed later
set(VST3_SDK_PATH "" CACHE PATH "Path to VST3_SDK. SDK version >= ${VST3_SDK_VERSION} required")

declare_muse_module_opt(WORKSPACE ON)

# === Tests ===
option(MUSE_BUILD_UNIT_TESTS "Build framework unit tests" ON)

# === Debug ===
option(MUSE_MODULE_GLOBAL_LOGGER_DEBUGLEVEL "Enable logging debug level" ON)
option(MUSE_MODULE_ACCESSIBILITY_TRACE "Enable accessibility logging" OFF)
option(MUSE_MODULE_DRAW_TRACE "Trace draw objects" ON)
option(MUSE_MODULE_UI_DISABLE_MODALITY "Disable dialogs modality for testing purpose" OFF)
