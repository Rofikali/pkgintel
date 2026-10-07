#ifndef PKGINTEL_TEST_FIXTURES_H
#define PKGINTEL_TEST_FIXTURES_H
#include <stddef.h>
void make_fixture(char *root, size_t root_size);
void remove_fixture(const char *root);
void make_multi_package_fixture(char *root, size_t root_size);
void remove_multi_package_fixture(const char *root);
#endif
