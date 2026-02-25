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

    module:	psm_sysro_base.c

        For Persistent Storage Manager Implementation (PSM),
        Common Component Software Platform (CCSP)

    ---------------------------------------------------------------

    description:

        This module implements the basic container object functions
        of the Psm Sys Registry Object.

        *   PsmSysroCreate
        *   PsmSysroRemove
        *   PsmSysroEnrollObjects
        *   PsmSysroInitialize

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Jian Wu

    ---------------------------------------------------------------

    revision:

        06/17/11    initial revision.

**********************************************************************/


#include "psm_sysro_global.h"
#include "safec_lib_common.h"

/* =================================================================================
 * NO-OP STUBS for removed XML/DOM paths (correct prototypes for Import/Export)
 * ================================================================================= */

/* import: (hThisObject, pBuf, ulBufSize, hReserved, ulFlags) -> ANSC_STATUS */
static ANSC_STATUS
PsmSysroNoopImport
(
    ANSC_HANDLE  hThisObject,
    void*        pBuffer,
    ULONG        ulBufSize,
    ANSC_HANDLE  hReserved,
    ULONG        ulFlags
)
{
    UNREFERENCED_PARAMETER(hThisObject);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(ulBufSize);
    UNREFERENCED_PARAMETER(hReserved);
    UNREFERENCED_PARAMETER(ulFlags);
    return ANSC_STATUS_SUCCESS;
}

/* export: (hThisObject, pBuf, pUlSize /*out* /, hReserved, ulFlags) -> ANSC_STATUS */
static ANSC_STATUS
PsmSysroNoopExport
(
    ANSC_HANDLE  hThisObject,
    void*        pBuffer,
    PULONG       pulBufSize,
    ANSC_HANDLE  hReserved,
    ULONG        ulFlags
)
{
    UNREFERENCED_PARAMETER(hThisObject);
    /* If caller passed a size pointer, just report 0 and don't touch pBuffer */
    if (pulBufSize) { *pulBufSize = 0; }
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(hReserved);
    UNREFERENCED_PARAMETER(ulFlags);
    return ANSC_STATUS_SUCCESS;
}

/* Generic success no-op for status-returning ops */
static ANSC_STATUS
PsmSysroNoopStatus(ANSC_HANDLE hThisObject)
{
    UNREFERENCED_PARAMETER(hThisObject);
    return ANSC_STATUS_SUCCESS;
}

/* Return size = 0 */
static ULONG
PsmSysroNoopGetSize(ANSC_HANDLE hThisObject)
{
    UNREFERENCED_PARAMETER(hThisObject);
    return 0;
}

/* CFM read returns empty buffer */
static ANSC_STATUS
PsmSysroNoopCfmRead
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

/* CFM save is a no-op */
static ANSC_STATUS
PsmSysroNoopCfmSave
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

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        PsmSysroCreate
            (
                ANSC_HANDLE                 hContainerContext,
                ANSC_HANDLE                 hOwnerContext,
                ANSC_HANDLE                 hAnscReserved
            );

    description:

        This function constructs the Psm Sys Registry Object and
        initializes the member variables and functions.

    argument:   ANSC_HANDLE                 hContainerContext
                This handle is used by the container object to interact
                with the outside world. It could be the real container
                or an target object.

                ANSC_HANDLE                 hOwnerContext
                This handle is passed in by the owner of this object.

                ANSC_HANDLE                 hAnscReserved
                This handle is passed in by the owner of this object.

    return:     newly created container object.

**********************************************************************/

ANSC_HANDLE
PsmSysroCreate
    (
        ANSC_HANDLE                 hContainerContext,
        ANSC_HANDLE                 hOwnerContext,
        ANSC_HANDLE                 hAnscReserved
    )
{
    UNREFERENCED_PARAMETER(hAnscReserved);
    PANSC_COMPONENT_OBJECT          pBaseObject  = NULL;
    PPSM_SYS_REGISTRY_OBJECT        pMyObject    = NULL;
   // CcspTraceInfo(("\n##PsmSysRoCreate() begins##\n"));

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PPSM_SYS_REGISTRY_OBJECT)AnscAllocateMemory(sizeof(PSM_SYS_REGISTRY_OBJECT));
    CcspTraceInfo(("\nPsmSysReg Object mem alloc\n"));
	
    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }
    else
    {
        pBaseObject = (PANSC_COMPONENT_OBJECT)pMyObject;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    /* AnscCopyString(pBaseObject->Name, PSM_SYS_REGISTRY_NAME); */

    pBaseObject->hContainerContext = hContainerContext;
    pBaseObject->hOwnerContext     = hOwnerContext;
    pBaseObject->Oid               = PSM_SYS_REGISTRY_OID;
    pBaseObject->Create            = PsmSysroCreate;
    pBaseObject->Remove            = PsmSysroRemove;
    pBaseObject->EnrollObjects     = PsmSysroEnrollObjects;
    pBaseObject->Initialize        = PsmSysroInitialize;

    pBaseObject->EnrollObjects((ANSC_HANDLE)pBaseObject);
    pBaseObject->Initialize   ((ANSC_HANDLE)pBaseObject);
	//CcspTraceInfo(("\n##PsmSysRoCreate() ends##\n"));
    return  (ANSC_HANDLE)pMyObject;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_STATUS
        PsmSysroRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function destroys the object.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of this object
                itself.

    return:     status of operation.

**********************************************************************/

ANSC_STATUS
PsmSysroRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    PPSM_SYS_REGISTRY_OBJECT        pMyObject    = (PPSM_SYS_REGISTRY_OBJECT)hThisObject;
    PANSC_TDO_CLIENT_OBJECT         pRegTimerIf  = (PANSC_TDO_CLIENT_OBJECT)pMyObject->hRegTimerIf;
    PANSC_TIMER_DESCRIPTOR_OBJECT   pRegTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)pMyObject->hRegTimerObj;

    pMyObject->Cancel((ANSC_HANDLE)pMyObject);
    pMyObject->Reset ((ANSC_HANDLE)pMyObject);

    /* Free enrolled objects still present */
    if (pRegTimerObj) { pRegTimerObj->Remove((ANSC_HANDLE)pRegTimerObj); }
    if (pRegTimerIf)  { AnscFreeMemory(pRegTimerIf); }

    AnscFreeLock(&pMyObject->AccessLock);
    AnscCoRemove((ANSC_HANDLE)pMyObject);
    CcspTraceInfo(("\nmemory dealloc\n"));
//	CcspTraceInfo(("\n##PsmSysRoRemove() ends##\n"));
    return  ANSC_STATUS_SUCCESS;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_STATUS
        PsmSysroEnrollObjects
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function enrolls all the objects required by this object.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of this object
                itself.

    return:     status of operation.

**********************************************************************/

ANSC_STATUS
PsmSysroEnrollObjects
    (
        ANSC_HANDLE                 hThisObject
    )
{
    PPSM_SYS_REGISTRY_OBJECT        pMyObject          = (PPSM_SYS_REGISTRY_OBJECT)hThisObject;
    PSYS_RAM_INTERFACE              pSysRamIf          = (PSYS_RAM_INTERFACE)pMyObject->hSysRamIf;
    PSYS_INFO_REPOSITORY_OBJECT     pSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)pMyObject->hSysInfoRepository;
    PANSC_TIMER_DESCRIPTOR_OBJECT   pRegTimerObj       = (PANSC_TIMER_DESCRIPTOR_OBJECT)pMyObject->hRegTimerObj;
    PANSC_TDO_CLIENT_OBJECT         pRegTimerIf        = (PANSC_TDO_CLIENT_OBJECT)pMyObject->hRegTimerIf;
    errno_t                         rc                 = -1;

    /* Legacy XML/DOM subsystems removed:
       - PPSM_CFM_INTERFACE pPsmCfmIf
       - PPSM_FILE_LOADER_OBJECT pPsmFileLoader
    */

    /* SysRamIf (kept; methods may be no-op stubs) */
    if (!pSysRamIf)
    {
        pSysRamIf = (PSYS_RAM_INTERFACE)AnscAllocateMemory(sizeof(SYS_RAM_INTERFACE));
        if (!pSysRamIf) { return ANSC_STATUS_RESOURCES; }

        pMyObject->hSysRamIf = (ANSC_HANDLE)pSysRamIf;

        rc = strcpy_s(pSysRamIf->Name, sizeof(pSysRamIf->Name), SYS_RAM_INTERFACE_NAME);
        if (rc != EOK) { ERR_CHK(rc); AnscFreeMemory(pSysRamIf); return ANSC_STATUS_FAILURE; }

        pSysRamIf->InterfaceId   = SYS_RAM_INTERFACE_ID;
        pSysRamIf->hOwnerContext = (ANSC_HANDLE)pMyObject;
        pSysRamIf->Size          = sizeof(SYS_RAM_INTERFACE);
        pSysRamIf->EnableFileSync= PsmSysroSysRamEnableFileSync; /* may be no-op */
        pSysRamIf->Notify        = PsmSysroSysRamNotify;         /* may be no-op */
    }

    /* SysInfoRepository (kept if used by platform; remove if not needed) */
    if (!pSysInfoRepository)
    {
        pSysInfoRepository =
            (PSYS_INFO_REPOSITORY_OBJECT)SysCreateInfoRepository
            (
                pMyObject->hContainerContext,
                (ANSC_HANDLE)pMyObject,
                (ANSC_HANDLE)NULL
            );
        if (!pSysInfoRepository) { return ANSC_STATUS_RESOURCES; }
        pMyObject->hSysInfoRepository = (ANSC_HANDLE)pSysInfoRepository;
    }

    /* Periodic registry timer (kept) */
    if (!pRegTimerObj)
    {
        pRegTimerObj =
            (PANSC_TIMER_DESCRIPTOR_OBJECT)AnscCreateTimerDescriptor
            (
                pMyObject->hContainerContext,
                (ANSC_HANDLE)pMyObject,
                (ANSC_HANDLE)NULL
            );
        if (!pRegTimerObj) { return ANSC_STATUS_RESOURCES; }

        pMyObject->hRegTimerObj = (ANSC_HANDLE)pRegTimerObj;
        pRegTimerObj->SetTimerType((ANSC_HANDLE)pRegTimerObj, ANSC_TIMER_TYPE_PERIODIC);
        pRegTimerObj->SetInterval ((ANSC_HANDLE)pRegTimerObj, PSM_SYSRO_REG_TIMER_INTERVAL);
        /* _ansc_strcpy(pRegTimerObj->Name, "PsmSysroRegTimer"); */
    }

    if (!pRegTimerIf)
    {
        pRegTimerIf = (PANSC_TDO_CLIENT_OBJECT)AnscAllocateMemory(sizeof(ANSC_TDO_CLIENT_OBJECT));
        if (!pRegTimerIf) { return ANSC_STATUS_RESOURCES; }

        pMyObject->hRegTimerIf      = (ANSC_HANDLE)pRegTimerIf;
        pRegTimerIf->hClientContext = (ANSC_HANDLE)pMyObject;
        pRegTimerIf->Invoke         = PsmSysroRegTimerInvoke;

        if (pRegTimerObj)
        {
            pRegTimerObj->SetClient((ANSC_HANDLE)pRegTimerObj, (ANSC_HANDLE)pRegTimerIf);
        }
    }

    AnscCoEnrollObjects((ANSC_HANDLE)pMyObject);
    return ANSC_STATUS_SUCCESS;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_STATUS
        PsmSysroInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function first calls the initialization member function
        of the base class object to set the common member fields
        inherited from the base class. It then initializes the member
        fields that are specific to this object.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of this object
                itself.

    return:     status of operation.

**********************************************************************/

ANSC_STATUS
PsmSysroInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    PPSM_SYS_REGISTRY_OBJECT       pMyObject    = (PPSM_SYS_REGISTRY_OBJECT)hThisObject;

    /*
     * Until you have to simulate C++ object-oriented programming style with standard C, you don't
     * appreciate all the nice little things come with C++ language and all the dirty works that
     * have been done by the C++ compilers. Member initialization is one of these things. While in
     * C++ you don't have to initialize all the member fields inherited from the base class since
     * the compiler will do it for you, such is not the case with C.
     */
    AnscCoInitialize((ANSC_HANDLE)pMyObject);

    /* base wiring */
    pMyObject->Oid            = PSM_SYS_REGISTRY_OID;
    pMyObject->Create         = PsmSysroCreate;
    pMyObject->Remove         = PsmSysroRemove;
    pMyObject->EnrollObjects  = PsmSysroEnrollObjects;
    pMyObject->Initialize     = PsmSysroInitialize;

    /* state */
    pMyObject->LastRegWriteAt   = 0;
    pMyObject->LastRegFlushAt   = 0;
    pMyObject->FileSyncRefCount = 0;
    pMyObject->bNoSave          = TRUE;  /* do NOT allow save until being notified */
    pMyObject->bNeedFlush       = FALSE; /* indicate whether save is required   */

    /* protect */
    AnscAcquireLock(&pMyObject->AccessLock);
    pMyObject->bSaveInProgress = FALSE;
    AnscReleaseLock(&pMyObject->AccessLock);

    pMyObject->bProcSeparation = TRUE; /* whether registry is in separate process */
    pMyObject->bActive         = FALSE;

    /* getters/setters */
    pMyObject->GetPsmSseIf         = PsmSysroGetPsmSseIf;
    pMyObject->SetPsmSseIf         = PsmSysroSetPsmSseIf;
    pMyObject->GetPsmFileLoader    = PsmSysroGetPsmFileLoader;     /* now returns NULL */
    pMyObject->GetSysInfoRepository= PsmSysroGetSysInfoRepository; /* valid if kept    */
    pMyObject->GetProperty         = PsmSysroGetProperty;
    pMyObject->SetProperty         = PsmSysroSetProperty;
    pMyObject->ResetProperty       = PsmSysroResetProperty;
    pMyObject->Reset               = PsmSysroReset;

    /* lifecycle */
    pMyObject->Engage              = PsmSysroEngage;
    pMyObject->Cancel              = PsmSysroCancel;
    pMyObject->RegTimerInvoke      = PsmSysroRegTimerInvoke;
    pMyObject->SetupEnv            = PsmSysroSetupEnv;
    pMyObject->CloseEnv            = PsmSysroCloseEnv;
    pMyObject->ResetToFactoryDefault = PsmSysroResetToFactoryDefault;

    /* XML/DOM persistence hooks -> NO-OPS */
    pMyObject->ImportConfig        = PsmSysroNoopStatus;
    pMyObject->ExportConfig        = PsmSysroNoopStatus;
    pMyObject->GetConfigSize       = PsmSysroNoopGetSize;
    pMyObject->SaveConfigToFlash   = PsmSysroNoopStatus;

    pMyObject->CfmReadCurConfig    = PsmSysroNoopCfmRead;
    pMyObject->CfmReadDefConfig    = PsmSysroNoopCfmRead;
    pMyObject->CfmSaveCurConfig    = PsmSysroNoopCfmSave;

    /* SysRam notifications (kept; may be stubs) */
    pMyObject->SysRamEnableFileSync= PsmSysroSysRamEnableFileSync;
    pMyObject->SysRamNotify        = PsmSysroSysRamNotify;

    AnscInitializeLock(&pMyObject->AccessLock);

    /* initialize properties with defaults (no XML path defaults) */
    pMyObject->ResetProperty((ANSC_HANDLE)pMyObject);

    return ANSC_STATUS_SUCCESS;
}
