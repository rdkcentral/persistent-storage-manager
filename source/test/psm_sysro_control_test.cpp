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

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::IsNull;


// Stub functions for mocking
PSYS_IRA_INTERFACE testIraInterface = nullptr;

static ANSC_HANDLE MockGetIraIf(ANSC_HANDLE someObject) {
    return (ANSC_HANDLE)testIraInterface;
}

static ANSC_HANDLE MockAddSysFolder(ANSC_HANDLE hOwnerContext, char* pFolderName) {
    return (ANSC_HANDLE)testIraInterface;
}

static ANSC_HANDLE MockAddFolder(ANSC_HANDLE hThisObject, ANSC_HANDLE hCurFolder, char* pSubFolderName) {
    return (ANSC_HANDLE)testIraInterface;
}

static ANSC_STATUS MockCloseFolder ( ANSC_HANDLE hThisObject, ANSC_HANDLE hCurFolder)  {
    return ANSC_STATUS_SUCCESS;
}

// Actual test cases
TEST_F(CcspPsmTestFixture, SetupEnvTest) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr);
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    testIraInterface = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraInterface, nullptr);
    memset(testIraInterface, 0, sizeof(SYS_IRA_INTERFACE));
    
    testSysInfoRepository->GetIraIf = MockGetIraIf;
    testMyObject->hSysInfoRepository = (ANSC_HANDLE)testSysInfoRepository;
    
    testMyObject->bActive = true;
    testIraInterface->hOwnerContext = (ANSC_HANDLE)1234;
    testIraInterface-> AddSysFolder = MockAddSysFolder;
    testIraInterface->AddFolder = MockAddFolder;
    testIraInterface->CloseFolder = MockCloseFolder;
    

    ANSC_STATUS ret = PsmSysroSetupEnv((ANSC_HANDLE)testMyObject);

    EXPECT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testIraInterface);
    free(testSysInfoRepository);
    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, SetupEnvTest_bActiveFalse) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    PSYS_INFO_REPOSITORY_OBJECT testSysInfoRepository = (PSYS_INFO_REPOSITORY_OBJECT)malloc(sizeof(SYS_INFO_REPOSITORY_OBJECT));
    ASSERT_NE(testSysInfoRepository, nullptr);
    memset(testSysInfoRepository, 0, sizeof(SYS_INFO_REPOSITORY_OBJECT));

    testIraInterface = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraInterface, nullptr);
    memset(testIraInterface, 0, sizeof(SYS_IRA_INTERFACE));
    
    testSysInfoRepository->GetIraIf = MockGetIraIf;
    testMyObject->hSysInfoRepository = (ANSC_HANDLE)testSysInfoRepository;
    
    testMyObject->bActive = false;

    ANSC_STATUS ret = PsmSysroSetupEnv((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(ret, ANSC_STATUS_NOT_READY);

    free(testIraInterface);
    free(testSysInfoRepository);
    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, CloseEnvSuccess) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    testMyObject->bActive = TRUE;

    ANSC_STATUS result = PsmSysroCloseEnv(testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_SUCCESS);

    free(testMyObject);

}

TEST_F(CcspPsmTestFixture, CloseEnv_bActiveFalse) {

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    testMyObject->bActive = FALSE;

    ANSC_STATUS result = PsmSysroCloseEnv(testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_NOT_READY);

    free(testMyObject);
}

