#first run source ~/repos/emsdk/emsdk_env.sh

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
# -pthread \
# -s USE_PTHREADS=1 
# -s STANDALONE_WASM=1 \
# -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']"

#-sSINGLE_FILE=1 \
#$HYPERION_LIB_DIR/platform/unix_common/log.cpp \

# $HYPERION_LIB_DIR/platform/unix_common/thread.cpp \

# emcc hello_react.c \
# $HYPERION_LIB_DIR/platform/unix_common/utils.cpp \
# $HYPERION_LIB_DIR/core/generation/patterns/helpers/tempo/tempo.cpp \
# -I$HYPERION_LIB_DIR \
# -I$HYPERION_LIB_DIR/platform/includes \
# -o hello_react.js \
# -sMODULARIZE -sSINGLE_FILE=1 \
# -sEXPORTED_FUNCTIONS=_hello_react,_process_data,_malloc,_free,getValue \
# -sEXPORTED_RUNTIME_METHODS=ccall