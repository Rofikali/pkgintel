#include <assert.h>
int test_context_behaviour(void);
int test_scan_behaviour(void);
int test_json_behaviour(void);
int test_snapshot_resource_budgets(void);
int main(void) {
    assert(test_context_behaviour() == 0);
    assert(test_scan_behaviour() == 0);
    assert(test_json_behaviour() == 0);
    assert(test_snapshot_resource_budgets() == 0);
    return 0;
}
