/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <modem/at_parser.h>

#include "at_match.h"

enum num_type {
	I16,
	U16,
	I32,
	U32,
	I64,
};

static bool is_subparam(const struct at_token *token)
{
	switch (token->type) {
	case AT_TOKEN_TYPE_INT:
	case AT_TOKEN_TYPE_QUOTED_STRING:
	case AT_TOKEN_TYPE_ARRAY:
	case AT_TOKEN_TYPE_EMPTY:
		return true;
	default:
		return false;
	}
}

static void update_at_parser_counters(struct at_parser *parser, const struct at_token *token)
{
	/* Beginning of new notification line.
	 * Example: "+NOTIF: 1\r\n+NOTIF2: ..."
	 * In the example above, the parser has parsed "+NOTIF2" last. This is the beginning of a
	 * new line, so reset the counters. */
	if (parser->counters.notif_count == 2) {
		memset(&parser->counters, 0, sizeof(parser->counters));
		parser->counters.notif_count = 1;
	}

	switch (token->type) {
	case AT_TOKEN_TYPE_CMD_TEST:
	case AT_TOKEN_TYPE_CMD_READ:
	case AT_TOKEN_TYPE_CMD_SET:
		parser->counters.cmd_count++;
		break;
	case AT_TOKEN_TYPE_NOTIF:
		parser->counters.notif_count++;
		break;
	case AT_TOKEN_TYPE_STRING:
		parser->counters.string_count++;
		break;
	case AT_TOKEN_TYPE_RESP:
		parser->counters.end_count++;
		break;
	default:
		parser->counters.subparam_count++;
	}
}

static bool is_valid_at_token(const struct at_token *token)
{
	if (!token->start) {
		return false;
	}

	size_t len = strlen(token->start);

	switch (token->type) {
	case AT_TOKEN_TYPE_CMD_TEST:
	case AT_TOKEN_TYPE_CMD_READ:
	case AT_TOKEN_TYPE_CMD_SET:
	case AT_TOKEN_TYPE_NOTIF:
	case AT_TOKEN_TYPE_INT:
	case AT_TOKEN_TYPE_ARRAY:
	case AT_TOKEN_TYPE_STRING:
	case AT_TOKEN_TYPE_RESP:
		return len != 0 && token->len != 0 && token->len <= len;
	case AT_TOKEN_TYPE_QUOTED_STRING:
		return len != 0 && token->len <= len;
	case AT_TOKEN_TYPE_EMPTY:
		return token->len == 0;
	default:
		return false;
	}
}

static bool is_valid_at_token_line(struct at_parser *parser)
{
	/* At most one command per line. */
	if (parser->counters.cmd_count > 1) {
		return false;
	}

	/* There must be at least one of either command or notification if there is more than
	 * one subparameter.
	 */
	if (parser->counters.subparam_count >= 1 && parser->counters.cmd_count < 1 &&
	    parser->counters.notif_count < 1) {
		return false;
	}

	/* If there is more than one notification, then there must be at least one subparameter
	 * (belonging to the first notification line).
	 */
	if (parser->counters.notif_count > 1 && parser->counters.subparam_count < 1) {
		return false;
	}

	return true;
}

int at_parser_init(struct at_parser *parser, const char *at)
{
	if (!parser || !at) {
		return -EINVAL;
	}

	size_t len = strlen(at);

	/* Check bounds. Set maximum limit to a reasonable amount of 65535 characters (64KB). */
	if (len == 0 || len > UINT16_MAX) {
		return -ERANGE;
	}

	memset(parser, 0, sizeof(struct at_parser));

	parser->ptr = at;
	parser->initialized = true;

	return 0;
}

int at_parser_tok(struct at_parser *parser, struct at_token *token)
{
	const char *remainder = NULL;
	struct at_token tok = {0};

	if (!parser || !token) {
		return -EINVAL;
	}

	if (!parser->initialized) {
		return -EPERM;
	}

	if (!parser->ptr) {
		return -EINVAL;
	}

	/* The lexer cannot match empty strings, so intercept the special case where the empty
	 * subparameter is the one after the previous token. This case is detected in the previous
	 * call to this function.
	 */
	if (parser->is_next_empty) {
		tok.start = parser->ptr;
		tok.len = 0;
		tok.type = AT_TOKEN_TYPE_EMPTY;
		tok.variant = AT_TOKEN_VARIANT_NO_COMMA;

		/* Set the remainder to the current cursor because an empty token has length
		 * zero.
		 */
		remainder = parser->ptr;

		goto matched;
	}

	/* Nothing left to tokenize. */
	if (strlen(parser->ptr) == 0) {
		return -EIO;
	}

	tok = at_match(parser->ptr, &remainder);
	if (tok.type == AT_TOKEN_TYPE_INVALID) {
		return -EBADMSG;
	}

matched:

	/* The lexer cannot differentiate between an integer and a non-quoted string containing
	 * purely numbers because they are syntactically equivalent.
	 * If the first token of the line is detected as an integer and does not have a trailing
	 * comma, then it is only valid as a non-quoted string because a subparameter as the first
	 * token of the line makes the line invalid.
	 */
	if (tok.type == AT_TOKEN_TYPE_INT && tok.variant == AT_TOKEN_VARIANT_NO_COMMA &&
	    parser->count == 0) {
		tok.type = AT_TOKEN_TYPE_STRING;
	}

	/* Update the parser's internal counters and ensure that the line is consistent. */
	update_at_parser_counters(parser, &tok);
	if (!is_valid_at_token_line(parser)) {
		return -EBADMSG;
	}

	/* Reset this flag at each call. */
	parser->is_next_empty = false;
	if (is_subparam(&tok)) {
		size_t rem_len = strlen(remainder);
		bool lookahead_crlf = rem_len >= 2 && remainder[0] == '\r' && remainder[1] == '\n';

		/* if the token is a subparameter but does not have a trailing comma then it is the
		 * last subparameter of the line.
		 * If there is more to parse, then the token must be followed by CRLF, otherwise the
		 * string is malformed.
		 */
		if (tok.variant != AT_TOKEN_VARIANT_COMMA) {
			if (rem_len > 0 && !lookahead_crlf) {
				return -EBADMSG;
			}
		} else {
			/* The next token is of type empty if and only if the current token has a
			 * trailing comma and the remaining string either is empty or starts with
			 * CRLF.
			 */
			if (rem_len == 0 || lookahead_crlf) {
				parser->is_next_empty = true;
			}
		}
	}

	parser->count++;
	parser->ptr = remainder;
	parser->prev_token = tok;

	memcpy(token, &tok, sizeof(struct at_token));

	return 0;
}

int at_parser_seek(struct at_parser *parser, size_t index, struct at_token *token)
{
	int ret;
	struct at_token tok = {0};

	if (!parser || !token) {
		return -EINVAL;
	}

	/* Prevent from going backwards. */
	if (index + 1 <= parser->count) {
		return -ERANGE;
	}

	do {
		ret = at_parser_tok(parser, &tok);
	} while ((index + 1 > parser->count) && (ret == 0));

	if (ret == 0) {
		memcpy(token, &tok, sizeof(struct at_token));
	}

	return ret;
}

int at_token_line_parse(struct at_token *tokens, size_t num_tokens, const char *at,
			const char **next_at)
{
	int ret;
	struct at_parser parser = {0};
	struct at_token token = {0};

	if (!tokens || !at) {
		return -EINVAL;
	}

	ret = at_parser_init(&parser, at);
	if (ret < 0) {
		return ret;
	}

	memset(tokens, 0, num_tokens * sizeof(struct at_token));

	while (parser.count < num_tokens) {
		ret = at_parser_tok(&parser, &token);
		/* When reading a line, it is acceptable to parse until the null terminator even
		 * if there is no AT response.
		 */
		if (ret == -EIO) {
			return 0;
		}
		if (ret < 0) {
			return ret;
		}

		/* Beginning of the next line. Inform the user that there is a new line to parse. */
		if (token.type == AT_TOKEN_TYPE_NOTIF && parser.counters.notif_count == 2) {
			return -EAGAIN;
		}

		if (token.type == AT_TOKEN_TYPE_RESP) {
			/* Null-terminate the remainder. There is nothing left to parse. */
			if (next_at) {
				*next_at = "\0";
			}

			return 0;
		}

		if (next_at) {
			*next_at = parser.ptr;
		}

		memcpy(&tokens[parser.count - 1], &token, sizeof(struct at_token));
	}

	return 0;
}

static int at_token_num_get(const struct at_token *token, void *value, enum num_type t, int64_t max,
			    int64_t min)
{
	if (!value || !token) {
		return -EINVAL;
	}

	if (token->type != AT_TOKEN_TYPE_INT) {
		return -EINVAL;
	}

	if (!is_valid_at_token(token)) {
		return -EINVAL;
	}

	char *next = NULL;
	int64_t val = strtoll(token->start, &next, 10);

	/* `strtoll` returns value out of range. */
	if ((val == INT64_MIN || val == INT64_MAX) && errno == ERANGE) {
		return -ERANGE;
	}

	/* Value out of range according to the expected type. */
	if ((val > max) || (val < min)) {
		return -ERANGE;
	}

	switch (t) {
	case I16:
		*(int16_t *)(value) = (int16_t)val;
		break;
	case U16:
		*(uint16_t *)(value) = (uint16_t)val;
		break;
	case I32:
		*(int32_t *)(value) = (int32_t)val;
		break;
	case U32:
		*(uint32_t *)(value) = (uint32_t)val;
		break;
	case I64:
		*(int64_t *)(value) = (int64_t)val;
		break;
	default:
		/* Unreachable. */
	}

	return 0;
}

int at_token_int16_get(const struct at_token *token, int16_t *value)
{
	return at_token_num_get(token, (void *)value, I16, INT16_MAX, INT16_MIN);
}

int at_token_uint16_get(const struct at_token *token, uint16_t *value)
{
	return at_token_num_get(token, (void *)value, U16, UINT16_MAX, 0);
}

int at_token_int32_get(const struct at_token *token, int32_t *value)
{
	return at_token_num_get(token, (void *)value, I32, INT32_MAX, INT32_MIN);
}

int at_token_uint32_get(const struct at_token *token, uint32_t *value)
{
	return at_token_num_get(token, (void *)value, U32, UINT32_MAX, 0);
}

int at_token_int64_get(const struct at_token *token, int64_t *value)
{
	return at_token_num_get(token, (void *)value, I64, INT64_MAX, INT64_MIN);
}

int at_token_string_get(const struct at_token *token, char *str, size_t *len)
{
	if (!token || !str || !len) {
		return -EINVAL;
	}

	switch (token->type) {
	/* Acceptable types. */
	case AT_TOKEN_TYPE_CMD_TEST:
	case AT_TOKEN_TYPE_CMD_SET:
	case AT_TOKEN_TYPE_CMD_READ:
	case AT_TOKEN_TYPE_NOTIF:
	case AT_TOKEN_TYPE_QUOTED_STRING:
	case AT_TOKEN_TYPE_STRING:
	case AT_TOKEN_TYPE_ARRAY:
	case AT_TOKEN_TYPE_RESP:
		break;
	default:
		return -EINVAL;
	}

	if (!is_valid_at_token(token)) {
		return -EINVAL;
	}

	/* Check if there is enough memory for null terminator. */
	if (*len < token->len + 1) {
		return -ENOMEM;
	}

	memcpy(str, token->start, token->len);

	/* Null-terminate the output string. */
	str[token->len] = '\0';

	/* Update the length to reflect the copied string length. */
	*len = token->len;

	return 0;
}

size_t at_token_valid_count_get(const struct at_token *tokens, size_t num_tokens)
{
	size_t count = 0;

	if (!tokens) {
		return -EINVAL;
	}

	for (size_t i = 0; i < num_tokens; i++) {
		if (is_valid_at_token(&tokens[i])) {
			count++;
		}
	}

	return count;
}

enum at_token_type at_cmd_type_get(const char *at)
{
	struct at_token tok;

	if (!at) {
		return AT_TOKEN_TYPE_INVALID;
	}

	tok = at_match(at, NULL);
	switch (tok.type) {
	case AT_TOKEN_TYPE_CMD_TEST:
	case AT_TOKEN_TYPE_CMD_READ:
	case AT_TOKEN_TYPE_CMD_SET:
		return tok.type;
	default:
		return AT_TOKEN_TYPE_INVALID;
	}
}
