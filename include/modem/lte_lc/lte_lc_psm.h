#ifndef LTE_LC_PSM_H__
#define LTE_LC_PSM_H__

#ifdef __cplusplus
extern "C" {
#endif

/** Power Saving Mode (PSM) configuration. */
struct lte_lc_psm_cfg {
	/** Periodic Tracking Area Update interval in seconds. */
	int tau;

	/** Active-time (time from RRC idle to PSM) in seconds or @c -1 if PSM is deactivated. */
	int active_time;
};

#ifdef __cplusplus
}
#endif

#endif
