/*
 * Stub header for factory_info_api.h
 *
 * The real header ships with the Technicolor platform SDK
 * (factory-nvram package) and is not available in public repositories.
 * This stub provides the minimum declarations needed so that the
 * Coverity / native build can compile the platform-guarded code in
 * source/Ssp/psm_hal_apis.c without the vendor SDK.
 *
 * Only used when _XB7_PRODUCT_REQ_ / _CBR_PRODUCT_REQ_ /
 * _XER5_PRODUCT_REQ_ is defined together with _COSA_BCM_ARM_.
 */

#ifndef FACTORY_INFO_API_H
#define FACTORY_INFO_API_H

#ifdef __cplusplus
extern "C" {
#endif

int factory_info_get_default_24_SSID(char *outputData);
int factory_info_get_default_50_SSID(char *outputData);
int factory_info_get_default_60_SSID(char *outputData);
int factory_info_get_default_xhs_SSID(char *outputData);
int factory_info_get_wifi_passwd(char *outputData);
int factory_info_get_xhs_passkey(char *outputData);

#ifdef __cplusplus
}
#endif

#endif /* FACTORY_INFO_API_H */
