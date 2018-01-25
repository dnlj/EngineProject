#pragma once

#define ENGINE_MACRO_CONCAT(x, y) __ENGINE_MACRO_CONCAT__(x, y)
#define __ENGINE_MACRO_CONCAT__(x, y) x##y
