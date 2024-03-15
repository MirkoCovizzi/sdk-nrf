/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef AT_PARSER_H__
#define AT_PARSER_H__

#include <stdbool.h>
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum at_token_type {
	AT_TOKEN_TYPE_INVALID = 0,
	AT_TOKEN_TYPE_CMD_TEST,
	AT_TOKEN_TYPE_CMD_READ,
	AT_TOKEN_TYPE_CMD_SET,
	AT_TOKEN_TYPE_NOTIF,
	AT_TOKEN_TYPE_INT,
	AT_TOKEN_TYPE_QUOTED_STRING,
	AT_TOKEN_TYPE_ARRAY,
	AT_TOKEN_TYPE_EMPTY,
	AT_TOKEN_TYPE_STRING,
	AT_TOKEN_TYPE_RESP
};

enum at_token_variant {
	AT_TOKEN_VARIANT_NO_COMMA = 0,
	AT_TOKEN_VARIANT_COMMA
};

struct at_token {
	const char *start;
	uint16_t len;
	uint8_t type :4;
	uint8_t variant :1;
} __attribute__ ((__packed__));

struct at_parser {
	const char *ptr;
	size_t count;
	struct {
		size_t cmd_count;
		size_t notif_count;
		size_t subparam_count;
		size_t string_count;
		size_t end_count;
	} counters;
	struct at_token prev_token;
	bool is_next_empty;
	bool initialized;
};

/**
 * @brief Initialize an AT parser.
 *
 * This function initializes an AT parser with the given AT command string.
 *
 * @param parser Pointer to the @ref at_parser structure to be initialized.
 * @param at     Pointer to a null-terminated string containing the AT command string to be
 *               parsed.
 *
 * @retval 0 If the operation was successful.
 * @retval -EINVAL @p parser or @p at are NULL.
 */
int at_parser_init(struct at_parser *parser, const char *at);

/**
 * @brief Parse the next token from the AT command string.
 *
 * This function parses the next token from the AT command string using the parser
 * state.
 *
 * @param parser Pointer to the @ref at_parser structure that holds the state of the parser.
 * @param token  Pointer to an @ref at_token structure where the parsed token will be stored.
 *
 * @retval 0 If the operation was successful.
 * @retval -EINVAL  @p parser or @p token are NULL or the parser state is invalid,
 * @retval -EPERM   @p parser has not been initialized,
 * @retval -EIO     There is nothing left to parse in the AT command string,
 * @retval -EBADMSG The token parsed is invalid.
 */
int at_parser_tok(struct at_parser *parser, struct at_token *token);

/**
 * @brief Seek to the specified token index in the AT command string.
 *
 * This function parses the next token from the AT command string using the parser
 * state.
 *
 * @param parser Pointer to the @ref at_parser structure that holds the state of the parser.
 * @param index  Zero-based index of the token to seek to within the AT command string.
 * @param token  Pointer to an @ref at_token structure where the parsed token will be stored.
 *
 * @retval 0 If the operation was successful.
 * @retval -EINVAL  @p parser or @p token are NULL or the parser state is invalid,
 * @retval -EPERM   @p parser has not been initialized,
 * @retval -EIO     There is nothing left to parse in the AT command string,
 * @retval -EBADMSG The token parsed is invalid.
 */
int at_parser_seek(struct at_parser *parser, size_t index, struct at_token *token);
int at_token_line_parse(struct at_token *tokens, size_t num_tokens, const char *at,
			const char **next_at);
int at_token_uint16_get(const struct at_token *token, uint16_t *value);
int at_token_int16_get(const struct at_token *token, int16_t *value);
int at_token_uint32_get(const struct at_token *token, uint32_t *value);
int at_token_int32_get(const struct at_token *token, int32_t *value);
int at_token_int64_get(const struct at_token *token, int64_t *value);
int at_token_string_get(const struct at_token *token, char *value, size_t *len);
size_t at_token_valid_count_get(const struct at_token *tokens, size_t num_tokens);
enum at_token_type at_cmd_type_get(const char *at);

#ifdef __cplusplus
}
#endif

#endif /* AT_PARSER_H__ */
