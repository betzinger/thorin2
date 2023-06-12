#include "dialects/typebug/typebug.h"

#include <thorin/plugin.h>

using namespace thorin;

extern "C" THORIN_EXPORT Plugin thorin_get_plugin() {
    return {"typebug", 
        nullptr,
        nullptr,
        nullptr
    };
}
