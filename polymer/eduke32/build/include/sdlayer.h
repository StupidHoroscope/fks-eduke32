// SDL interface layer
// for the Build Engine
// by Jonathon Fowler (jonof@edgenetwk.com)

#ifndef __build_interface_layer__
#define __build_interface_layer__ SDL

#include "baselayer.h"

struct sdlappicon {
	int32_t width,height;
	uint32_t *pixels;
	uint8_t *mask;
};

enum ScreenScaleMode_t
{
	SCREENSCALE_CROPPED = 0,
	SCREENSCALE_SCALED = 1,
	SCREENSCALE_STRETCHED = 2,
	SCREENSCALE_MAX
};

#else
#if (__build_interface_layer__ != SDL)
#error "Already using the " __build_interface_layer__ ". Can't now use SDL."
#endif
#endif // __build_interface_layer__

