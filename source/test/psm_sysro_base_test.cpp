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

extern SafecLibMock * g_safecLibMock;
extern AnscCoMock* g_anscCoMock;
extern AnscMemoryMock * g_anscMemoryMock;
extern SysInfoRepositoryMock * g_sysInfoRepositoryMock;
extern AnscTimerSchedulerMock * g_anscTimerSchedulerMock;

using ::testing::_;
using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::Eq;

// Stub functions for mocking
static ANSC_STATUS MockCancel(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockReset(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockRemove(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockSetTimerType (ANSC_HANDLE hThisObject, ULONG ulTimerType) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockSetInterval (ANSC_HANDLE hThisObject, ULONG ulInterval) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockSetClient(ANSC_HANDLE hThisObject, ANSC_HANDLE hClientContext) {
    return ANSC_STATUS_SUCCESS;
}

// Actual test cases
TEST_F(CcspPsmTestFixture, CreatePsmSysRegistryObject) {

    ANSC_HANDLE hContainerContext = (ANSC_HANDLE)1;
    ANSC_HANDLE hOwnerContext = (ANSC_HANDLE)2;
    ANSC_HANDLE hAnscReserved = (ANSC_HANDLE)3;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .Times(2)
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(_))
        .Times(1)
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_sysInfoRepositoryMock, SysCreateInfoRepository(_, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return((ANSC_HANDLE)NULL));
    
    ANSC_HANDLE testObject = PsmSysroCreate(hContainerContext, hOwnerContext, hAnscReserved);
    ASSERT_NE(testObject, (ANSC_HANDLE)NULL) << "Failed to create PSM_SYS_REGISTRY_OBJECT";

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)testObject;
    PANSC_COMPONENT_OBJECT pBaseObject = (PANSC_COMPONENT_OBJECT)testMyObject;

    EXPECT_EQ(pBaseObject->Oid, PSM_SYS_REGISTRY_OID);
    EXPECT_EQ(pBaseObject->hContainerContext, hContainerContext);
    EXPECT_EQ(pBaseObject->hOwnerContext, hOwnerContext);

    free(testObject);
    testObject = (ANSC_HANDLE)NULL;
}


TEST_F(CcspPsmTestFixture, PsmSysroRemoveTest) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PPSM_CFM_INTERFACE testPsmCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testPsmCfmIf, nullptr);
    memset(testPsmCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PPSM_FILE_LOADER_OBJECT testPsmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testPsmFileLoader, nullptr);
    memset(testPsmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_RAM_INTERFACE testSysRamIf = (PSYS_RAM_INTERFACE)malloc(sizeof(SYS_RAM_INTERFACE));
    ASSERT_NE(testSysRamIf, nullptr);
    memset(testSysRamIf, 0, sizeof(SYS_RAM_INTERFACE));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr);
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    PANSC_TIMER_DESCRIPTOR_OBJECT testRegTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(testRegTimerObj, nullptr);
    memset(testRegTimerObj, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));

    PANSC_TDO_CLIENT_OBJECT testRegTimerIf = (PANSC_TDO_CLIENT_OBJECT)malloc(sizeof(ANSC_TDO_CLIENT_OBJECT));
    ASSERT_NE(testRegTimerIf, nullptr);
    memset(testRegTimerIf, 0, sizeof(ANSC_TDO_CLIENT_OBJECT));

    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testPsmCfmIf;
    testMyObject->hPsmFileLoader = (ANSC_HANDLE)testPsmFileLoader;
    testMyObject->hSysRamIf = (ANSC_HANDLE)testSysRamIf;
    testMyObject->hSysInfoRepository = (ANSC_HANDLE)testSysInfoRepository;
    testMyObject->hRegTimerObj = (ANSC_HANDLE)testRegTimerObj;
    testMyObject->hRegTimerIf = (ANSC_HANDLE)testRegTimerIf;

    testMyObject->Cancel = MockCancel;
    testMyObject->Reset = MockReset;
    testPsmFileLoader->Remove = MockRemove;
    testSysInfoRepository->Remove = MockRemove;
    testRegTimerObj->Remove = MockRemove;

    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    ANSC_STATUS ret = PsmSysroRemove((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    EXPECT_EQ(testMyObject->hPsmCfmIf, (ANSC_HANDLE)NULL);
    EXPECT_EQ(testMyObject->hPsmFileLoader, (ANSC_HANDLE)NULL);
    EXPECT_EQ(testMyObject->hSysRamIf, (ANSC_HANDLE)NULL);
    EXPECT_EQ(testMyObject->hSysInfoRepository, (ANSC_HANDLE)NULL);

    free(testMyObject);
    free(testPsmCfmIf);
    free(testPsmFileLoader);
    free(testSysRamIf);
    free(testSysInfoRepository);
    free(testRegTimerObj);
    free(testRegTimerIf);
}

TEST_F(CcspPsmTestFixture, InitializesPsmSysRegistryObject) {

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate PSM_SYS_REGISTRY_OBJECT";
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    ANSC_STATUS status = PsmSysroInitialize(testMyObject);
    ASSERT_EQ(status, ANSC_STATUS_SUCCESS) << "Failed to initialize PSM_SYS_REGISTRY_OBJECT";

    ASSERT_EQ(testMyObject->Oid, PSM_SYS_REGISTRY_OID);

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, EnrollsObjects_Success) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate PSM_SYS_REGISTRY_OBJECT";
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr) << "Failed to allocate SYS_INFO_REPOSITORY_OBJECT";
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    PANSC_TIMER_DESCRIPTOR_OBJECT testRegTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(testRegTimerObj, nullptr) << "Failed to allocate ANSC_TIMER_DESCRIPTOR_OBJECT";
    memset(testRegTimerObj, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));

    testMyObject -> hPsmCfmIf              =   (ANSC_HANDLE)NULL;
    testMyObject -> hPsmFileLoader         =   (ANSC_HANDLE)NULL;
    testMyObject -> hSysRamIf              =   (ANSC_HANDLE)NULL;
    testMyObject -> hSysInfoRepository     =   (ANSC_HANDLE)NULL;
    testMyObject->  hRegTimerObj           =   (ANSC_HANDLE)NULL;
    testMyObject->  hRegTimerIf            =   (ANSC_HANDLE)NULL;

    testRegTimerObj->SetTimerType = MockSetTimerType;
    testRegTimerObj->SetInterval = MockSetInterval;
    testRegTimerObj->SetClient = MockSetClient;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_sysInfoRepositoryMock, SysCreateInfoRepository(_, _, _))
        .Times(1)
        .WillOnce(Return(testSysInfoRepository));

    EXPECT_CALL(*g_anscTimerSchedulerMock, AnscCreateTimerDescriptor(_, _, _))
        .Times(1)
        .WillOnce(Return(testRegTimerObj));
    
    ANSC_STATUS status = PsmSysroEnrollObjects((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS) << "Failed to enroll objects.";

    free(testMyObject);
    free(testSysInfoRepository);
    free(testRegTimerObj);
}

TEST_F(CcspPsmTestFixture, EnrollsObjects_SysCreateInfoRepository_Failed) {
    
    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate PSM_SYS_REGISTRY_OBJECT";
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    testMyObject -> hPsmCfmIf              =   (ANSC_HANDLE)NULL;
    testMyObject -> hPsmFileLoader         =   (ANSC_HANDLE)NULL;
    testMyObject -> hSysRamIf              =   (ANSC_HANDLE)NULL;
    testMyObject -> hSysInfoRepository     =   (ANSC_HANDLE)NULL;
    testMyObject->  hRegTimerObj           =   (ANSC_HANDLE)NULL;
    testMyObject->  hRegTimerIf            =   (ANSC_HANDLE)NULL;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_sysInfoRepositoryMock, SysCreateInfoRepository(_, _, _))
        .Times(1)
        .WillRepeatedly(Return((ANSC_HANDLE)NULL));

    ANSC_STATUS status = PsmSysroEnrollObjects((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(status, ANSC_STATUS_RESOURCES) << "Failed to enroll objects.";

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, EnrollsObjects_AnscCreateTimerDescriptor_Failed) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate PSM_SYS_REGISTRY_OBJECT";
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr) << "Failed to allocate SYS_INFO_REPOSITORY_OBJECT";
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    testMyObject -> hPsmCfmIf              =   (ANSC_HANDLE)NULL;
    testMyObject -> hPsmFileLoader         =   (ANSC_HANDLE)NULL;
    testMyObject -> hSysRamIf              =   (ANSC_HANDLE)NULL;
    testMyObject -> hSysInfoRepository     =   (ANSC_HANDLE)NULL;
    testMyObject->  hRegTimerObj           =   (ANSC_HANDLE)NULL;
    testMyObject->  hRegTimerIf            =   (ANSC_HANDLE)NULL;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_sysInfoRepositoryMock, SysCreateInfoRepository(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(testSysInfoRepository));

    EXPECT_CALL(*g_anscTimerSchedulerMock, AnscCreateTimerDescriptor(_, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return((ANSC_HANDLE)NULL));

    ANSC_STATUS status = PsmSysroEnrollObjects((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(status, ANSC_STATUS_RESOURCES) << "Failed to enroll objects.";

    free(testMyObject);
    free(testSysInfoRepository);
}

TEST_F(CcspPsmTestFixture, EnrollsObjects_strcpy_s_error1) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate PSM_SYS_REGISTRY_OBJECT";
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(-1))
        .WillRepeatedly(Return(0));

    ANSC_STATUS status = PsmSysroEnrollObjects((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE) << "Failed to enroll objects.";

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, EnrollsObjects_strcpy_s_error2) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate PSM_SYS_REGISTRY_OBJECT";
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(0))
        .WillOnce(Return(-1))
        .WillRepeatedly(Return(0));
        
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    ANSC_STATUS status = PsmSysroEnrollObjects((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE) << "Failed to enroll objects.";

    free(testMyObject);
}

