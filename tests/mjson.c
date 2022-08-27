#include "gun.h"
#include <check.h>

#include "../src/mjson.h"
#include <stdio.h>

static const char *json = "[{\"#\": \"asldasd\"}, {\"#\": \"item two\"}]";
static const char *blank_json = "[]";

START_TEST(test_mjson_array_iterator)
{
	int koff, klen, voff, vlen, vtype, off;

	for (off = 0; (off = mjson_next(json, strlen(json), off, &koff, &klen,
					&voff, &vlen, &vtype)) != 0;) {
		printf("key: %.*s, value: %.*s type %d\n", klen, json + koff,
		       vlen, json + voff, vtype);
	}
}
END_TEST

START_TEST(test_blank_iterator)
{
	int koff, klen, voff, vlen, vtype, off;

	for (off = 0;
	     (off = mjson_next(blank_json, strlen(blank_json), off, &koff,
			       &klen, &voff, &vlen, &vtype)) != 0;) {
		printf("blank key: %.*s, value: %.*s type %d\n", klen,
		       json + koff, vlen, json + voff, vtype);
	}
}
END_TEST

START_TEST(id_generator)
{
	char id[9];
	gun_generate_random_string(8, id);

	printf("id %s\n", id);
}
END_TEST

int main(void)
{
	Suite *s = suite_create("Gunc");
	TCase *c = tcase_create("mjson");
	TCase *id = tcase_create("ID generator");
	SRunner *runner;

	tcase_add_test(c, test_mjson_array_iterator);
	tcase_add_test(c, test_blank_iterator);
	tcase_add_test(id, id_generator);

	suite_add_tcase(s, c);
	suite_add_tcase(s, id);

	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);
}
