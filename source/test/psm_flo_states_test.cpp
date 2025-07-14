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

// Test case for PsmFloGetPsmCfmIf
TEST_F(CcspPsmTestFixture, PsmFloGetPsmCfmIf) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));

    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;
    ANSC_HANDLE hPsmCfmIf = (ANSC_HANDLE)4;
    pMyObject->hPsmCfmIf = hPsmCfmIf;

    ANSC_HANDLE result = PsmFloGetPsmCfmIf(hObject);
    ASSERT_EQ(result, hPsmCfmIf);

    PsmFloRemove(hObject);
}

// Test case for PsmFloSetPsmCfmIf
TEST_F(CcspPsmTestFixture, PsmFloSetPsmCfmIf) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    ANSC_HANDLE hPsmCfmIf = (ANSC_HANDLE)4;
    ANSC_STATUS status = PsmFloSetPsmCfmIf(hObject, hPsmCfmIf);
    ASSERT_EQ(status, ANSC_STATUS_SUCCESS);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;
    ASSERT_EQ(pMyObject->hPsmCfmIf, hPsmCfmIf);

    PsmFloRemove(hObject);
}

// Test case for PsmFloGetSysIraIf
TEST_F(CcspPsmTestFixture, PsmFloGetSysIraIf) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;
    ANSC_HANDLE hSysIraIf = (ANSC_HANDLE)5;
    pMyObject->hSysIraIf = hSysIraIf;

    ANSC_HANDLE result = PsmFloGetSysIraIf(hObject);
    ASSERT_EQ(result, hSysIraIf);

    PsmFloRemove(hObject);
}

// Test case for PsmFloSetSysIraIf
TEST_F(CcspPsmTestFixture, PsmFloSetSysIraIf) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    ANSC_HANDLE hSysIraIf = (ANSC_HANDLE)5;
    ANSC_STATUS status = PsmFloSetSysIraIf(hObject, hSysIraIf);
    ASSERT_EQ(status, ANSC_STATUS_SUCCESS);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;
    ASSERT_EQ(pMyObject->hSysIraIf, hSysIraIf);

    PsmFloRemove(hObject);
}

// Test case for PsmFloGetProperty
TEST_F(CcspPsmTestFixture, PsmFloGetProperty) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;
    PSM_FILE_LOADER_PROPERTY property = {};
    pMyObject->Property = property;

    PSM_FILE_LOADER_PROPERTY retrievedProperty;
    ANSC_STATUS status = PsmFloGetProperty(hObject, &retrievedProperty);
    ASSERT_EQ(status, ANSC_STATUS_SUCCESS);
    ASSERT_EQ(memcmp(&retrievedProperty, &property, sizeof(PSM_FILE_LOADER_PROPERTY)), 0);

    PsmFloRemove(hObject);
}

// Test case for PsmFloSetProperty
TEST_F(CcspPsmTestFixture, PsmFloSetProperty) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    PSM_FILE_LOADER_PROPERTY property = {};
    ANSC_STATUS status = PsmFloSetProperty(hObject, &property);
    ASSERT_EQ(status, ANSC_STATUS_SUCCESS);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;
    ASSERT_EQ(memcmp(&pMyObject->Property, &property, sizeof(PSM_FILE_LOADER_PROPERTY)), 0);

    PsmFloRemove(hObject);
}

// Test case for PsmFloResetProperty
TEST_F(CcspPsmTestFixture, PsmFloResetProperty) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    PsmFloResetProperty(hObject);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;

    PsmFloRemove(hObject);
}

// Test case for PsmFloReset
TEST_F(CcspPsmTestFixture, PsmFloReset) {
    EXPECT_CALL(*g_anscCoMock, AnscCoEnrollObjects(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoInitialize(::testing::_))
        .WillOnce(Return(0));
    EXPECT_CALL(*g_anscCoMock, AnscCoRemove(::testing::_))
        .WillOnce(Return(0));
        
    ANSC_HANDLE hObject = PsmFloCreate((ANSC_HANDLE)1, (ANSC_HANDLE)2, (ANSC_HANDLE)3);
    ASSERT_NE(hObject, nullptr);

    PsmFloReset(hObject);

    PPSM_FILE_LOADER_OBJECT pMyObject = (PPSM_FILE_LOADER_OBJECT)hObject;

    PsmFloRemove(hObject);
}