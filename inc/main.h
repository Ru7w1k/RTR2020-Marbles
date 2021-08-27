#pragma once
#include <windows.h>
#include <Windowsx.h>
#include <stdio.h>

#include "gl/glew.h"
#include "gl/wglew.h"
#include <gl/GL.h>

#include "AL/al.h"
#include "AL/alc.h"

#include "vmath.h"
using namespace vmath;

// enums
enum {
	RMC_ATTRIBUTE_POSITION = 0,
	RMC_ATTRIBUTE_COLOR,
	RMC_ATTRIBUTE_NORMAL,
	RMC_ATTRIBUTE_TEXTURE0,
	RMC_ATTRIBUTE_BONEIDS,
	RMC_ATTRIBUTE_BONEWEIGHTS,
};

// Resources
#define RMC_ICON 101
