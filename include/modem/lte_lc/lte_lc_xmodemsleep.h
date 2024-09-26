#ifndef LTE_LC_XMODEMSLEEP_H__
#define LTE_LC_XMODEMSLEEP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Modem sleep type. */
enum lte_lc_modem_sleep_type {
	/** Power Saving Mode (PSM). */
	LTE_LC_MODEM_SLEEP_PSM			= 1,

	/** RF inactivity, for example eDRX. */
	LTE_LC_MODEM_SLEEP_RF_INACTIVITY	= 2,

	/** Limited service or out of coverage. */
	LTE_LC_MODEM_SLEEP_LIMITED_SERVICE	= 3,

	/** Flight mode. */
	LTE_LC_MODEM_SLEEP_FLIGHT_MODE		= 4,

	/**
	 * Proprietary PSM.
	 *
	 * @note This type is only supported by modem firmware versions >= 2.0.0.
	 */
	LTE_LC_MODEM_SLEEP_PROPRIETARY_PSM	= 7,
};

/** Modem sleep information. */
struct lte_lc_modem_sleep {
	/** Sleep type. */
	enum lte_lc_modem_sleep_type type;

	/**
	 * Sleep time in milliseconds. If this value is set to -1, the sleep is considered infinite.
	 */
	int64_t time;
};

#ifdef __cplusplus
}
#endif

#endif
