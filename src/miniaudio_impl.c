/* Disable features we don't use to minimize binary size */
#define MA_NO_DECODING          /* we don't load files */
#define MA_NO_ENCODING          /* we don't encode/write audio */
#define MA_NO_GENERATION        /* we don't use built-in waveform generation */
#define MA_NO_RESOURCE_MANAGER  /* we don't use the resource manager */
#define MA_NO_NODE_GRAPH        /* we don't use the node graph */
#define MA_NO_ENGINE            /* we don't use the high-level engine */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
