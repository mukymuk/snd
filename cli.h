#ifndef __CLI_H__
#define __CLI_H__

#include "cbuf.h"

void cli_init( cbuf_t * p_cbuf_cmd, cbuf_t * p_cbuf_rsp );
void cli_event(void);

#endif
