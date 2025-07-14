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

extern AnscMemoryMock * g_anscMemoryMock;
extern UserTimeMock * g_usertimeMock;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::IsNull;

TEST_F(CcspPsmTestFixture, SysRam_EnableFileSync) {

   EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)AnscAllocateMemory(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr) << "Failed to allocate memory for PSM_SYS_REGISTRY_OBJECT";

    testMyObject->FileSyncRefCount = 0;

    ANSC_STATUS status = PsmSysroSysRamEnableFileSync((ANSC_HANDLE)testMyObject, TRUE);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS) << "[PsmSysroSysRamEnableFileSync] Failed! when enabling file sync";
    EXPECT_EQ(testMyObject->FileSyncRefCount, -1);

    if (testMyObject) {
            free(testMyObject);
            testMyObject = nullptr;
    }
}

TEST_F(CcspPsmTestFixture, SysRam_DisableFileSync) {

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)AnscAllocateMemory(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr)  << "Failed to allocate memory for PSM_SYS_REGISTRY_OBJECT";

    testMyObject->FileSyncRefCount = 0;

    ANSC_STATUS status = PsmSysroSysRamEnableFileSync((ANSC_HANDLE)testMyObject, FALSE);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS) << "[PsmSysroSysRamEnableFileSync] Failed! when disabling file sync";
    EXPECT_EQ(testMyObject->FileSyncRefCount, 1);

    if (testMyObject) {
        free(testMyObject);
        testMyObject = nullptr;
    }

}

TEST_F(CcspPsmTestFixture, SysRamNotify_NoSave) {
    
    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)AnscAllocateMemory(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);

    testMyObject->bNoSave = TRUE;
    testMyObject->bSaveInProgress = FALSE;
    testMyObject->LastRegWriteAt = 0;
    testMyObject->bNeedFlush = FALSE;

    ANSC_STATUS status = PsmSysroSysRamNotify((ANSC_HANDLE)testMyObject, nullptr, SYS_RAM_EVENT_folderAdded);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    EXPECT_FALSE(testMyObject->bNeedFlush);
    EXPECT_EQ(testMyObject->LastRegWriteAt, 0);

    if (testMyObject) {
        free(testMyObject);
        testMyObject = nullptr;
    }
}

TEST_F(CcspPsmTestFixture, SysRamNotify_SaveInProgress) {

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)AnscAllocateMemory(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);

    testMyObject->bNoSave = FALSE;
    testMyObject->bSaveInProgress = TRUE;
    testMyObject->LastRegWriteAt = 0;
    testMyObject->bNeedFlush = FALSE;

    ANSC_STATUS status = PsmSysroSysRamNotify((ANSC_HANDLE)testMyObject, nullptr, SYS_RAM_EVENT_folderAdded);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    EXPECT_FALSE(testMyObject->bNeedFlush);
    EXPECT_EQ(testMyObject->LastRegWriteAt, 0);

    if (testMyObject) {
        free(testMyObject);
        testMyObject = nullptr;
    }
}

TEST_F(CcspPsmTestFixture, SysRamNotify_FolderEvents) {

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    PPSM_SYS_REGISTRY_OBJECT testMyObject = (PPSM_SYS_REGISTRY_OBJECT)AnscAllocateMemory(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(testMyObject, nullptr);

    ULONG events[] = {
        SYS_RAM_EVENT_folderAdded,
        SYS_RAM_EVENT_folderUpdated,
        SYS_RAM_EVENT_folderDeleted,
        SYS_RAM_EVENT_folderCleared
    };

    for (ULONG event : events) {
        testMyObject->bNoSave = FALSE;
        testMyObject->bSaveInProgress = FALSE;
        testMyObject->LastRegWriteAt = 0;
        testMyObject->bNeedFlush = FALSE;

        EXPECT_CALL(*g_usertimeMock, UserGetTickInSeconds2())
            .WillOnce(Return(12345));

        ANSC_STATUS status = PsmSysroSysRamNotify((ANSC_HANDLE)testMyObject, nullptr, event);
        EXPECT_EQ(status, ANSC_STATUS_SUCCESS);
        EXPECT_TRUE(testMyObject->bNeedFlush);
        EXPECT_EQ(testMyObject->LastRegWriteAt, 12345);
    }

    if (testMyObject) {
        free(testMyObject);
        testMyObject = nullptr;
    }
}
