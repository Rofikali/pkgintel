#include <assert.h>
int test_context_behaviour(void);
int test_scan_behaviour(void);
int main(void) {
    assert(test_context_behaviour() == 0);
    assert(test_scan_behaviour() == 0);
    return 0;
}
