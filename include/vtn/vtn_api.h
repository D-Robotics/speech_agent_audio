#pragma once
#ifndef VTN_API_H
#define VTN_API_H

#include "vtn_param.h"

typedef struct vtn_context_s *vtn_handle;

#define VTN_API_LIST(func)    \
	func(vtn_api_get_version) \
	func(vtn_api_init)        \
	func(vtn_api_interact)    \
	func(vtn_api_destroy)


#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief get version
     * 
     * @return const char*  version string
     */
    ATTR_PUBLIC(const char *) vtn_api_get_version();
    typedef const char *(ATTR_CALLING_CONVENTION * proc_vtn_api_get_version)();

    /**
     * @brief init api by param
     * 
     * @param handle_p pointer of vtn_handle
     * @param param_p pointer of vtn_init_param_t
     * @return vtn_status_code_e of init status
     */
    ATTR_PUBLIC(vtn_status_code_e) vtn_api_init(vtn_handle *handle_p, vtn_init_param_t *param_p);
    typedef vtn_status_code_e(ATTR_CALLING_CONVENTION * proc_vtn_api_init)(vtn_handle *handle_p, vtn_init_param_t *param_p);

    /**
     * @brief interact with api
     * 
     * @param handle vtn_handle
     * @param info_p pointer of interact info
     * @return vtn_status_code_e of interact result status
     */
    ATTR_PUBLIC(vtn_status_code_e) vtn_api_interact(vtn_handle handle, vtn_interact_info_t *info_p);
    typedef vtn_status_code_e(ATTR_CALLING_CONVENTION * proc_vtn_api_interact)(vtn_handle handle, vtn_interact_info_t *info_p);

    /**
     * @brief destroy api
     * 
     * @param handle vtn_handle
     * @return vtn_status_code_e of destroy result 
     */
    ATTR_PUBLIC(vtn_status_code_e) vtn_api_destroy(vtn_handle handle);
    typedef vtn_status_code_e(ATTR_CALLING_CONVENTION * proc_vtn_api_destroy)(vtn_handle handle);

#ifdef __cplusplus
}
#endif

#endif // !VTN_API_H
