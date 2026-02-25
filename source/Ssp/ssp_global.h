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

#include "ansc_platform.h"

#include "ansc_ifo_interface.h"

#include "ccsp_custom.h"

/* Keep only the PSM SysRegistry headers (no XML/FileLoader/IRA) */
#include "psm_co_name.h"
#include "psm_properties.h"
#include "psm_sysro_exported_api.h"
#include "psm_sysro_interface.h"

#include "ccsp_message_bus.h"
#include "ccsp_base_api.h"

#include "psm_hal_apis.h"

#include "dslh_cpeco_interface.h"
#include "dslh_cpeco_exported_api.h"
/*
 *  Define custom trace module ID
 */
#ifdef   ANSC_TRACE_MODULE_ID
    #undef  ANSC_TRACE_MODULE_ID
#endif

#define  ANSC_TRACE_MODULE_ID                       ANSC_TRACE_ID_SSP

#define  CCSP_COMPONENT_VERSION_PSM                 1
#define  CCSP_COMPONENT_AUTHOR_PSM                  "Jian Wu"

#include "syscfg_lmdb.h"
#define LMDB_PERSIST_DIR "/nvram/syscfg_lmdb"
#define MAPSIZE (8u * 1024u * 1024u)
// LMDB context for syscfg_lmdb
extern syscfg_lmdb_t *g_lmdb_ctx;


int  cmd_dispatch(int  command);
int  gather_info ();

int  PsmDbusInit ();
int  PsmRbusInit ();

int  setParameterValues(
    int sessionId,
    unsigned int writeID,
    parameterValStruct_t *val,
    int size,
    dbus_bool commit,
    char **str,
    void            *user_data
);

int  getParameterValues(
    unsigned int writeID,
    char * parameterNames[],
    int size,
    int *val_size,
    parameterValStruct_t ***param_val,
    void * user_data
);

int  setParameterAttributes(
    int sessionId,
    parameterAttributeStruct_t *val,
    int size,
    void            *user_data
);

int getParameterNames(
    char * parameterName,
    dbus_bool nextLevel,
    int *val_size ,
    parameterInfoStruct_t ***param_val,
    void *user_data
);
