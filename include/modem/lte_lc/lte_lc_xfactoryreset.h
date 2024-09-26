#ifndef LTE_LC_XFACTORYRESET_H__
#define LTE_LC_XFACTORYRESET_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Type of factory reset to perform. */
enum lte_lc_factory_reset_type {
	/** Reset all modem data to factory settings. */
	LTE_LC_FACTORY_RESET_ALL = 0,

	/** Reset user-configurable data to factory settings. */
	LTE_LC_FACTORY_RESET_USER = 1,
};

/**
 * Reset modem to factory settings.
 *
 * This operation is allowed only when the modem is not activated.
 *
 * @note This feature is only supported by modem firmware versions >= 1.3.0.
 *
 * @param[in] type Factory reset type.
 *
 * @retval 0 if factory reset was performed successfully.
 * @retval -EFAULT if an AT command failed.
 */
int lte_lc_factory_reset(enum lte_lc_factory_reset_type type);

#ifdef __cplusplus
}
#endif

#endif
