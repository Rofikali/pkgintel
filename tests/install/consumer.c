#include <pkgintel/pkgintel.h>

int main(void) {
    pkg_context *context = NULL;
    pkg_status status = pkg_context_create(NULL, &context);
    if (status != PKG_STATUS_OK) {
        return 1;
    }
    pkg_context_destroy(context);
    return pkg_status_string(PKG_STATUS_OK) == NULL;
}
