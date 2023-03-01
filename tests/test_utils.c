#include "test.c"
#include "../inireader.c"

void test_trim(void)
{
    char strings[][2][50] = {
        {"needs no trimming", "needs no trimming"},
        {"  \n\tleft trim", "left trim"},
        {"right trim  \n\t", "right trim"},
        {"  \t\n trim  \n\t", "trim"},
        {"", ""},
        {" \n \t  \n", ""}
    };

    for (size_t i = 0; i < ARRAY_LEN(strings); ++i) {
        char *trimmed = trim(strings[i][0]);
        char *correct = strings[i][1];
        TEST_EQ_STR(trimmed, correct);
    }
}

void test_copy_string(void)
{
    char *copy;
    // case 1: *dest == null && strlen(string) == 0
    {
        char *dest = NULL;
        size_t n;

        copy = copy_string(&dest, &n, "");

        TEST_EXPECT(n > 0, "%zu > 0", n);
        TEST_EQ_STR(copy, "");

        free(copy);
    }

    // case 2: *dest == null && strlen(string) > 0
    {
        char *dest = NULL;
        size_t n;

        copy = copy_string(&dest, &n, "123");

        TEST_EXPECT(n > 0, "%zu > 0", n);
        TEST_EQ_STR(copy, "123");

        free(copy);
    }

    // case 3: *dest != null && strlen(string) == 0
    {
        size_t n = 5;
        char *dest = test_malloc(n);

        copy = copy_string(&dest, &n, "");

        TEST_EXPECT(n > 0, "%zu > 0", n);
        TEST_EQ_STR(copy, "");

        free(copy);
    }

    // case 4: *dest != null && strlen(string) > 0
    {
        size_t n = 5;
        char *dest = test_malloc(n);

        copy = copy_string(&dest, &n, "123");

        TEST_EXPECT(n > 0, "%zu > 0", n);
        TEST_EQ_STR(copy, "123");

        free(copy);
    }

    // case 4: n < strlen(string)
    {
        size_t n = 5;
        char *dest = test_malloc(n);

        copy = copy_string(&dest, &n, "123456789");

        TEST_EXPECT(n > 5, "%zu > 5", n);
        TEST_EQ_STR(copy, "123456789");

        free(copy);
    }
}

int run_test(int argc, char *argv[])
{
    test_trim();
    test_copy_string();
    return 0;
}
