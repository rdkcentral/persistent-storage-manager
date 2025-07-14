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
    #include "psm_flo_global.h"
}

extern AnscXmlMock* g_anscXmlMock;
extern AnscMemoryMock * g_anscMemoryMock;

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::IsNull;

// Stub functions for mocking
static ANSC_STATUS MockCloseFolder ( ANSC_HANDLE hThisObject, ANSC_HANDLE hCurFolder)  {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_HANDLE MockOpenFolder (ANSC_HANDLE hThisObject, ANSC_HANDLE hCurFolder,  char* pSubFolderName)  {
    return (ANSC_HANDLE)123;
}

static ANSC_HANDLE MockOpenFolder_Fail (ANSC_HANDLE hThisObject, ANSC_HANDLE hCurFolder,  char* pSubFolderName)  {
    return (ANSC_HANDLE)NULL;
}

static ANSC_STATUS MockReadCurConfig(ANSC_HANDLE hThisObject, void** ppCfgBuffer, ULONG* pulCfgSize) {

    const char* testData = "mocked_data";
    ULONG dataSize = strlen(testData) + 1;

    *ppCfgBuffer = malloc(dataSize);
    if (*ppCfgBuffer == nullptr) {
        return ANSC_STATUS_FAILURE;
    }

    memcpy(*ppCfgBuffer, testData, dataSize);

    *pulCfgSize = dataSize;

    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockReadCurConfig_Fail(ANSC_HANDLE hThisObject, void** ppCfgBuffer, ULONG* pulCfgSize){
    return ANSC_STATUS_RESOURCES;
}

static ANSC_STATUS MockRemove(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockAcqWriteAccess(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockAcqWriteAccess_Fail(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_FAILURE;
}

static ANSC_STATUS MockRelWriteAccess(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockEnumSubFolder (
        ANSC_HANDLE                 hThisObject,
        ANSC_HANDLE                 hCurFolder,
        ULONG                       ulIndex,
        char*                       pSubFolderName,
        PULONG                      pulNameSize )
{
    if (pulNameSize)
    {
        *pulNameSize = 10;
    }
    if (pSubFolderName)
    {
        strcpy(pSubFolderName, "TestFolder");
    }
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockQueryFolder
    (
        ANSC_HANDLE                 hThisObject,
        ANSC_HANDLE                 hCurFolder,
        PULONG                      pulTimestamp,
        PULONG                      pulPermission,
        PULONG                      pulFolderType,
        PULONG                      pulSubFolderCount,
        PULONG                      pulRecordCount,
        PULONG                      pulContentType,
        PANSC_HANDLE                phRenderAttr
    )
    {
        return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockGetRecordByIndex(
    ANSC_HANDLE hThisObject,
    ANSC_HANDLE hCurFolder,
    ULONG nIndex,
    PCHAR pName,
    PULONG pNameSize,
    PULONG pRecType,
    PULONG pContentType,
    PULONG pAccess,
    PUCHAR* ppData,
    PULONG pDataSize
) {
    strcpy(pName, "TestRecord");
    *pNameSize = strlen("TestRecord");
    *pRecType = SYS_REP_RECORD_TYPE_ASTR;
    *pContentType = SYS_RECORD_CONTENT_DEFAULT;
    *pAccess = SYS_RFO_ACCESS_MODE_ALL;
    *ppData = (PUCHAR)AnscAllocateMemory(10);
    strcpy((char*)*ppData, "TestValue");
    *pDataSize = strlen("TestValue");
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS MockSaveCurConfig
    (
        ANSC_HANDLE                 hThisObject,
        void*                       pCfgBuffer,
        ULONG                       ulCfgSize
    )
{  
    return ANSC_STATUS_SUCCESS;
}

// Actual test cases
TEST_F(CcspPsmTestFixture, PsmFloTestRegFile_Success) {
    const char* testXmlContent = "<root><test>data</test></root>";
    ULONG testSize = strlen(testXmlContent);

    PANSC_XML_DOM_NODE_OBJECT  testMyObject = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomParseString(_, _, testSize))
        .WillOnce(::testing::DoAll(
            ::testing::SetArgPointee<1>(const_cast<char*>(testXmlContent)),
            Return(reinterpret_cast<ANSC_HANDLE>(testMyObject))
        ));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeRemove(_))
        .Times(AnyNumber())
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    ULONG result = PsmFloTestRegFile(testMyObject, (void*)testXmlContent, testSize);
    EXPECT_EQ(result, PSM_FLO_ERROR_CODE_noError);

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, PsmFloTestRegFile_InvalidNode) {
    const char* testXmlContent = "<root><test>data</test></root>";
    ULONG testSize = strlen(testXmlContent);

    PANSC_XML_DOM_NODE_OBJECT  testMyObject = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomParseString(_, _, testSize))
        .WillOnce(Return((ANSC_HANDLE)NULL));

    ULONG result = PsmFloTestRegFile(testMyObject, (void*)testXmlContent, testSize);
    EXPECT_EQ(result, PSM_FLO_ERROR_CODE_invalidFormat);

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, PsmFloLoadRegFile_Success) {
    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PPSM_CFM_INTERFACE testCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testCfmIf, nullptr);
    memset(testCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT testXmlNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testXmlNode, nullptr);
    memset(testXmlNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testCfmIf->ReadCurConfig = MockReadCurConfig;
    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testCfmIf;

    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->CloseFolder = MockCloseFolder;
    testXmlNode->Remove = MockRemove;

    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;

    testMyObject->bActive = true;

    const char* testBackData = "someData";
    ULONG testSize = strlen(testBackData);
    
    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomParseString(_, _, _))
        .WillOnce(::testing::DoAll(
            ::testing::SetArgPointee<1>(const_cast<char*>(testBackData)),
            Return(reinterpret_cast<ANSC_HANDLE>(testXmlNode))
        ));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeGetHeadChild(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return((ANSC_HANDLE)NULL));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(1);

    ANSC_STATUS result = PsmFloLoadRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_SUCCESS);

    free(testMyObject);
    free(testCfmIf);
    free(testIraIf);
    free(testXmlNode);
}

TEST_F(CcspPsmTestFixture, PsmFloLoadRegFile_bActiveIsFalse) {
    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    testMyObject->bActive = false;

    ANSC_STATUS result = PsmFloLoadRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_NOT_READY);

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, PsmFloLoadRegFile_OpenFolderFail) {
    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    testIraIf->OpenFolder = MockOpenFolder_Fail;
    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    testMyObject->bActive = true;

    ANSC_STATUS result = PsmFloLoadRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_ACCESS_DENIED);

    free(testMyObject);
    free(testIraIf);
}

TEST_F(CcspPsmTestFixture, PsmFloLoadRegFile_ParseStringFail) {
    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PPSM_CFM_INTERFACE testCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testCfmIf, nullptr);
    memset(testCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT testXmlNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testXmlNode, nullptr);
    memset(testXmlNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testCfmIf->ReadCurConfig = MockReadCurConfig;
    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testCfmIf;

    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->CloseFolder = MockCloseFolder;

    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;

    testMyObject->bActive = true;

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomParseString(_, _, _))
        .WillOnce(Return((ANSC_HANDLE)NULL));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(1);

    ANSC_STATUS result = PsmFloLoadRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_RESOURCES);

    free(testMyObject);
    free(testCfmIf);
    free(testIraIf);
    free(testXmlNode);
}

TEST_F(CcspPsmTestFixture, PsmFloLoadRegFile_ReadCurConfigFail) {
    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PPSM_CFM_INTERFACE testCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testCfmIf, nullptr);
    memset(testCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    testCfmIf->ReadCurConfig = MockReadCurConfig_Fail;
    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testCfmIf;

    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->CloseFolder = MockCloseFolder;

    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;

    testMyObject->bActive = true;

    ANSC_STATUS result = PsmFloLoadRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_RESOURCES);

    free(testMyObject);
    free(testCfmIf);
    free(testIraIf);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_bActiveIsFalse) {

    // If bActive is false, the function returns early
    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    testMyObject->bActive = false;

    ANSC_STATUS result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_NOT_READY);

    free(testMyObject);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_AcqWriteAccessFail) {

    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)calloc(1, sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    testMyObject->bActive = true;
    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    
    testIraIf->hOwnerContext = (ANSC_HANDLE)12;
    testIraIf->AcqWriteAccess = MockAcqWriteAccess_Fail;

    ULONG result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_FAILURE);

    free(testMyObject);
    free(testIraIf);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_OpenFolderFail) {

    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));
    
    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    testIraIf->hOwnerContext = (ANSC_HANDLE)123;

    testIraIf->AcqWriteAccess = MockAcqWriteAccess;
    testIraIf->OpenFolder = MockOpenFolder_Fail;
    testIraIf->RelWriteAccess = MockRelWriteAccess;

    testMyObject->bActive = true;

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());
    
    ANSC_STATUS result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_ACCESS_DENIED);

    free(testMyObject);
    free(testIraIf);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_AnscCreateXmlDomNode_Failure) {

    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT  testXmlNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testXmlNode, nullptr);
    memset(testXmlNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    testIraIf->hOwnerContext = (ANSC_HANDLE)1234;

    testIraIf->AcqWriteAccess = MockAcqWriteAccess;
    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->RelWriteAccess = MockRelWriteAccess;
    testIraIf->CloseFolder = MockCloseFolder;

    testMyObject->bActive = true;

    EXPECT_CALL(*g_anscXmlMock, AnscCreateXmlDomNode(_))
        .WillOnce(Return((ANSC_HANDLE)NULL));

    ANSC_STATUS result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_RESOURCES);

    free(testMyObject);
    free(testIraIf);
    free(testXmlNode);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_Success) {

    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    PPSM_CFM_INTERFACE testCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testCfmIf, nullptr);
    memset(testCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT  testXmlNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testXmlNode, nullptr);
    memset(testXmlNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testIraIf->hOwnerContext = (ANSC_HANDLE)1234;
    
    testIraIf->AcqWriteAccess = MockAcqWriteAccess;
    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->CloseFolder = MockCloseFolder;
    testIraIf->RelWriteAccess = MockRelWriteAccess;
    testIraIf -> QueryFolder = MockQueryFolder;
    testIraIf -> EnumSubFolder = MockEnumSubFolder;
    testIraIf->GetRecordByIndex = MockGetRecordByIndex;
    testIraIf->OpenFolder = MockOpenFolder;
    testCfmIf->SaveCurConfig = MockSaveCurConfig;
    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testCfmIf;

    testMyObject->bActive = true;

    ANSC_HANDLE hSysFolder = reinterpret_cast<ANSC_HANDLE>(2);

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrUlong(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscXmlMock, AnscCreateXmlDomNode(_))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(testXmlNode)));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetName(Eq(reinterpret_cast<ANSC_HANDLE>(testXmlNode)), StrEq("Provision")))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeGetEncodedSize(_))
        .WillOnce(Return((ULONG)10));

    
    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeEncode(_, _, _))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeRemove(_))
        .Times(AnyNumber())
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    ANSC_STATUS result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_SUCCESS);
    
    free(testMyObject);
    free(testCfmIf);
    free(testIraIf);
    free(testXmlNode);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_pDataIsZero) {

    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    PPSM_CFM_INTERFACE testCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testCfmIf, nullptr);
    memset(testCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT  testXmlNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testXmlNode, nullptr);
    memset(testXmlNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testIraIf->hOwnerContext = (ANSC_HANDLE)1234;
    
    testIraIf->AcqWriteAccess = MockAcqWriteAccess;
    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->CloseFolder = MockCloseFolder;
    testIraIf->RelWriteAccess = MockRelWriteAccess;
    testIraIf -> QueryFolder = MockQueryFolder;
    testIraIf -> EnumSubFolder = MockEnumSubFolder;
    testIraIf->GetRecordByIndex = MockGetRecordByIndex;
    testIraIf->OpenFolder = MockOpenFolder;
    testCfmIf->SaveCurConfig = MockSaveCurConfig;
    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testCfmIf;

    testMyObject->bActive = true;

    ANSC_HANDLE hSysFolder = reinterpret_cast<ANSC_HANDLE>(2);

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrUlong(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscXmlMock, AnscCreateXmlDomNode(_))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(testXmlNode)));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetName(Eq(reinterpret_cast<ANSC_HANDLE>(testXmlNode)), StrEq("Provision")))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeGetEncodedSize(_))
        .WillOnce(Return((ULONG)0));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeRemove(_))
        .Times(AnyNumber())
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    ANSC_STATUS result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_FAILURE);
    
    free(testMyObject);
    free(testCfmIf);
    free(testIraIf);
    free(testXmlNode);
}

TEST_F(CcspPsmTestFixture, PsmFloSaveRegFile_AnscXmlDomNodeEncode_Fail) {

    PPSM_FILE_LOADER_OBJECT testMyObject = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(testMyObject, nullptr);
    memset(testMyObject, 0, sizeof(PSM_FILE_LOADER_OBJECT));

    PSYS_IRA_INTERFACE testIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testIraIf, nullptr);
    memset(testIraIf, 0, sizeof(SYS_IRA_INTERFACE));

    PPSM_CFM_INTERFACE testCfmIf = (PPSM_CFM_INTERFACE)malloc(sizeof(PSM_CFM_INTERFACE));
    ASSERT_NE(testCfmIf, nullptr);
    memset(testCfmIf, 0, sizeof(PSM_CFM_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT  testXmlNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testXmlNode, nullptr);
    memset(testXmlNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testIraIf->hOwnerContext = (ANSC_HANDLE)1234;
    
    testIraIf->AcqWriteAccess = MockAcqWriteAccess;
    testIraIf->OpenFolder = MockOpenFolder;
    testIraIf->CloseFolder = MockCloseFolder;
    testIraIf->RelWriteAccess = MockRelWriteAccess;
    testIraIf -> QueryFolder = MockQueryFolder;
    testIraIf -> EnumSubFolder = MockEnumSubFolder;
    testIraIf->GetRecordByIndex = MockGetRecordByIndex;
    testIraIf->OpenFolder = MockOpenFolder;
    testCfmIf->SaveCurConfig = MockSaveCurConfig;
    testMyObject->hSysIraIf = (ANSC_HANDLE)testIraIf;
    testMyObject->hPsmCfmIf = (ANSC_HANDLE)testCfmIf;

    testMyObject->bActive = true;

    ANSC_HANDLE hSysFolder = reinterpret_cast<ANSC_HANDLE>(2);

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrUlong(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscXmlMock, AnscCreateXmlDomNode(_))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(testXmlNode)));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetName(Eq(reinterpret_cast<ANSC_HANDLE>(testXmlNode)), StrEq("Provision")))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));


    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeGetEncodedSize(_))
        .WillOnce(Return((ULONG)10));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(AnyNumber());

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeRemove(_))
        .Times(AnyNumber())
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeEncode(_, _, _))
        .WillOnce(Return(ANSC_STATUS_FAILURE));

    ANSC_STATUS result = PsmFloSaveRegFile((ANSC_HANDLE)testMyObject);
    EXPECT_EQ(result, ANSC_STATUS_FAILURE);
    
    free(testMyObject);
    free(testCfmIf);
    free(testIraIf);
    free(testXmlNode);
}