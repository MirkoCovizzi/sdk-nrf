#ifndef LTE_LC_REDMOB_H__
#define LTE_LC_REDMOB_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Reduced mobility mode. */
enum lte_lc_reduced_mobility_mode {
	/** Functionality according to the 3GPP relaxed monitoring feature. */
	LTE_LC_REDUCED_MOBILITY_DEFAULT = 0,

	/** Enable Nordic-proprietary reduced mobility feature. */
	LTE_LC_REDUCED_MOBILITY_NORDIC = 1,

	/**
	 * Full measurements for best possible mobility.
	 *
	 * Disable the 3GPP relaxed monitoring and Nordic-proprietary reduced mobility features.
	 */
	LTE_LC_REDUCED_MOBILITY_DISABLED = 2,
};

/**
 * Read the current reduced mobility mode.
 *
 * @note This feature is only supported by modem firmware versions >= 1.3.2.
 *
 * @param[out] mode Reduced mobility mode.
 *
 * @retval 0 if a mode was found and written to the provided pointer.
 * @retval -EINVAL if input parameter was @c NULL.
 * @retval -EFAULT if an AT command failed.
 */
int lte_lc_reduced_mobility_get(enum lte_lc_reduced_mobility_mode *mode);

/**
 * Set reduced mobility mode.
 *
 * @note This feature is only supported by modem firmware versions >= 1.3.2.
 *
 * @param[in] mode Reduced mobility mode.
 *
 * @retval 0 if the new reduced mobility mode was accepted by the modem.
 * @retval -EFAULT if an AT command failed.
 */
int lte_lc_reduced_mobility_set(enum lte_lc_reduced_mobility_mode mode);

#ifdef __cplusplus
}
#endif

#endif
