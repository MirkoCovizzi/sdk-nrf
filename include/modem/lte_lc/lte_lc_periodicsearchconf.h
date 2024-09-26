#ifndef LTE_LC_PERIODICSEARCHCONF_H__
#define LTE_LC_PERIODICSEARCHCONF_H__

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Search pattern type. */
enum lte_lc_periodic_search_pattern_type {
	/** Range search pattern. */
	LTE_LC_PERIODIC_SEARCH_PATTERN_RANGE = 0,

	/** Table search pattern. */
	LTE_LC_PERIODIC_SEARCH_PATTERN_TABLE = 1,
};

/** Configuration for periodic search of type @ref LTE_LC_PERIODIC_SEARCH_PATTERN_RANGE. */
struct lte_lc_periodic_search_range_cfg {
	/**
	 * Sleep time between searches in the beginning of the range.
	 *
	 * Allowed values: 0 - 65535 seconds.
	 */
	uint16_t initial_sleep;

	/**
	 * Sleep time between searches in the end of the range.
	 *
	 * Allowed values: 0 - 65535 seconds.
	 */
	uint16_t final_sleep;

	/**
	 * Optional target time in minutes for achieving the @c final_sleep value.
	 *
	 * This can be used to determine angle factor between the initial and final sleep times.
	 * The timeline for the @c time_to_final_sleep starts from the beginning of the search
	 * pattern. If given, the value cannot be greater than the value of the
	 * @c pattern_end_point  value in the same search pattern. If not given, the angle factor
	 * is calculated by using the @c pattern_end_point value so, that the @c final_sleep
	 * value is reached at the point of @c pattern_end_point.
	 *
	 * Allowed values:
	 * * -1: Not used
	 * * 0 - 1080 minutes
	 */
	int16_t time_to_final_sleep;

	/**
	 * Time that must elapse before entering the next search pattern.
	 *
	 * The timeline for @c pattern_end_point starts from the beginning of the limited service
	 * starting point, which is the moment when the first sleep period started.
	 *
	 * Allowed values:
	 * * 0 - 1080 minutes.
	 */
	int16_t pattern_end_point;
};

/**
 * Configuration for periodic search of type @ref LTE_LC_PERIODIC_SEARCH_PATTERN_TABLE.
 *
 * 1 to 5 sleep time values for sleep between searches can be configured. It is mandatory to provide
 * @c val_1, while the rest are optional. Unused values must be set to -1. After going through all
 * values, the last value of the last search pattern is repeated, if not configured differently by
 * the @c loop or @c return_to_pattern parameters.
 *
 * Allowed values:
 * * -1: Value unused.
 * * 0 - 65535 seconds.
 */
struct lte_lc_periodic_search_table_cfg {
	/** Mandatory sleep time. */
	int val_1;

	/**
	 * Optional sleep time.
	 *
	 * Must be set to -1 if not used.
	 */
	int val_2;

	/**
	 * Optional sleep time.
	 *
	 * @c val_2 must be configured for this parameter to take effect.
	 *
	 *  Must be set to -1 if not used.
	 */
	int val_3;

	/**
	 * Optional sleep time.
	 *
	 * @c val_3 must be configured for this parameter to take effect.
	 *
	 * Must be set to -1 if not used.
	 */
	int val_4;

	/**
	 * Optional sleep time.
	 *
	 * @c val_4 must be configured for this parameter to take effect.
	 *
	 * Must be set to -1 if not used.
	 */
	int val_5;
};

/**
 * Periodic search pattern.
 *
 * A search pattern may be of either 'range' or 'table' type.
 */
struct lte_lc_periodic_search_pattern {
	/** Search pattern type. */
	enum lte_lc_periodic_search_pattern_type type;

	union {
		/** Configuration for periodic search of type 'range'. */
		struct lte_lc_periodic_search_range_cfg range;

		/** Configuration for periodic search of type 'table'. */
		struct lte_lc_periodic_search_table_cfg table;
	};
};

/** Periodic search configuration. */
struct lte_lc_periodic_search_cfg {
	/**
	 * Indicates if the last given pattern is looped from the beginning when the pattern has
	 * ended.
	 *
	 * If several patterns are configured, this impacts only the last pattern.
	 */
	bool loop;

	/**
	 * Indicates if the modem can return to a given search pattern with shorter sleeps, for
	 * example, when radio conditions change and the given pattern index has already been
	 * exceeded.
	 *
	 * Allowed values:
	 * * 0: No return pattern.
	 * * 1 - 4: Return to search pattern index 1..4.
	 */
	uint16_t return_to_pattern;

	/**
	 * Indicates if band optimization shall be used.
	 *
	 * * 0: No optimization. Every periodic search shall be all band search.
	 * * 1: Use default optimizations predefined by modem. Predefinition depends on
	 *      the active data profile.
	 *      See "nRF91 AT Commands - Command Reference Guide" for more information.
	 * * 2 - 20: Every n periodic search must be an all band search.
	 */
	uint16_t band_optimization;

	/** The number of valid patterns. Range 1 - 4. */
	size_t pattern_count;

	/** Array of periodic search patterns. */
	struct lte_lc_periodic_search_pattern patterns[4];
};

/**
 * Configure periodic searches.
 *
 * This configuration affects the periodic searches that the modem performs in limited service state
 * to obtain normal service. See @ref lte_lc_periodic_search_cfg and
 * "nRF91 AT Commands - Command Reference Guide" for more information and in-depth explanations of
 * periodic search configuration.
 *
 * @param[in] cfg Periodic search configuration.
 *
 * @retval 0 if the configuration was successfully sent to the modem.
 * @retval -EINVAL if an input parameter was @c NULL or contained an invalid pattern count.
 * @retval -EFAULT if an AT command could not be sent to the modem.
 * @retval -EBADMSG if the modem responded with an error to an AT command.
 */
int lte_lc_periodic_search_set(const struct lte_lc_periodic_search_cfg *const cfg);

/**
 * Get the configured periodic search parameters.
 *
 * @param[out] cfg Periodic search configuration.
 *
 * @retval 0 if a configuration was found and populated to the provided pointer.
 * @retval -EINVAL if input parameter was @c NULL.
 * @retval -ENOENT if periodic search configuration was not set.
 * @retval -EFAULT if an AT command failed.
 * @retval -EBADMSG if the modem responded with an error to an AT command or the
 *         response could not be parsed.
 */
int lte_lc_periodic_search_get(struct lte_lc_periodic_search_cfg *const cfg);

/**
 * Clear the configured periodic search parameters.
 *
 * @retval 0 if the configuration was cleared.
 * @retval -EFAULT if an AT command could not be sent to the modem.
 * @retval -EBADMSG if the modem responded with an error to an AT command.
 */
int lte_lc_periodic_search_clear(void);

/**
 * Request an extra search.
 *
 * This can be used for example when modem is in sleep state between periodic searches. The search
 * is performed only when the modem is in sleep state between periodic searches.
 *
 * @retval 0 if the search request was successfully delivered to the modem.
 * @retval -EFAULT if an AT command could not be sent to the modem.
 */
int lte_lc_periodic_search_request(void);

#ifdef __cplusplus
}
#endif

#endif
