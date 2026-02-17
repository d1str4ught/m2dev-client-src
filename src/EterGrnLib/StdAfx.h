#pragma once

#pragma warning(disable : 4786) // character 255 넘어가는거 끄기

// #include <crtdbg.h>
#include <granny.h>

#include "EterBase/Utils.h"
#include "EterBase/Debug.h"
#include "EterBase/Stl.h"

#include "Util.h"

// Armadillo nanomite protection
#ifndef NANOBEGIN
#ifdef __BORLANDC__
#define NANOBEGIN
#define NANOEND
#else
#define NANOBEGIN
#define NANOEND
#endif
#endif
