#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_library_sources(cereg.c)
zephyr_library_sources(cfun.c)
zephyr_library_sources(cscon.c)
zephyr_library_sources(mdmev.c)
zephyr_library_sources(xsystemmode.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_CONEVAL coneval.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_EDRX edrx.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_NCELLMEAS ncellmeas.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_PSM psm.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_PERIODICSEARCHCONF periodicsearchconf.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_REDMOB redmob.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_XFACTORYRESET xfactoryreset.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_XMODEMSLEEP xmodemsleep.c)
zephyr_library_sources_ifdef(CONFIG_LTE_LC_XT3412 xt3412.c)
