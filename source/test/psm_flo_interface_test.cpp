/*
* If not stated otherwise in this file or this component's LICENSE file the
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
#include "psm_mock.h"

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

extern "C"
{
#include "psm_flo_global.h"
}
extern AnscCoMock* g_anscCoMock;

// Test case for PsmCreateFileLoader
TEST_F(CcspPsmTestFixture, CreateFileLoader) {
    ANSC_HANDLE hContainerContext = (ANSC_HANDLE)1;
    ANSC_HANDLE hOwnerContext = (ANSC_HANDLE)2;
    ANSC_HANDLE hAnscReserved = (ANSC_HANDLE)3;

    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(::testing::Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(::testing::Return(0));

    ANSC_HANDLE result = PsmCreateFileLoader(hContainerContext, hOwnerContext, hAnscReserved);
    ASSERT_NE(result, nullptr);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)result;
    ASSERT_EQ(pMyObject->hContainerContext, hContainerContext);
    ASSERT_EQ(pMyObject->hOwnerContext, hOwnerContext);
    ASSERT_EQ(pMyObject->Oid, PSM_FILE_LOADER_OID);

    PsmFloRemove(result);
}
