/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include <modem/at_parser.h>

/* Imported from `at_cmd_parser` test. */
#define SINGLELINE_PARAM_COUNT      5
#define PDULINE_PARAM_COUNT         4
#define SINGLEPARAMLINE_PARAM_COUNT 1
#define EMPTYPARAMLINE_PARAM_COUNT  6
#define CERTIFICATE_PARAM_COUNT     5

/* Imported from `at_cmd_parser` test. */
static const char * const singleline[] = {
	"+CEREG: 2,\"76C1\",\"0102DA04\", 7\r\n+CME ERROR: 10\r\n",
	"+CEREG: 2,\"76C1\",\"0102DA04\", 7\r\nOK\r\n",
	"+CEREG: 2,\"76C1\",\"0102DA04\", 7\r\n"
};

/* Imported from `at_cmd_parser` test. */
static const char * const multiline[] = {
	"+CGEQOSRDP: 0,0,,\r\n"
	"+CGEQOSRDP: 1,2,,\r\n"
	"+CGEQOSRDP: 2,4,,,1,65280000\r\n",
	"+CGEQOSRDP: 0,0,,\r\n"
	"+CGEQOSRDP: 1,2,,\r\n"
	"+CGEQOSRDP: 2,4,,,1,65280000\r\nOK\r\n"
	"+CGEQOSRDP: 0,0,,\r\n"
	"+CGEQOSRDP: 1,2,,\r\n"
	"+CGEQOSRDP: 2,4,,,1,65280000\r\nERROR\r\n"
};

/* Imported from `at_cmd_parser` test. */
static const char * const pduline[] = {
	"+CMT: \"12345678\", 24\r\n"
	"06917429000171040A91747966543100009160402143708006C8329BFD0601\r\n+CME ERROR: 123\r\n",
	"+CMT: \"12345678\", 24\r\n"
	"06917429000171040A91747966543100009160402143708006C8329BFD0601\r\nOK\r\n",
	"\r\n+CMT: \"12345678\", 24\r\n"
	"06917429000171040A91747966543100009160402143708006C8329BFD0601\r\n\r\nOK\r\n",
	"+CMT: \"12345678\", 24\r\n"
	"06917429000171040A91747966543100009160402143708006C8329BFD0601\r\n",
	"\r\n+CMT: \"12345678\", 24\r\n"
	"06917429000171040A91747966543100009160402143708006C8329BFD0601\r\n"
};

/* Imported from `at_cmd_parser` test. */
static const char * const singleparamline[] = {
	"mfw_nrf9160_0.7.0-23.prealpha\r\n+CMS ERROR: 123\r\n",
	"mfw_nrf9160_0.7.0-23.prealpha\r\nOK\r\n",
	"mfw_nrf9160_0.7.0-23.prealpha\r\n"
};

/* Imported from `at_cmd_parser` test. */
static const char * const emptyparamline[] = {
	"+CPSMS: 1,,,\"10101111\",\"01101100\"\r\n",
	"+CPSMS: 1,,,\"10101111\",\"01101100\"\r\nOK\r\n",
	"+CPSMS: 1,,,\"10101111\",\"01101100\"\r\n+CME ERROR: 123\r\n"
};

/* Imported from `at_cmd_parser` test. */
static const char * const certificate =
	"%CMNG: 12345678, 0, \"978C...02C4\","
	"\"-----BEGIN CERTIFICATE-----"
	"MIIBc464..."
	"...bW9aAa4"
	"-----END CERTIFICATE-----\"\r\nERROR\r\n";

static void test_params_before(void *fixture)
{
	ARG_UNUSED(fixture);
}

static void test_params_after(void *fixture)
{
	ARG_UNUSED(fixture);
}

/* Ported from `at_cmd_parser` test. */
ZTEST(at_parser, test_at_token_line_parse_quoted_string)
{
	int ret;
	const char *remainder = NULL;
	struct at_token tokens[32] = { 0 };
	char tmpbuf[32];
	uint32_t tmpbuf_len;
	int32_t tmpint;

	const char *str1 = "+TEST:1,\"Hello World!\"\r\n";
	const char *str2 = "%TEST: 1, \"Hello World!\"\r\n";
	const char *str3 = "#TEST:1,\"Hello World!\"\r\n"
			   "+TEST: 2, \"FOOBAR\"\r\n";

	/* String without spaces between parameters (str1)*/
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, &remainder);
	zassert_ok(ret);
	zassert_equal(*remainder, '\0');

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 3);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_QUOTED_STRING);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("+TEST"), tmpbuf_len);
	zassert_mem_equal("+TEST", tmpbuf, tmpbuf_len);

	ret = at_token_int32_get(&tokens[1], &tmpint);
	zassert_ok(ret);
	zassert_equal(tmpint, 1);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[2], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("Hello World!"), tmpbuf_len);
	zassert_mem_equal("Hello World!", tmpbuf, tmpbuf_len);

	/* String with spaces between parameters (str2)*/
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, &remainder);
	zassert_ok(ret);
	zassert_equal(*remainder, '\0');

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 3);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_QUOTED_STRING);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("%TEST"), tmpbuf_len);
	zassert_mem_equal("%TEST", tmpbuf, tmpbuf_len);

	ret = at_token_int32_get(&tokens[1], &tmpint);
	zassert_ok(ret);
	zassert_equal(tmpint, 1);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[2], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("Hello World!"), tmpbuf_len);
	zassert_mem_equal("Hello World!", tmpbuf, tmpbuf_len);

	/* String with multiple notifications (str3) */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, &remainder);
	zassert_equal(ret, -EAGAIN);
	zassert_not_equal(*remainder, '\0');

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 3);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_QUOTED_STRING);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("#TEST"), tmpbuf_len);
	zassert_mem_equal("#TEST", tmpbuf, tmpbuf_len);

	ret = at_token_int32_get(&tokens[1], &tmpint);
	zassert_ok(ret);
	zassert_equal(tmpint, 1);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[2], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("Hello World!"), tmpbuf_len);
	zassert_mem_equal("Hello World!", tmpbuf, tmpbuf_len);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), remainder, &remainder);
	zassert_ok(ret);
	zassert_equal(*remainder, '\0');

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 3);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_QUOTED_STRING);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("+TEST"), tmpbuf_len);
	zassert_mem_equal("+TEST", tmpbuf, tmpbuf_len);

	ret = at_token_int32_get(&tokens[1], &tmpint);
	zassert_ok(ret);
	zassert_equal(tmpint, 2);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[2], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("FOOBAR"), tmpbuf_len);
	zassert_mem_equal("FOOBAR", tmpbuf, tmpbuf_len);
}

/* Ported from `at_cmd_parser` test. */
ZTEST(at_parser, test_at_token_line_parse_empty)
{
	int ret;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+TEST: 1,\r\n";
	const char *str2 = "+TEST: ,1\r\n";
	const char *str3 = "+TEST: 1,,\"Hello World!\"";
	const char *str4 = "+TEST: 1,,,\r\n";
	const char *str5 = "+TEST: ,,,1\r\n";

	/* Empty parameter at the end of the parameter list */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 3);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_EMPTY);

	/* Empty parameter at the beginning of the parameter list */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 3);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_EMPTY);

	/* Empty parameter between two other parameter types */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 4);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_EMPTY);

	/* 3 empty parameter at the end of the parameter list */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str4, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 5);
	for (int i = 2; i < 4; i++) {
		zassert_equal(tokens[i].type, AT_TOKEN_TYPE_EMPTY);
	}

	/* 3 empty parameter at the beginning of the parameter list */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str5, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 5);
	for (int i = 1; i < 3; i++) {
		zassert_equal(tokens[i].type, AT_TOKEN_TYPE_EMPTY);
	}
}

/* Ported from `at_cmd_parser` test. */
ZTEST(at_parser, test_at_token_line_parse_testcases)
{
	int ret;
	const char *remainder;
	struct at_token tokens[32] = { 0 };

	/* Try to parse the singleline string */
	for (size_t i = 0; i < ARRAY_SIZE(singleline); i++) {
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), singleline[i], NULL);
		zassert_ok(ret);
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, SINGLELINE_PARAM_COUNT);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_QUOTED_STRING);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_QUOTED_STRING);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_INT);

	/* Try to parse the pduline string */
	for (size_t i = 0; i < ARRAY_SIZE(pduline); i++) {
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), pduline[i], NULL);
		zassert_ok(ret);
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, PDULINE_PARAM_COUNT);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_QUOTED_STRING);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_STRING);

	/* Try to parse the singleparamline string */
	for (size_t i = 0; i < ARRAY_SIZE(singleparamline); i++) {
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), singleparamline[i], NULL);
		zassert_ok(ret);
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, SINGLEPARAMLINE_PARAM_COUNT);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_STRING);

	/* Try to parse the string containing empty/optional parameters  */
	for (size_t i = 0; i < ARRAY_SIZE(emptyparamline); i++) {
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), emptyparamline[i], NULL);
		zassert_ok(ret);
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, EMPTYPARAMLINE_PARAM_COUNT);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_QUOTED_STRING);
	zassert_equal(tokens[5].type, AT_TOKEN_TYPE_QUOTED_STRING);

	/* Try to parse the string containing multiple notifications  */
	for (size_t i = 0; i < ARRAY_SIZE(multiline); i++) {
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), multiline[i], &remainder);
		zassert_equal(ret, -EAGAIN);

		ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
		zassert_equal(ret, 5);
		zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
		zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[3].type, AT_TOKEN_TYPE_EMPTY);
		zassert_equal(tokens[4].type, AT_TOKEN_TYPE_EMPTY);

		/* 2nd iteration */
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), remainder, &remainder);
		zassert_equal(ret, -EAGAIN);

		ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
		zassert_equal(ret, 5);
		zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
		zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[3].type, AT_TOKEN_TYPE_EMPTY);
		zassert_equal(tokens[4].type, AT_TOKEN_TYPE_EMPTY);

		/* 3rd iteration */
		ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), remainder, &remainder);
		zassert_ok(ret);

		ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
		zassert_equal(ret, 7);
		zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
		zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[3].type, AT_TOKEN_TYPE_EMPTY);
		zassert_equal(tokens[4].type, AT_TOKEN_TYPE_EMPTY);
		zassert_equal(tokens[5].type, AT_TOKEN_TYPE_INT);
		zassert_equal(tokens[6].type, AT_TOKEN_TYPE_INT);
	}

	/* Try to parse the string containing certificate data  */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), certificate, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, CERTIFICATE_PARAM_COUNT);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_QUOTED_STRING);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_QUOTED_STRING);
}

/* Ported from `at_cmd_parser` test. */
ZTEST(at_parser, test_at_cmd_type_get_cmd_set)
{
	int ret;
	struct at_token tokens[32] = { 0 };
	enum at_token_type type;
	char tmpbuf[64];
	uint32_t tmpbuf_len;
	int16_t tmpshrt;
	uint16_t tmpushrt;

	/* CGMI */
	static const char at_cmd_cgmi[] = "AT+CGMI";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_cmd_cgmi, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 1);

	type = at_cmd_type_get(at_cmd_cgmi);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_SET);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("AT+CGMI", tmpbuf, tmpbuf_len);

	/* CCLK */
	static const char at_cmd_cclk[] = "AT+CCLK=\"18/12/06,22:10:00+08\"";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_cmd_cclk, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 2);

	type = at_cmd_type_get(at_cmd_cclk);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_SET);

	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_CMD_SET);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_QUOTED_STRING);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("AT+CCLK", tmpbuf, tmpbuf_len);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[1], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("18/12/06,22:10:00+08", tmpbuf, tmpbuf_len);

	/* XSYSTEMMODE */
	static const char at_cmd_xsystemmode[] = "AT%XSYSTEMMODE=1,2,3,4";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_cmd_xsystemmode, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 5);

	type = at_cmd_type_get(at_cmd_xsystemmode);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_SET);

	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_CMD_SET);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &tmpshrt);
	zassert_ok(ret);
	zassert_equal(tmpshrt, 1);

	ret = at_token_int16_get(&tokens[2], &tmpshrt);
	zassert_ok(ret);
	zassert_equal(tmpshrt, 2);

	ret = at_token_int16_get(&tokens[3], &tmpshrt);
	zassert_ok(ret);
	zassert_equal(tmpshrt, 3);

	ret = at_token_int16_get(&tokens[4], &tmpshrt);
	zassert_ok(ret);
	zassert_equal(tmpshrt, 4);

	ret = at_token_uint16_get(&tokens[1], &tmpushrt);
	zassert_ok(ret);
	zassert_equal(tmpushrt, 1);

	ret = at_token_uint16_get(&tokens[2], &tmpushrt);
	zassert_ok(ret);
	zassert_equal(tmpushrt, 2);

	ret = at_token_uint16_get(&tokens[3], &tmpushrt);
	zassert_ok(ret);
	zassert_equal(tmpushrt, 3);

	ret = at_token_uint16_get(&tokens[4], &tmpushrt);
	zassert_ok(ret);
	zassert_equal(tmpushrt, 4);

	/* AT */
	static const char lone_at_cmd[] = "AT";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), lone_at_cmd, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 1);

	type = at_cmd_type_get(lone_at_cmd);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_SET);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("AT", tmpbuf, tmpbuf_len);

	/* CLAC */
	static const char at_cmd_clac[] = "AT+CLAC\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_cmd_clac, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 1);

	type = at_cmd_type_get(at_cmd_clac);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_SET);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("AT+CLAC", tmpbuf, tmpbuf_len);

	/* CLAC RSP */
	static const char at_clac_rsp[] = "AT+CLAC\r\nAT+COPS\r\nAT%COPS\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_clac_rsp, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 1);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal(at_clac_rsp, tmpbuf, tmpbuf_len);
}

/* Ported from `at_cmd_parser` test. */
ZTEST(at_parser, test_at_cmd_type_get_cmd_read)
{
	int ret;
	char tmpbuf[64];
	uint32_t tmpbuf_len;
	struct at_token tokens[32] = { 0 };
	enum at_token_type type;

	static const char at_cmd_cfun_read[] = "AT+CFUN?";

	type = at_cmd_type_get(at_cmd_cfun_read);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_READ);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_cmd_cfun_read, NULL);
	zassert_ok(ret, "%d", ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 1);

	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_CMD_READ);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("AT+CFUN", tmpbuf, tmpbuf_len);
}

/* Ported from `at_cmd_parser` test. */
ZTEST(at_parser, test_at_cmd_type_get_cmd_test)
{
	int ret;
	char tmpbuf[64];
	uint32_t tmpbuf_len;
	struct at_token tokens[32] = { 0 };
	enum at_token_type type;

	static const char at_cmd_cfun_test[] = "AT+CFUN=?";

	type = at_cmd_type_get(at_cmd_cfun_test);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_TEST);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), at_cmd_cfun_test, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 1);

	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_CMD_TEST);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_mem_equal("AT+CFUN", tmpbuf, tmpbuf_len);
}

ZTEST(at_parser, test_at_parser_init_einval)
{
	int ret;
	struct at_parser parser = { 0 };

	ret = at_parser_init(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_init(&parser, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_init(NULL, "AT+CFUN?");
	zassert_equal(ret, -EINVAL);
}

ZTEST(at_parser, test_at_parser_init_erange)
{
	int ret;
	struct at_parser parser = {0};

	/* 1) String too short. */
	ret = at_parser_init(&parser, "");
	zassert_equal(ret, -ERANGE);

	/* 2) String too large. */
	static char str[UINT16_MAX + 8] = {0};

	for (int i = 0; i < UINT16_MAX + 1; i++) {
		str[i] = 'A';
	}

	zassert_true(strlen(str) > UINT16_MAX);

	ret = at_parser_init(&parser, str);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_parser_init)
{
	int ret;
	struct at_parser parser = { 0 };

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);
}

ZTEST(at_parser, test_at_parser_tok_einval)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };

	ret = at_parser_tok(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_tok(&parser, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_tok(NULL, &tok);
	zassert_equal(ret, -EINVAL);

	const char *str1 = "+TEST: 1,2\r\nOK\r\n";

	/* Initialize and then modify the parser's `ptr` cursor. */
	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	parser.ptr = NULL;

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EINVAL);
}

ZTEST(at_parser, test_at_parser_tok_eperm)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };

	/* The parser has not been initialized. */
	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EPERM);
}

ZTEST(at_parser, test_at_parser_tok_eio)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_RESP);

	/* There is nothing left to parse. */
	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EIO);
}

ZTEST(at_parser, test_at_parser_tok_invalid_token_ebadmsg)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };

	/* Invalid token. `#31` is not a valid token. */
	const char *str1 = "+NOTIF: 1,#31,1";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(tok.type, AT_TOKEN_TYPE_INT);

	/* Invalid token. ` !+NOTIF: ` is not a valid token. */
	const char *str2 = " !+NOTIF: 1,2,3";

	ret = at_parser_init(&parser, str2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Invalid token. `+!NOTIF: ` is not a valid token. */
	const char *str3 = "+!NOTIF: 1,2,3";

	ret = at_parser_init(&parser, str3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Invalid token. `%!NOTIF: ` is not a valid token. */
	const char *str4 = "%!NOTIF: 1,2,3";

	ret = at_parser_init(&parser, str4);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Invalid token. Cannot have null terminator inside quoted string. */
	const char *str5 = "+NOTIF: \"\0ABCD\"";

	ret = at_parser_init(&parser, str5);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Invalid token. Cannot have null terminator inside quoted string. */
	const char *str6 = "+NOTIF: \"A\0BCD\"";

	ret = at_parser_init(&parser, str6);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Invalid token. Cannot have non-quoted string inside parenthesis. */
	const char *str7 = "+NOTIF: (ABCD,1)";

	ret = at_parser_init(&parser, str7);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);

	/* Invalid token. A non-quoted string cannot start with `-`. */
	const char *str8 = "-ABCD";

	ret = at_parser_init(&parser, str8);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &tok);
	zassert_equal(ret, -EBADMSG);
}

ZTEST(at_parser, test_at_parser_tok_invalid_constraints_ebadmsg)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_len = sizeof(buffer);

	/* 1) At most one command per line. */
	const char *str1 = "AT+TEST=AT+TEST?\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);
	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	/* 2) There must be at least one of either command or notification if there is more than
	 * one subparameter (except non-quoted string).
	 */
	const char *str2 = "1,\"test\",2\r\n";

	ret = at_parser_init(&parser, str2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* 3) If there is more than one notification, then there must be at least one subparameter
	 * (belonging to the first notification line).
	 */
	const char *str3 = "+NOTIF: #TEST: \r\nOK\r\n";

	ret = at_parser_init(&parser, str3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret, "%d", ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	/* 4) For each notification line, there must be at least one subparameter.
	 */
	const char *str4= "+NOTIF: 1\r\n#TEST: +TEST:\r\nOK\r\n";

	ret = at_parser_init(&parser, str4);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("#TEST"), buffer_len);
	zassert_mem_equal("#TEST", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("#TEST"), buffer_len);
	zassert_mem_equal("#TEST", buffer, buffer_len);
}

ZTEST(at_parser, test_at_parser_tok_last_subparameter_ebadmsg)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};

	/* 1) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by an integer rather than a CRLF. Thus, the string is
	 * malformed.
	 */
	const char *str1 = "+NOTIF: 1,2,\"TEST\"9,(1,7,2,0),,3,\"TEST2, \",\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	/* 2) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by only a CR rather than a CRLF. Thus, the string is
	 * malformed.
	 */
	const char *str2 = "+NOTIF: 1,2,\"TEST\"\r9,(1,7,2,0),,3,\"TEST2, \",\r\nOK\r\n";

	ret = at_parser_init(&parser, str2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	/* 3) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by only a CR which is also the last character of the
	 * string. Thus, the string is malformed.
	 */
	const char *str3 = "+NOTIF: 1,2,\"TEST\"\r";

	ret = at_parser_init(&parser, str3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	/* 4) The last subparamter of the line is empty, however it is followed only by a CR which
	 * is also the last character of the string. Thus, the string is malformed.
	 */
	const char *str4 = "+NOTIF: 1,2,\r";

	ret = at_parser_init(&parser, str4);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_parser_tok(&parser, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);
}

ZTEST(at_parser, test_at_parser_tok_long_string)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	int32_t val;
	char buffer[32] = {0};
	size_t buffer_len = sizeof(buffer);

	const char *str = "+NOTIF: ,178,(1,2-3,\"a\"-\"b\"),+21,\"TEST\",-9,,\"TEST2, \",\r\n"
			  "#ABBA: 1,2\r\n"
			  "%TEST: 1\r\n"
			  "OK\r\n";

	ret = at_parser_init(&parser, str);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_EMPTY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 178);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 21);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("TEST"), buffer_len);
	zassert_mem_equal("TEST", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, -9);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_EMPTY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("TEST2, "), buffer_len);
	zassert_mem_equal("TEST2, ", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_EMPTY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("#ABBA"), buffer_len);
	zassert_mem_equal("#ABBA", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 2);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("%TEST"), buffer_len);
	zassert_mem_equal("%TEST", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_len);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_len);
}

ZTEST(at_parser, test_at_parser_tok_long_string_spaces)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	int32_t val;
	char buffer[32] = {0};
	size_t buffer_len = sizeof(buffer);

	const char *str = "+NOTIF: , 178, (1,2-3,\"a\"-\"b\"), +21, \"TEST\", -9, ,\"TEST2, \",\r\n"
			  "#ABBA:1, 2\r\n"
			  "%TEST: 1\r\n"
			  "OK\r\n";

	ret = at_parser_init(&parser, str);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_EMPTY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 178);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 21);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("TEST"), buffer_len);
	zassert_mem_equal("TEST", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, -9);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_EMPTY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("TEST2, "), buffer_len);
	zassert_mem_equal("TEST2, ", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_EMPTY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("#ABBA"), buffer_len);
	zassert_mem_equal("#ABBA", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 2);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("%TEST"), buffer_len);
	zassert_mem_equal("%TEST", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_len);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_len);
}

ZTEST(at_parser, test_at_parser_tok_resp_errors)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	int32_t val;
	char buffer[32] = {0};
	size_t buffer_len = sizeof(buffer);

	/* ERROR */
	const char *str1 = "+NOTIF: 1\r\nERROR\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nERROR\r\n"), buffer_len);
	zassert_mem_equal("\r\nERROR\r\n", buffer, buffer_len);

	/* +CME ERROR */
	const char *str2 = "+NOTIF: 1\r\n+CME ERROR: 123\r\n";

	ret = at_parser_init(&parser, str2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("\r\n+CME ERROR: 123\r\n"), buffer_len);
	zassert_mem_equal("\r\n+CME ERROR: 123\r\n", buffer, buffer_len);

	/* +CMS ERROR */
	const char *str3 = "+NOTIF: 1\r\n+CMS ERROR: 123\r\n";

	ret = at_parser_init(&parser, str3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("\r\n+CMS ERROR: 123\r\n"), buffer_len);
	zassert_mem_equal("\r\n+CMS ERROR: 123\r\n", buffer, buffer_len);
}

ZTEST(at_parser, test_at_parser_seek_einval)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };
	const char *at = "+TEST: 1,2\r\nOK\r\n";
	size_t index = 1;

	ret = at_parser_seek(NULL, index, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_seek(&parser, index, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_seek(NULL, index, &tok);
	zassert_equal(ret, -EINVAL);

	/* Initialize and then modify the parser's `ptr` cursor. */
	ret = at_parser_init(&parser, at);
	zassert_ok(ret);

	parser.ptr = NULL;

	ret = at_parser_seek(&parser, index, &tok);
	zassert_equal(ret, -EINVAL);
}

ZTEST(at_parser, test_at_parser_seek_eperm)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };

	/* The parser has not been initialized. */
	ret = at_parser_seek(&parser, 0, &tok);
	zassert_equal(ret, -EPERM);
}

ZTEST(at_parser, test_at_parser_seek_eio)
{
	int ret;
	struct at_parser parser = { 0 };
	struct at_token tok = { 0 };

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 4, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_RESP);

	ret = at_parser_seek(&parser, 5, &tok);
	zassert_equal(ret, -EIO);
}

ZTEST(at_parser, test_at_parser_seek_invalid_token_ebadmsg)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token tok = {0};

	/* Invalid token. */
	const char *str1 = "+NOTIF: 1,#31,1";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 3, &tok);
	zassert_equal(ret, -EBADMSG);
}

ZTEST(at_parser, test_at_parser_seek_invalid_constraints_ebadmsg)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_len = sizeof(buffer);

	/* 1) At most one command per line. */
	const char *str1 = "AT+TEST=AT+TEST?\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 0, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_parser_seek(&parser, 1, &token);
	zassert_equal(ret, -EBADMSG);
	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	/* 2) There must be at least one of either command or notification if there is more than
	 * one subparameter (except non-quoted string).
	 */
	const char *str2 = "1,\"test\",2\r\n";

	ret = at_parser_init(&parser, str2);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 0, &token);
	zassert_equal(ret, -EBADMSG);

	/* 3) If there is more than one notification, then there must be at least one subparameter
	 * (belonging to the first notification line).
	 */
	const char *str3 = "+NOTIF: #TEST: \r\nOK\r\n";

	ret = at_parser_init(&parser, str3);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 0, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	ret = at_parser_seek(&parser, 1, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret, "%d", ret);
	zassert_equal(strlen("+NOTIF"), buffer_len);
	zassert_mem_equal("+NOTIF", buffer, buffer_len);

	/* 4) For each notification line, there must be at least one subparameter.
	 */
	const char *str4 = "+NOTIF: 1\r\n#TEST: +TEST:\r\nOK\r\n";

	ret = at_parser_init(&parser, str4);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 2, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("#TEST"), buffer_len);
	zassert_mem_equal("#TEST", buffer, buffer_len);

	ret = at_parser_seek(&parser, 3, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("#TEST"), buffer_len);
	zassert_mem_equal("#TEST", buffer, buffer_len);
}

ZTEST(at_parser, test_at_parser_seek_last_subparameter_ebadmsg)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};

	/* 1) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by an integer rather than a CRLF. Thus, the string is
	 * malformed.
	 */
	const char *str1 = "+NOTIF: ,178,21,\"TEST\"9,(1,7,2,0),,3,\"TEST2, \",\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* Seeking directly towards the malformed token. */
	ret = at_parser_seek(&parser, 4, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* 2) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by only a CR rather than a CRLF. Thus, the string is
	 * malformed.
	 */
	const char *str2 = "+NOTIF: ,178,21,\"TEST\"\r9,(1,7,2,0),,3,\"TEST2, \",\r\nOK\r\n";

	ret = at_parser_init(&parser, str2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* Seeking directly towards the malformed token. */
	ret = at_parser_seek(&parser, 4, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* 3) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by only a CR which is also the last character of the
	 * string. Thus, the string is malformed.
	 */
	const char *str3 = "+NOTIF: 1,2,\"TEST\"\r";

	ret = at_parser_init(&parser, str3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* Seeking directly towards the malformed token. */
	ret = at_parser_seek(&parser, 3, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* 4) The last subparamter of the line is empty, however it is followed only by a CR which
	 * is also the last character of the string. Thus, the string is malformed.
	 */
	const char *str4 = "+NOTIF: 1,2,\r";

	ret = at_parser_init(&parser, str4);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	/* Seeking directly towards the malformed token. */
	ret = at_parser_seek(&parser, 3, &token);
	zassert_equal(ret, -EBADMSG);

	/* Unchanged. */
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);
}

ZTEST(at_parser, test_at_parser_seek_erange)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token tok = {0};
	uint32_t val;

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 2, &tok);
	zassert_ok(ret);
	zassert_equal(tok.type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tok, &val);
	zassert_ok(ret);
	zassert_equal(val, 2);

	/* Let's try going backwards. */
	ret = at_parser_seek(&parser, 1, &tok);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_parser_seek)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	uint32_t val;
	char buffer[32] = {0};
	size_t buffer_len = sizeof(buffer);

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_seek(&parser, 2, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 2);

	ret = at_parser_seek(&parser, 4, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_len = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_len);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_len);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_len);
}

ZTEST(at_parser, test_at_token_line_parse_einval)
{
	int ret;
	struct at_token tokens[32] = { 0 };
	const char *remainder = NULL;

	const char *at = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_token_line_parse(NULL, ARRAY_SIZE(tokens), at, &remainder);
	zassert_equal(ret, -EINVAL);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), NULL, &remainder);
	zassert_equal(ret, -EINVAL);
}

ZTEST(at_parser, test_at_token_line_parse_erange)
{
	int ret;
	struct at_token tokens[32] = { 0 };
	const char *remainder = NULL;

	/* 1) String too short. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), "", &remainder);
	zassert_equal(ret, -ERANGE);

	/* 2) String too large. */
	static char str[UINT16_MAX + 8] = {0};

	for (int i = 0; i < UINT16_MAX + 1; i++) {
		str[i] = 'A';
	}

	zassert_true(strlen(str) > UINT16_MAX);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str, &remainder);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_token_line_parse_eagain)
{
	int ret;
	struct at_token tokens[32] = { 0 };
	const char *remainder = NULL;
	const char *multi = "+CGEQOSRDP: 0,0,,\r\n"
			    "+CGEQOSRDP: 1,2,,\r\n"
			    "+CGEQOSRDP: 2,4,,,1,65280000\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), multi, &remainder);
	zassert_equal(ret, -EAGAIN);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), remainder, &remainder);
	zassert_equal(ret, -EAGAIN);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), remainder, &remainder);
	zassert_equal(ret, 0);
}

ZTEST(at_parser, test_at_token_line_parse_ebadmsg)
{
	int ret;
	struct at_token tokens[32] = { 0 };

	/* The notification ID should not contain anything other than [A-Za-z0-9]. */
	const char *str1 = "+NOTIF-: 1,2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_equal(ret, -EBADMSG);
}

ZTEST(at_parser, test_at_token_line_parse_invalid_constraints_ebadmsg)
{
	int ret;
	struct at_token tokens[32] = { 0 };

	/* At most one command per line. */
	const char *str1 = "AT+TEST=AT+TEST?\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_equal(ret, -EBADMSG);

	/* There must be at least one of either command or notification if there is more than
	 * one subparameter (except non-quoted string).
	 */
	const char *str2 = "1,\"test\",2\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_equal(ret, -EBADMSG);

	/* If there is more than one notification, then there must be at least one subparameter
	 * (belonging to the first notification line).
	 */
	const char *str3 = "+NOTIF: #TEST: \r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, NULL);
	zassert_equal(ret, -EBADMSG);
}

ZTEST(at_parser, test_at_token_line_parse_last_subparameter_ebadmsg)
{
	int ret;
	struct at_token tokens[32] = {0};

	/* 1) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by an integer rather than a CRLF. Thus, the string is
	 * malformed.
	 */
	const char *str1 = "+NOTIF: ,178,21,\"TEST\"9,(1,7,2,0),,3,\"TEST2, \",\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_equal(ret, -EBADMSG);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 4);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_INVALID);

	/* 2) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by only a CR rather than a CRLF. Thus, the string is
	 * malformed.
	 */
	const char *str2 = "+NOTIF: ,178,21,\"TEST\"\r9,(1,7,2,0),,3,\"TEST2, \",\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_equal(ret, -EBADMSG);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 4);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_INVALID);

	/* 3) "TEST" looks like the last subparameter of the line since it is missing a trailing
	 * comma, however it is followed by only a CR which is also the last character of the
	 * string. Thus, the string is malformed.
	 */
	const char *str3 = "+NOTIF: ,178,21,\"TEST\"\r";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, NULL);
	zassert_equal(ret, -EBADMSG);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 4);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_INVALID);

	/* 4) The last subparamter of the line is empty, however it is followed only by a CR which
	 * is also the last character of the string. Thus, the string is malformed.
	 */
	const char *str4 = "+NOTIF: ,178,21,\r";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str4, NULL);
	zassert_equal(ret, -EBADMSG);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 4);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_INVALID);
}

ZTEST(at_parser, test_at_token_line_parse_string)
{
	int ret;
	const char *remainder = NULL;
	struct at_token tokens[32] = { 0 };
	char tmpbuf[32];
	uint32_t tmpbuf_len;

	const char *str1 = "+CGEV: ME PDN ACT 0\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, &remainder);
	zassert_ok(ret);
	zassert_equal(*remainder, '\0');

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 2);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_STRING);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[0], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("+CGEV"), tmpbuf_len);
	zassert_mem_equal("+CGEV", tmpbuf, tmpbuf_len);

	tmpbuf_len = sizeof(tmpbuf);
	ret = at_token_string_get(&tokens[1], tmpbuf, &tmpbuf_len);
	zassert_ok(ret);
	zassert_equal(strlen("ME PDN ACT 0"), tmpbuf_len);
	zassert_mem_equal("ME PDN ACT 0", tmpbuf, tmpbuf_len);
}

ZTEST(at_parser, test_at_token_line_parse_truncated)
{
	int ret;
	struct at_token tokens[32] = { 0 };
	/* Only consider the first 2 tokens. */
	size_t count = 2;

	const char *str1 = "+NOTIF: 1,2,3\r\n";

	ret = at_token_line_parse(tokens, count, str1, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, count);
	zassert_equal(ret, count);
}

ZTEST(at_parser, test_at_token_uint16_get_einval)
{
	int ret;
	uint16_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	/* 1) Invalid input pointers. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_token_uint16_get(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_uint16_get(&tokens[1], NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_uint16_get(NULL, &val);
	zassert_equal(ret, -EINVAL);

	/* 2) The token is pointing to a null string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = NULL;

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 3) The token is pointing to an empty string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = "";

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 4) The length of the token is zero. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = 0;

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 5) The length of the token is larger than the length of the string it points to. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = strlen(tokens[1].start) + 1;

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 6) The token has an invalid type. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	/* Override type with anything other than `AT_TOKEN_TYPE_INT`. */
	for (int i = AT_TOKEN_TYPE_INVALID; i < AT_TOKEN_TYPE_RESP; i++) {
		if (i != AT_TOKEN_TYPE_INT) {
			tokens[1].type = i;
			ret = at_token_uint16_get(&tokens[1], &val);
			zassert_equal(ret, -EINVAL);
		}
	}
}

ZTEST(at_parser, test_at_token_uint16_get_erange)
{
	int ret;
	uint16_t val;
	struct at_token tokens[32] = { 0 };

	char str1[64] = {0};

	snprintf(str1, sizeof(str1), "+NOTIF: %d,-2,3\r\nOK\r\n", UINT16_MAX + 1);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_token_uint16_get)
{
	int ret;
	uint16_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %d,-2,3\r\nOK\r\n", UINT16_MAX);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_uint16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, UINT16_MAX);
}

ZTEST(at_parser, test_at_token_int16_get_einval)
{
	int ret;
	int16_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	/* 1) Invalid input pointers. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_token_int16_get(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_int16_get(&tokens[1], NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_int16_get(NULL, &val);
	zassert_equal(ret, -EINVAL);

	/* 2) The token is pointing to a null string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = NULL;

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 3) The token is pointing to an empty string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = "";

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 4) The length of the token is zero. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = 0;

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 5) The length of the token is larger than the length of the string it points to. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = strlen(tokens[1].start) + 1;

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 6) The token has an invalid type. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	/* Override type with anything other than `AT_TOKEN_TYPE_INT`. */
	for (int i = AT_TOKEN_TYPE_INVALID; i < AT_TOKEN_TYPE_RESP; i++) {
		if (i != AT_TOKEN_TYPE_INT) {
			tokens[1].type = i;
			ret = at_token_int16_get(&tokens[1], &val);
			zassert_equal(ret, -EINVAL);
		}
	}
}

ZTEST(at_parser, test_at_token_int16_get_erange)
{
	int ret;
	int16_t val;
	struct at_token tokens[32] = { 0 };

	char str1[64] = {0};

	snprintf(str1, sizeof(str1), "+NOTIF: %d,-2,3\r\nOK\r\n", INT16_MAX + 1);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int16_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %d,-2,3\r\nOK\r\n", INT16_MIN - 1);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int16_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_token_int16_get)
{
	int ret;
	int16_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %d,-2,3\r\nOK\r\n", INT16_MAX);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, INT16_MAX);

	char str3[64] = {0};

	snprintf(str3, sizeof(str3), "+NOTIF: %d,-2,3\r\nOK\r\n", INT16_MIN);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int16_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, INT16_MIN);
}

ZTEST(at_parser, test_at_token_uint32_get_einval)
{
	int ret;
	uint32_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	/* 1) Invalid input pointers. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_token_uint32_get(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_uint32_get(&tokens[1], NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_uint32_get(NULL, &val);
	zassert_equal(ret, -EINVAL);

	/* 2) The token is pointing to a null string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = NULL;

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 3) The token is pointing to an empty string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = "";

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 4) The length of the token is zero. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = 0;

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 5) The length of the token is larger than the length of the string it points to. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = strlen(tokens[1].start) + 1;

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 6) The token has an invalid type. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	/* Override type with anything other than `AT_TOKEN_TYPE_INT`. */
	for (int i = AT_TOKEN_TYPE_INVALID; i < AT_TOKEN_TYPE_RESP; i++) {
		if (i != AT_TOKEN_TYPE_INT) {
			tokens[1].type = i;
			ret = at_token_uint32_get(&tokens[1], &val);
			zassert_equal(ret, -EINVAL);
		}
	}
}

ZTEST(at_parser, test_at_token_uint32_get_erange)
{
	int ret;
	uint32_t val;
	struct at_token tokens[32] = { 0 };

	char str1[64] = {0};

	snprintf(str1, sizeof(str1), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)UINT32_MAX + 1);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_token_uint32_get)
{
	int ret;
	uint32_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)UINT32_MAX);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_uint32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, UINT32_MAX);
}

ZTEST(at_parser, test_at_token_int32_get_einval)
{
	int ret;
	int32_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	/* 1) Invalid input pointers. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_token_int32_get(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_int32_get(&tokens[1], NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_int32_get(NULL, &val);
	zassert_equal(ret, -EINVAL);

	/* 2) The token is pointing to a null string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = NULL;

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 3) The token is pointing to an empty string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = "";

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 4) The length of the token is zero. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = 0;

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 5) The length of the token is larger than the length of the string it points to. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = strlen(tokens[1].start) + 1;

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 6) The token has an invalid type. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	/* Override type with anything other than `AT_TOKEN_TYPE_INT`. */
	for (int i = AT_TOKEN_TYPE_INVALID; i < AT_TOKEN_TYPE_RESP; i++) {
		if (i != AT_TOKEN_TYPE_INT) {
			tokens[1].type = i;
			ret = at_token_int32_get(&tokens[1], &val);
			zassert_equal(ret, -EINVAL);
		}
	}
}

ZTEST(at_parser, test_at_token_int32_get_erange)
{
	int ret;
	int32_t val;
	struct at_token tokens[32] = { 0 };

	char str1[64] = {0};

	snprintf(str1, sizeof(str1), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)INT32_MAX + 1);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int32_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)INT32_MIN - 1);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int32_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_token_int32_get)
{
	int ret;
	int32_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)INT32_MAX);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, INT32_MAX);

	char str3[64] = {0};

	snprintf(str3, sizeof(str3), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)INT32_MIN);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int32_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, INT32_MIN);
}

ZTEST(at_parser, test_at_token_int64_get_einval)
{
	int ret;
	int64_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	/* 1) Invalid input pointers. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_token_int64_get(NULL, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_int64_get(&tokens[1], NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_token_int64_get(NULL, &val);
	zassert_equal(ret, -EINVAL);

	/* 2) The token is pointing to a null string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = NULL;

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 3) The token is pointing to an empty string. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].start = "";

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 4) The length of the token is zero. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = 0;

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 5) The length of the token is larger than the length of the string it points to. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	tokens[1].len = strlen(tokens[1].start) + 1;

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_equal(ret, -EINVAL);

	/* 6) The token has an invalid type. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	/* Override type with anything other than `AT_TOKEN_TYPE_INT`. */
	for (int i = AT_TOKEN_TYPE_INVALID; i < AT_TOKEN_TYPE_RESP; i++) {
		if (i != AT_TOKEN_TYPE_INT) {
			tokens[1].type = i;
			ret = at_token_int64_get(&tokens[1], &val);
			zassert_equal(ret, -EINVAL);
		}
	}
}

ZTEST(at_parser, test_at_token_int64_get_erange)
{
	int ret;
	int64_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 100000000000000000000,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int64_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);

	const char *str2 = "+NOTIF: -100000000000000000000,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int64_get(&tokens[1], &val);
	zassert_equal(ret, -ERANGE);
}

ZTEST(at_parser, test_at_token_int64_get)
{
	int ret;
	int64_t val;
	struct at_token tokens[32] = { 0 };

	const char *str1 = "+NOTIF: 1,-2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);

	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	char str2[64] = {0};

	snprintf(str2, sizeof(str2), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)INT64_MAX);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str2, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, INT64_MAX);

	char str3[64] = {0};

	snprintf(str3, sizeof(str3), "+NOTIF: %lld,-2,3\r\nOK\r\n", (long long int)INT64_MIN);

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str3, NULL);
	zassert_ok(ret);

	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	
	ret = at_token_int64_get(&tokens[1], &val);
	zassert_ok(ret);
	zassert_equal(val, INT64_MIN);
}

ZTEST(at_parser, test_at_token_string_get_enomem)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = { 0 };
	size_t len = sizeof(buffer);

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	len = 1;

	ret = at_token_string_get(&token, buffer, &len);
	zassert_equal(ret, -ENOMEM);
}

ZTEST(at_parser, test_at_token_string_get_einval)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	struct at_token mod_token = {0};
	char buffer[32] = { 0 };
	size_t len = sizeof(buffer);

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_token_string_get(NULL, buffer, &len);
	zassert_equal(ret, -EINVAL);

	ret = at_token_string_get(&token, NULL, &len);
	zassert_equal(ret, -EINVAL);

	ret = at_token_string_get(&token, buffer, NULL);
	zassert_equal(ret, -EINVAL);

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	memcpy(&mod_token, &token, sizeof(struct at_token));
	mod_token.type = AT_TOKEN_TYPE_INT;

	ret = at_token_string_get(&mod_token, buffer, &len);
	zassert_equal(ret, -EINVAL);

	memcpy(&mod_token, &token, sizeof(struct at_token));
	mod_token.start = NULL;

	ret = at_token_string_get(&mod_token, buffer, &len);
	zassert_equal(ret, -EINVAL);
}

ZTEST(at_parser, test_at_token_string_get)
{
	int ret;
	struct at_token tokens[32] = {0};

	const char *str1 = "+CGEV: ME PDN ACT 0";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 2);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_NOTIF);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_STRING);
}

ZTEST(at_parser, test_at_token_valid_count_get_einval)
{
	int ret;

	ret = at_token_valid_count_get(NULL, 42);
	zassert_equal(ret, -EINVAL);
}

ZTEST(at_parser, test_at_token_valid_count_get_invalid)
{
	int ret;
	int original_valid_count;
	struct at_token tokens[32] = {0};

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	/* 1) Type is invalid. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	original_valid_count = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(original_valid_count, 4);

	/* Take an invalid value, for example outside the explicit types.
	 * In this case we choose the maximum possible value for the member `type` of
	 * `struct at_token`.
	 */
	int type = 15;
	for (int i = AT_TOKEN_TYPE_INVALID; i < AT_TOKEN_TYPE_RESP; i++) {
		zassert_not_equal(type, i);
	}

	for (int i = 0; i < ARRAY_SIZE(tokens); i++) {
		/* Override the type */
		tokens[i].type = type;
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_not_equal(ret, original_valid_count);
	zassert_equal(ret, 0);

	/* 2) `start` pointer is invalid. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	original_valid_count = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(original_valid_count, 4);

	/* Override each token to have `start` point to an empty string. This is invalid because
	 * these tokens are expected to contain a non-empty string.
	 */
	for (int i = 0; i < ARRAY_SIZE(tokens); i++) {
		tokens[i].start = "";
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_not_equal(ret, original_valid_count);
	zassert_equal(ret, 0);

	/* 3) Token length is zero. */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	original_valid_count = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(original_valid_count, 4);

	/* Override each token to have length zero. This is invalid because these tokens are
	 * expected to have non-zero length.
	 */
	for (int i = 0; i < ARRAY_SIZE(tokens); i++) {
		tokens[i].len = 0;
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_not_equal(ret, original_valid_count);
	zassert_equal(ret, 0);

	/* 4) Token length is larger than the length of the remaining string pointed by the
	 * token.
	 */
	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	original_valid_count = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(original_valid_count, 4);

	/* Override each token to have length larger than the length of the string they point to. */
	for (int i = 0; i < ARRAY_SIZE(tokens); i++) {
		tokens[i].len = UINT16_MAX;
	}

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_not_equal(ret, original_valid_count);
	zassert_equal(ret, 0);
}

ZTEST(at_parser, test_at_token_valid_count_get)
{
	int ret;
	struct at_token tokens[32] = {0};

	const char *str1 = "+NOTIF: 1,2,3\r\nOK\r\n";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 4);
}

ZTEST(at_parser, test_at_cmd_type_get_invalid)
{
	enum at_token_type type;

	type = at_cmd_type_get(NULL);
	zassert_equal(type, AT_TOKEN_TYPE_INVALID);

	type = at_cmd_type_get("");
	zassert_equal(type, AT_TOKEN_TYPE_INVALID);

	const char *str1 = "ABBA";

	type = at_cmd_type_get(str1);
	zassert_equal(type, AT_TOKEN_TYPE_INVALID);

	/* Not a valid command (note: it's a notification). */
	const char *str2 = "+NOTIF: 1,2,3\r\nOK\r\n";

	type = at_cmd_type_get(str2);
	zassert_equal(type, AT_TOKEN_TYPE_INVALID);
}

ZTEST(at_parser, test_at_cmd_type_get_valid)
{
	enum at_token_type type;

	const char *str1 = "AT+CMD=?";

	type = at_cmd_type_get(str1);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_TEST);

	const char *str2 = "AT+CMD?";

	type = at_cmd_type_get(str2);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_READ);

	const char *str3 = "AT+CMD=1,2,3";

	type = at_cmd_type_get(str3);
	zassert_equal(type, AT_TOKEN_TYPE_CMD_SET);
}

ZTEST(at_parser, test_at_token_type_empty_cmd_set)
{
	int ret;
	struct at_token tokens[32] = {0};

	const char *str1 = "AT+CMD=1,2,3,,";

	ret = at_token_line_parse(tokens, ARRAY_SIZE(tokens), str1, NULL);
	zassert_ok(ret);

	ret = at_token_valid_count_get(tokens, ARRAY_SIZE(tokens));
	zassert_equal(ret, 6);
	zassert_equal(tokens[0].type, AT_TOKEN_TYPE_CMD_SET);
	zassert_equal(tokens[1].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[2].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[3].type, AT_TOKEN_TYPE_INT);
	zassert_equal(tokens[4].type, AT_TOKEN_TYPE_EMPTY);
	zassert_equal(tokens[5].type, AT_TOKEN_TYPE_EMPTY);
}

ZTEST(at_parser, test_cgmi)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);

	const char *cmd = "AT+CGMI";

	ret = at_parser_init(&parser, cmd);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+CGMI"), buffer_size);
	zassert_mem_equal("AT+CGMI", buffer, buffer_size);

	const char *resp = "Nordic Semiconductor ASA\r\nOK\r\n";

	ret = at_parser_init(&parser, resp);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("Nordic Semiconductor ASA"), buffer_size);
	zassert_mem_equal("Nordic Semiconductor ASA", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_cgmm)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);

	const char *cmd = "AT+CGMM";

	ret = at_parser_init(&parser, cmd);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+CGMM"), buffer_size);
	zassert_mem_equal("AT+CGMM", buffer, buffer_size);

	const char *resp = "nRF9160-SICA\r\nOK\r\n";

	ret = at_parser_init(&parser, resp);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("nRF9160-SICA"), buffer_size);
	zassert_mem_equal("nRF9160-SICA", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_cgmr)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);

	const char *cmd = "AT+CGMR";

	ret = at_parser_init(&parser, cmd);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+CGMR"), buffer_size);
	zassert_mem_equal("AT+CGMR", buffer, buffer_size);

	const char *resp = "mfw_nrf9160_1.1.1\r\nOK\r\n";

	ret = at_parser_init(&parser, resp);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("mfw_nrf9160_1.1.1"), buffer_size);
	zassert_mem_equal("mfw_nrf9160_1.1.1", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_cgsn)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);
	int32_t val = 0;

	const char *cmd_1 = "AT+CGSN";

	ret = at_parser_init(&parser, cmd_1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+CGSN"), buffer_size);
	zassert_mem_equal("AT+CGSN", buffer, buffer_size);

	const char *resp_1 = "352656100367872\r\nOK\r\n";

	ret = at_parser_init(&parser, resp_1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret, "%d", ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("352656100367872"), buffer_size);
	zassert_mem_equal("352656100367872", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);

	const char *cmd_2 = "AT+CGSN=1";

	ret = at_parser_init(&parser, cmd_2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+CGSN"), buffer_size);
	zassert_mem_equal("AT+CGSN", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	const char *resp_2 = "+CGSN: \"352656100367872\"\r\nOK\r\n";

	ret = at_parser_init(&parser, resp_2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("+CGSN"), buffer_size);
	zassert_mem_equal("+CGSN", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("352656100367872"), buffer_size);
	zassert_mem_equal("352656100367872", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);

	const char *cmd_3 = "AT+CGSN=?";

	ret = at_parser_init(&parser, cmd_3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_TEST);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+CGSN"), buffer_size);
	zassert_mem_equal("AT+CGSN", buffer, buffer_size);

	const char *resp_3 = "+CGSN: (0-3)\r\nOK\r\n";

	ret = at_parser_init(&parser, resp_3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("+CGSN"), buffer_size);
	zassert_mem_equal("+CGSN", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("0-3"), buffer_size);
	zassert_mem_equal("0-3", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_shortswver)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);

	const char *cmd = "AT%SHORTSWVER";

	ret = at_parser_init(&parser, cmd);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT%SHORTSWVER"), buffer_size);
	zassert_mem_equal("AT%SHORTSWVER", buffer, buffer_size);

	const char *resp = "%SHORTSWVER: nrf9160_1.1.2\r\nOK\r\n";

	ret = at_parser_init(&parser, resp);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("%SHORTSWVER"), buffer_size);
	zassert_mem_equal("%SHORTSWVER", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("nrf9160_1.1.2"), buffer_size);
	zassert_mem_equal("nrf9160_1.1.2", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_hwversion)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);

	const char *cmd = "AT%HWVERSION";

	ret = at_parser_init(&parser, cmd);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT%HWVERSION"), buffer_size);
	zassert_mem_equal("AT%HWVERSION", buffer, buffer_size);

	const char *resp = "%HWVERSION: nRF9160 SICA B0A\r\nOK\r\n";

	ret = at_parser_init(&parser, resp);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("%HWVERSION"), buffer_size);
	zassert_mem_equal("%HWVERSION", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("nRF9160 SICA B0A"), buffer_size);
	zassert_mem_equal("nRF9160 SICA B0A", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_xmodemuuid)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[64] = {0};
	size_t buffer_size = sizeof(buffer);

	const char *cmd = "AT%XMODEMUUID";

	ret = at_parser_init(&parser, cmd);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT%XMODEMUUID"), buffer_size);
	zassert_mem_equal("AT%XMODEMUUID", buffer, buffer_size);

	const char *resp = "%XMODEMUUID: 25c95751-efa4-40d4-8b4a-1dcaab81fac9\r\nOK\r\n";

	ret = at_parser_init(&parser, resp);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("%XMODEMUUID"), buffer_size);
	zassert_mem_equal("%XMODEMUUID", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("25c95751-efa4-40d4-8b4a-1dcaab81fac9"), buffer_size);
	zassert_mem_equal("25c95751-efa4-40d4-8b4a-1dcaab81fac9", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_odis)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);

	/* 1) Set command. */
	const char *cmd1 = "AT+ODIS=\"HDID01\",\"HDMAN01\",\"HDMOD01\",\"HDSW01\"";

	ret = at_parser_init(&parser, cmd1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+ODIS"), buffer_size);
	zassert_mem_equal("AT+ODIS", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDID01"), buffer_size);
	zassert_mem_equal("HDID01", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDMAN01"), buffer_size);
	zassert_mem_equal("HDMAN01", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDMOD01"), buffer_size);
	zassert_mem_equal("HDMOD01", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDSW01"), buffer_size);
	zassert_mem_equal("HDSW01", buffer, buffer_size);

	const char *resp1 = "\r\nOK\r\n";

	ret = at_parser_init(&parser, resp1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);

	/* 2) Read command. */
	const char *cmd2 = "AT+ODIS?";

	ret = at_parser_init(&parser, cmd2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_READ);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT+ODIS"), buffer_size);
	zassert_mem_equal("AT+ODIS", buffer, buffer_size);

	const char *resp2 = "+ODIS: \"HDMAN01\",\"HDMOD01\",\"HDSW01\"\r\nOK\r\n";

	ret = at_parser_init(&parser, resp2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("+ODIS"), buffer_size);
	zassert_mem_equal("+ODIS", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDMAN01"), buffer_size);
	zassert_mem_equal("HDMAN01", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDMOD01"), buffer_size);
	zassert_mem_equal("HDMOD01", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("HDSW01"), buffer_size);
	zassert_mem_equal("HDSW01", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_cind)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};

	/* 3) Test command. */
	const char *resp3 = "+CIND: (\"service\",(0,1)),(\"roam\",(0,1)),(\"message\",(0,1))\r\n"
			    "OK\r\n";
	
	ret = at_parser_init(&parser, resp3);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);
}

ZTEST(at_parser, test_xbandlock)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[128] = {0};
	size_t buffer_size = sizeof(buffer);

	/* 2) Read command. */
	const char *resp2 = "%XBANDLOCK: \"\",\"000000000000000000000000000000000000000000000000000"
			    "0000000000000000000000001000000001001\"\r\nOK\r\n";

	ret = at_parser_init(&parser, resp2);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("%XBANDLOCK"), buffer_size);
	zassert_mem_equal("%XBANDLOCK", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen(""), buffer_size, "%d", buffer_size);
	zassert_mem_equal("", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_QUOTED_STRING);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret, "%d", ret);
	zassert_equal(strlen("000000000000000000000000000000000000000000000000000000000000000000000"
		      "0000001000000001001"), buffer_size);
	zassert_mem_equal("000000000000000000000000000000000000000000000000000000000000000000000000"
			  "0001000000001001", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\nOK\r\n"), buffer_size);
	zassert_mem_equal("\r\nOK\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_xt3412)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};
	char buffer[32] = {0};
	size_t buffer_size = sizeof(buffer);
	int32_t val = 0;

	/* 1) Set command. */
	const char *cmd1 = "AT%XT3412=1,2000,30000";

	ret = at_parser_init(&parser, cmd1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_CMD_SET);

	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("AT%XT3412"), buffer_size);
	zassert_mem_equal("AT%XT3412", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 2000);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 30000);

	const char *resp1 = "%XT3412: 1200000\r\n";

	ret = at_parser_init(&parser, resp1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("%XT3412"), buffer_size);
	zassert_mem_equal("%XT3412", buffer, buffer_size);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_INT);

	ret = at_token_int32_get(&token, &val);
	zassert_ok(ret);
	zassert_equal(val, 1200000);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);

	buffer_size = sizeof(buffer);
	ret = at_token_string_get(&token, buffer, &buffer_size);
	zassert_ok(ret);
	zassert_equal(strlen("\r\n"), buffer_size);
	zassert_mem_equal("\r\n", buffer, buffer_size);
}

ZTEST(at_parser, test_array)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};

	const char *str1 = "+NOTIF: ((\"\"),1)\r\nOK\r\n";

	ret = at_parser_init(&parser, str1);
	zassert_ok(ret);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_NOTIF);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_ARRAY);

	ret = at_parser_tok(&parser, &token);
	zassert_ok(ret);
	zassert_equal(token.type, AT_TOKEN_TYPE_RESP);
}

ZTEST_SUITE(at_parser, NULL, NULL, test_params_before, test_params_after, NULL);
