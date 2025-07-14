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
extern SysInfoRepositoryMock * g_sysInfoRepositoryMock;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::IsNull;

TEST_F(CcspPsmTestFixture, CreateSysRegistry) {

    EXPECT_CALL(*g_sysInfoRepositoryMock, SysCreateInfoRepository(_, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return((ANSC_HANDLE)NULL));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    ANSC_HANDLE result = PsmCreateSysRegistry( (ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3 );
    EXPECT_NE(result, (ANSC_HANDLE)NULL) << "PsmCreateSysRegistry should return the handle from PsmSysroCreate";
}

