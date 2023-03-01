#include "test.c"
#include "../inireader.c"

#define EXPECT_ERR_STATE(errcode) TEST_EQ(state.err, errcode)
#define EXPECT_STATE_OK() EXPECT_ERR_STATE(INI_OK)
#define RESET_ERR_STATE() (state.err = INI_OK)

struct ini_object *obj;
struct ini_state state;

void valid_comment_test(const char *input, const char *expect)
{
    RESET_ERR_STATE();
    obj = ini_read_object(&state, input);
    EXPECT_STATE_OK();
    TEST_EQ(obj->type, INI_COMMENT);
    TEST_EQ_STR(obj->comment.whole, expect);
}

void valid_section_test(const char *input, const char *expect)
{
    RESET_ERR_STATE();
    obj = ini_read_object(&state, input);
    EXPECT_STATE_OK();
    TEST_EQ(obj->type, INI_SECTION);
    TEST_EQ_STR(obj->section.name, expect);
}

void valid_key_test(const char *input, const char *expect_name,
        const char *expect_value)
{
    RESET_ERR_STATE();
    obj = ini_read_object(&state, input);
    EXPECT_STATE_OK();
    TEST_EQ(obj->type, INI_KEY);
    TEST_EQ_STR(obj->key.name, expect_name);
    TEST_EQ_STR(obj->key.value, expect_value);
}

void whitespace_test(const char *ws)
{
    RESET_ERR_STATE();

    obj = ini_read_object(&state, ws);
    EXPECT_STATE_OK();
    TEST_EQ(obj, NULL);
}

int run_test(int, char*[])
{
    EXPECT_STATE_OK();
 
    whitespace_test("");
    whitespace_test("\n");
    whitespace_test("  \t  \n");

    valid_comment_test("; comment\n", "; comment");
    valid_comment_test("# comment\n", "# comment");
    valid_comment_test("# comment     \n", "# comment");
    valid_comment_test("  ; co#m;ment\n", "; co#m;ment");
    valid_comment_test(";comment\n", ";comment");
    valid_comment_test("; comment ; sdfhj\n", "; comment ; sdfhj");

    valid_section_test("[section]\n", "section");
    valid_section_test("[   section]  \n", "section");
    valid_section_test("[   section   ]\n", "section");
    valid_section_test("[s23ec.tion]\n", "s23ec.tion");

    valid_key_test("key=\n", "key", "");
    valid_key_test("key=value\n", "key", "value");
    valid_key_test("key = value\n", "key", "value");
    valid_key_test("key = [fake section]\n", "key", "[fake section]");
    valid_key_test("  key   =  ; val123ue   \n", "key", "; val123ue");
    valid_key_test("key key = value value\n", "key key", "value value");

    RESET_ERR_STATE();
    obj = ini_read_object(&state, "key without value");
    EXPECT_ERR_STATE(INI_MISSING_SYM);
    TEST_EQ(obj, NULL);

    RESET_ERR_STATE();
    obj = ini_read_object(&state, "[section with ;bad char]\n");
    EXPECT_ERR_STATE(INI_INVALID_SYM);
    TEST_EQ(obj, NULL);

    RESET_ERR_STATE();
    obj = ini_read_object(&state, "[section_that_does_not_end_properly] ; comment\n");
    EXPECT_ERR_STATE(INI_INVALID_SYM);
    TEST_EQ(obj, NULL);
    return 0;
}
