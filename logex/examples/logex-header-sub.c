#define LOGEX_TAG "SUB"
#include "logex.h"
LOGEX_MODULE(sub);

void 
sub_hello()
{
    fatal("Hello");
    critical("Hello");
    error("Hello");
    warning("Hello");
    info("Hello");
    verbose("Hello");
    command("Hello");
    debug("Hello");
    trace("Hello");
}
