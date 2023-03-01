#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define eprintf(...)    fprintf(stderr, __VA_ARGS__)
#define ARRAY_LEN(a)    (sizeof(a) / sizeof(*(a)))

#define TEST_PASS()     exit(0)
#define TEST_FAIL()     exit(1)

#define TEST_EXPECT(expr, ...) \
    if (!(expr)) {          \
        eprintf("TEST FAILED:\n" \
                "\tfile: %s\n" \
                "\tline: %d\n" \
                "\texpected: ", __FILE__, __LINE__); \
        eprintf(__VA_ARGS__); \
        eprintf("\n"); \
        TEST_FAIL(); \
    }
#define TEST_EQ(a, b)           TEST_EXPECT((a) == (b), #a " == " #b)
#define TEST_EQ_STR(a, b)       TEST_EXPECT(a && b && !strcmp(a, b), "\"%s\" == \"%s\"", a, b)


void *test_malloc(size_t size)
{
    void *ptr = malloc(size);
    TEST_EXPECT(ptr != NULL, "malloc(%zu) succeeds", size);
    return ptr;
}

int run_test(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    return run_test(argc, argv);
}
