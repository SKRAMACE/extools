#include "logex.h"

void 
simple_hello()
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
