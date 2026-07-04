#pragma once
// The "prefix header" we precompile. Scoped to <string> only: it is the most
// complete part of psychicstd, so a PCH comparison against libstdc++ stays fair
// (it measures PCH, not psychicstd's missing features).
#include <string>
