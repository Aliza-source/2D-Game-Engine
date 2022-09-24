#pragma once
#pragma warning(disable: 4820) // Disabled warning about structre padding;
#pragma warning(disable: 5045) // Disabled warning about Spectre / Meltdown INTEL CPU vulnerability
#pragma warning(disable: 4710) // Disabled warning about function not inlined;
#pragma warning(disable: 4711) // Disabled warning about automatic function inlining;

#define SIMD

#include "Settings.h"
#include "Player.h"
#include "SystemErrors.h"
#include "Structures.h"
#include "Functions.h"
#include "Classes.h"