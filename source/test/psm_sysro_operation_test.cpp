/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <experimental/filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "psm_mock.h"

extern "C" {
    #include "psm_sysro_global.h"
}

extern UserTimeMock * g_usertimeMock;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::IsNull;

// Stub functions for mocking
static ANSC_HANDLE MockGetIraIf(ANSC_HANDLE someObject) {
    return (ANSC_HANDLE)1234;
}

static ULONG MockStart(ANSC_HANDLE someObject) {
    return 1;
}

static ANSC_STATUS MockIraSetSysRamIf (ANSC_HANDLE hThisObject, ANSC_HANDLE hInterface) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockEngage (ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockSetupEnv (ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockCloseEnv (ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ULONG MockSetPsmCfmIf(ANSC_HANDLE hThisObject, ANSC_HANDLE hInterface) {
    return 0;
}

static ULONG MockSetSysIraIf(ANSC_HANDLE hThisObject, ANSC_HANDLE hInterface) {
    return 0;
}

static ANSC_STATUS MockLoadRegFile (ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockCancel (ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS; 
}

static ANSC_STATUS MockStop (ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS; 
}

static ANSC_STATUS MockAcqThreadLock(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockRelThreadLock(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockSaveRegFile(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

// Actual test cases
TEST_F(CcspPsmTestFixture, PsmSysRegistryEngageTest_bActiveTrue) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr);
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_RAM_INTERFACE testSysRamIf = (PSYS_RAM_INTERFACE)malloc(sizeof(SYS_RAM_INTERFACE));
    ASSERT_NE(testSysRamIf, nullptr);
    memset(testSysRamIf, 0, sizeof(SYS_RAM_INTERFACE));
    
    testSysInfoRepository->GetIraIf = MockGetIraIf;
    testMyObject->hSysInfoRepository = (ANSC_HANDLE)testSysInfoRepository;
    
    testMyObject->bActive = true;
    
    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testMyObject->hSysRamIf = (ANSC_HANDLE)testSysRamIf;

    ANSC_STATUS ret = PsmSysroEngage((ANSC_HANDLE)testMyObject);

    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testSysInfoRepository);
    free(testPsmFileLoader);
    free(testSysRamIf);
}

TEST_F(CcspPsmTestFixture, PsmSysRegistryEngageTest_bActiveIsFalse) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr);
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    PSYS_IRA_INTERFACE testIraInterface = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraInterface, nullptr);
    memset(testIraInterface, 0, sizeof(SYS_IRA_INTERFACE));

    PANSC_TIMER_DESCRIPTOR_OBJECT testTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(testTimerObj, nullptr);
    memset(testTimerObj, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_RAM_INTERFACE testSysRamIf = (PSYS_RAM_INTERFACE)malloc(sizeof(SYS_RAM_INTERFACE));
    ASSERT_NE(testSysRamIf, nullptr);
    memset(testSysRamIf, 0, sizeof(SYS_RAM_INTERFACE));
    
    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testMyObject->hSysRamIf = (ANSC_HANDLE)testSysRamIf;
    testMyObject->hSysInfoRepository = (ANSC_HANDLE)testSysInfoRepository;
    testMyObject->hRegTimerObj = (ANSC_HANDLE)testTimerObj;
    testSysInfoRepository->GetIraIf = MockGetIraIf;


    testSysInfoRepository->IraSetSysRamIf = MockIraSetSysRamIf;
    testSysInfoRepository->Engage = MockEngage;
    testTimerObj-> Start = MockStart;
    testMyObject->SetupEnv = MockSetupEnv;
    testPsmFileLoader -> SetPsmCfmIf = MockSetPsmCfmIf;
    testPsmFileLoader->SetSysIraIf = MockSetSysIraIf;
    testPsmFileLoader -> Engage = MockEngage;
    testPsmFileLoader -> LoadRegFile = MockLoadRegFile;

    testMyObject->bActive = FALSE;

    ANSC_STATUS ret = PsmSysroEngage((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testSysInfoRepository);
    free(testIraInterface);
    free(testTimerObj);
    free(testPsmFileLoader);
    free(testSysRamIf);
}

TEST_F(CcspPsmTestFixture, PsmSysRegistryCancelTest_bActiveIsFalse) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PANSC_TIMER_DESCRIPTOR_OBJECT testTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(testTimerObj, nullptr);
    memset(testTimerObj, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));

    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testMyObject->hRegTimerObj = (ANSC_HANDLE)testTimerObj;

    testMyObject->bActive = FALSE;

    ANSC_STATUS ret = PsmSysroCancel((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);
    
    free(testMyObject);
    free(testPsmFileLoader);
    free(testTimerObj);
}


TEST_F(CcspPsmTestFixture, PsmSysRegistryCancelTest_bActiveIsTrue) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PANSC_TIMER_DESCRIPTOR_OBJECT testTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(testTimerObj, nullptr);
    memset(testTimerObj, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));

    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testMyObject->hRegTimerObj = (ANSC_HANDLE)testTimerObj;

    testMyObject->bActive = TRUE;
    
    testPsmFileLoader->Cancel = MockCancel;
    testTimerObj->Stop = MockStop;
    testMyObject->CloseEnv = MockCloseEnv;

    ANSC_STATUS ret = PsmSysroCancel((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testPsmFileLoader);
    free(testTimerObj);
}

TEST_F(CcspPsmTestFixture, PsmSysroRegTimerInvokeTest_bNeedFlushFalse) {
    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraInterface = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraInterface, nullptr);
    memset(testIraInterface, 0, sizeof(SYS_IRA_INTERFACE));

    // Properly link dependencies
    testMyObject->bNeedFlush = FALSE;
    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testMyObject->hSysRamIf = (ANSC_HANDLE)testIraInterface;
    
    ANSC_STATUS ret = PsmSysroRegTimerInvoke((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testPsmFileLoader);
    free(testIraInterface);
}

TEST_F(CcspPsmTestFixture, PsmSysroRegTimerInvokeTest_LastRegWriteAtRecent) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testSysIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testSysIraIf, nullptr);
    memset(testSysIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testPsmFileLoader->hSysIraIf = (ANSC_HANDLE)testSysIraIf;

    testMyObject->bNeedFlush = TRUE;

    /* Check if enough time has passed since the last registry write.
    * If less than PSM_SYSRO_REG_FLUSH_DELAY (3 seconds) have elapsed,
    * we skip the rest of operations to avoid excessive writes.
    * 
    * In this case, we test LastRegWriteAt to be less than 3 seconds. 
    * So we skip other ops and return ANSC_STATUS_SUCCESS.
    */
    testMyObject->LastRegWriteAt = 1003;
    EXPECT_CALL(*g_usertimeMock, UserGetTickInSeconds2()).WillOnce(Return(1005));

    ANSC_STATUS ret = PsmSysroRegTimerInvoke((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testPsmFileLoader);
    free(testSysIraIf);
}

TEST_F(CcspPsmTestFixture, PsmSysroRegTimerInvokeTest_FileSyncRefCountGreaterThanZero) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testSysIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testSysIraIf, nullptr);
    memset(testSysIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    testMyObject->hOwnerContext = (ANSC_HANDLE)1;
    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testPsmFileLoader->hSysIraIf = (ANSC_HANDLE)testSysIraIf;
    testSysIraIf->AcqThreadLock = MockAcqThreadLock;
    testSysIraIf->RelThreadLock = MockRelThreadLock;

    testMyObject->bNeedFlush = TRUE;

    /* Check if enough time has passed since the last registry write.
    * If less than PSM_SYSRO_REG_FLUSH_DELAY (3 seconds) have elapsed,
    * we skip the rest of operations to avoid excessive writes.
    * 
    * In this case, we test LastRegWriteAt to be greater than 3 seconds. 
    * So we proceed with the rest of the write operations.
    */
    testMyObject->LastRegWriteAt = 1000;
    EXPECT_CALL(*g_usertimeMock, UserGetTickInSeconds2()).WillOnce(Return(1005));

    testMyObject->FileSyncRefCount = 1;

    ANSC_STATUS ret = PsmSysroRegTimerInvoke((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testPsmFileLoader);
    free(testSysIraIf);
}

TEST_F(CcspPsmTestFixture, PsmSysroRegTimerInvokeTest_FileSyncRefCountZero) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testSysIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testSysIraIf, nullptr);
    memset(testSysIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    testMyObject->hOwnerContext = (ANSC_HANDLE)1;
    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testPsmFileLoader->hSysIraIf = (ANSC_HANDLE)testSysIraIf;
    testSysIraIf->AcqThreadLock = MockAcqThreadLock;
    testSysIraIf->RelThreadLock = MockRelThreadLock;
    testPsmFileLoader->SaveRegFile = MockSaveRegFile;

    testMyObject->bNeedFlush = TRUE;

    testMyObject->LastRegWriteAt = 1000;
    EXPECT_CALL(*g_usertimeMock, UserGetTickInSeconds2())
        .Times(2)
        .WillRepeatedly(Return(1005));

    testMyObject->FileSyncRefCount = 0;
    ANSC_STATUS ret = PsmSysroRegTimerInvoke((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testPsmFileLoader);
    free(testSysIraIf);
}
