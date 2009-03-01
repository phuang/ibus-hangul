/* vim:set et sts=4: */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_HANGUL_ENGINE	\
	(ibus_hangul_engine_get_type ())

GType   ibus_hangul_engine_get_type    (void);

void    ibus_hangul_init (IBusBus *bus);
void    ibus_hangul_exit (void);

#endif
