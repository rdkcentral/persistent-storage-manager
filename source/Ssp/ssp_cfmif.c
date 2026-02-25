/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/


/**********************************************************************

    module:	ssp_cfmif.c

        For Persistent Storage Manager Implementation (PSM),
        Common Component Software Platform (CCSP)

   ---------------------------------------------------------------

    description:

        This module implements the advanced interface functions
        of the Psm Sys Registry Object.

        *   ssp_CfmReadCurConfig
        *   ssp_CfmReadDefConfig
        *   ssp_CfmSaveCurConfig
        *   ssp_CfmUpdateConfigs

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Jian Wu
        Lei Chen <leichen2@cisco.com>

    ---------------------------------------------------------------

    revision:

        04/25/12    initial revision.
        08/06/12    adjusting the function to support backup/default config,
                    adding codes to support custom logic.
        08/28/12    adding "Update Configs" and some internal functions.

**********************************************************************/
#include "ssp_global.h"
#include "psm_ifo_cfm.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif

/*
 * Read current configuration into memory.
 * Since XML/DOM is removed, we just return an empty buffer (NULL/0) and success.
 */
ANSC_STATUS
ssp_CfmReadCurConfig
(
    ANSC_HANDLE hThisObject,
    void**      ppCfgBuffer,
    PULONG      pulCfgSize
)
{
    UNREFERENCED_PARAMETER(hThisObject);
    if (ppCfgBuffer) { *ppCfgBuffer = NULL; }
    if (pulCfgSize)  { *pulCfgSize  = 0; }
    return ANSC_STATUS_SUCCESS;
}

/*
 * Read default configuration into memory.
 * Returns empty buffer to keep compatibility without XML.
 */
ANSC_STATUS
ssp_CfmReadDefConfig
(
    ANSC_HANDLE hThisObject,
    void**      ppCfgBuffer,
    PULONG      pulCfgSize
)
{
    UNREFERENCED_PARAMETER(hThisObject);
    if (ppCfgBuffer) { *ppCfgBuffer = NULL; }
    if (pulCfgSize)  { *pulCfgSize  = 0; }
    return ANSC_STATUS_SUCCESS;
}

/*
 * Save current configuration.
 * No-op now that XML/DOM persistence is removed.
 */
ANSC_STATUS
ssp_CfmSaveCurConfig
(
    ANSC_HANDLE hThisObject,
    void*       pCfgBuffer,
    ULONG       ulCfgSize
)
{
    UNREFERENCED_PARAMETER(hThisObject);
    UNREFERENCED_PARAMETER(pCfgBuffer);
    UNREFERENCED_PARAMETER(ulCfgSize);
    return ANSC_STATUS_SUCCESS;
}

/*
 * Update device configs from a new config path.
 * No-op without XML; returns success to avoid breaking callers.
 */
ANSC_STATUS
ssp_CfmUpdateConfigs
(
    ANSC_HANDLE  hThisObject,
    const char*  newConfPath
)
{
    UNREFERENCED_PARAMETER(hThisObject);
    UNREFERENCED_PARAMETER(newConfPath);
    return ANSC_STATUS_SUCCESS;
}
