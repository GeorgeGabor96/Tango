#include "utils/random.h"


internal void
random_init() {
    srand((u32)time(0));
}


internal bool
random_get_bool(f32 chance) {
    bool result = ((f64)rand() / (f64)RAND_MAX) <= (f64)chance ? TRUE : FALSE;
    return result;
}