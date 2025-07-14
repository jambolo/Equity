#pragma once

// Simple stub header to satisfy crypto dependencies
// This avoids pulling in the full utility library with all its dependencies

#ifdef DEBUG
#define ASSERT(x) do { if (!(x)) { abort(); } } while(0)
#else
#define ASSERT(x) do {} while(0)
#endif

// Add other debug macros as needed
