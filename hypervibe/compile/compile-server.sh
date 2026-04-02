source ~/repos/emsdk/emsdk_env.sh > /dev/null 2>&1

emcc main.cpp \
$HYPERION_LIB_DIR/platform/unix_common/utils.cpp \
$HYPERION_LIB_DIR/platform/unix_common/log.cpp \
$HYPERION_LIB_DIR/core/generation/patterns/helpers/tempo/tempo.cpp \
thread.cpp \
-I$HYPERION_LIB_DIR \
-I$HYPERION_LIB_DIR/platform/includes \
-o pattern.mjs \
-sMODULARIZE \
--no-entry \
-s EXPORTED_FUNCTIONS=_init,_process,_beat,_malloc,_free,_setParam,_setPrimary,_setSecondary,_setHighlight,_setGradientStop,_applyPalette \
-s EXPORTED_RUNTIME_METHODS=wasmMemory,HEAPU8,HEAPF32 \