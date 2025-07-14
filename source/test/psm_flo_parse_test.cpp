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

extern SafecLibMock * g_safecLibMock;
extern AnscXmlMock* g_anscXmlMock;
extern AnscMemoryMock * g_anscMemoryMock;

using ::testing::_;
using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::Eq;

// Stub Functions for Mocking
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

static ANSC_STATUS MockQueryFolder_PopulateWithOneRec (
    ANSC_HANDLE hThisObject,
    ANSC_HANDLE hCurFolder,
    PULONG pulTimestamp,
    PULONG pulPermission,
    PULONG pulFolderType,
    PULONG pulSubFolderCount,
    PULONG pulRecordCount,
    PULONG pulContentType,
    PANSC_HANDLE phRenderAttr)
{

    if (pulTimestamp)
    {
        *pulTimestamp = 12345;
    }
    if (pulPermission)
    {
        *pulPermission = SYS_RFO_ACCESS_MODE_READ;
    }
    if (pulFolderType)
    {
        *pulFolderType = 0;
    }
    if (pulSubFolderCount)
    {
        *pulSubFolderCount = 1;
    }
    if (pulRecordCount)
    {
        *pulRecordCount = 1;
    }
    if (pulContentType)
    {
        *pulContentType = SYS_FOLDER_CONTENT_CATEGORY;
    }
    if (phRenderAttr)
    {
        *phRenderAttr = nullptr;
    }

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

static ANSC_HANDLE MockOpenFolder
    (
        ANSC_HANDLE                 hThisObject,
        ANSC_HANDLE                 hCurFolder,
        char*                       pSubFolderName
    )
{
    return  (ANSC_HANDLE)NULL;
}

// Actual Test Cases
TEST_F(CcspPsmTestFixture, PsmSysFolderToXMLHandle_ZeroFolders) {
    PSYS_IRA_INTERFACE testSysIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testSysIraIf, nullptr);
    memset(testSysIraIf, 0, sizeof(PSYS_IRA_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT testNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testNode, nullptr);
    memset(testNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testSysIraIf -> hOwnerContext = reinterpret_cast<ANSC_HANDLE>(1);
    ANSC_HANDLE hSysFolder = reinterpret_cast<ANSC_HANDLE>(2);
    testSysIraIf -> QueryFolder = MockQueryFolder;

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrUlong(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

    ANSC_STATUS ret = PsmSysFolderToXMLHandle(testSysIraIf, (ANSC_HANDLE)hSysFolder, testNode);
    ASSERT_EQ(ret, ANSC_STATUS_SUCCESS);

    free(testSysIraIf);
    free(testNode);
}

TEST_F(CcspPsmTestFixture, PsmSysFolderToXMLHandle_OneFolder) {
    PSYS_IRA_INTERFACE testSysIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testSysIraIf, nullptr);
    memset(testSysIraIf, 0, sizeof(PSYS_IRA_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT testNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testNode, nullptr);
    memset(testNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testSysIraIf -> hOwnerContext = reinterpret_cast<ANSC_HANDLE>(1);
    ANSC_HANDLE hSysFolder = reinterpret_cast<ANSC_HANDLE>(2);
    testSysIraIf -> QueryFolder = MockQueryFolder_PopulateWithOneRec;
    testSysIraIf -> EnumSubFolder = MockEnumSubFolder;
    testSysIraIf->GetRecordByIndex = MockGetRecordByIndex;
    testSysIraIf->OpenFolder = MockOpenFolder;

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrUlong(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

    PANSC_XML_DOM_NODE_OBJECT TestChildNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(TestChildNode, nullptr);
    memset(TestChildNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    EXPECT_CALL(*g_anscXmlMock, AnscCreateXmlDomNode(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(TestChildNode)));

    
    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetName(_, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeAddChild(_, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(TestChildNode)));

    EXPECT_CALL (*g_anscXmlMock, AnscXmlDomNodeSetAttrString(_, _, _, _))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .Times(1);

    ANSC_STATUS ret = PsmSysFolderToXMLHandle(testSysIraIf, hSysFolder, testNode);
    ASSERT_EQ(ret, ANSC_STATUS_SUCCESS)<< "PsmSysFolderToXMLHandle failed";

    free(testSysIraIf);
    free(testNode);
}

TEST_F(CcspPsmTestFixture, PsmSysFolderToXMLHandle_OneFolder_XmlNodeFailure) {
    PSYS_IRA_INTERFACE testSysIraIf = (PSYS_IRA_INTERFACE)malloc(sizeof(SYS_IRA_INTERFACE));
    ASSERT_NE(testSysIraIf, nullptr);
    memset(testSysIraIf, 0, sizeof(PSYS_IRA_INTERFACE));

    PANSC_XML_DOM_NODE_OBJECT testNode = (PANSC_XML_DOM_NODE_OBJECT)malloc(sizeof(ANSC_XML_DOM_NODE_OBJECT));
    ASSERT_NE(testNode, nullptr);
    memset(testNode, 0, sizeof(ANSC_XML_DOM_NODE_OBJECT));

    testSysIraIf -> hOwnerContext = reinterpret_cast<ANSC_HANDLE>(1);
    ANSC_HANDLE hSysFolder = reinterpret_cast<ANSC_HANDLE>(2);
    testSysIraIf -> QueryFolder = MockQueryFolder_PopulateWithOneRec;
    testSysIraIf -> EnumSubFolder = MockEnumSubFolder;
    testSysIraIf->GetRecordByIndex = MockGetRecordByIndex;

    EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrUlong(_, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

     EXPECT_CALL(*g_anscXmlMock, AnscXmlDomNodeSetAttrString(_, _, _, _))
        .Times(1)
        .WillRepeatedly(Return(0));

    EXPECT_CALL(*g_anscXmlMock, AnscCreateXmlDomNode(_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(NULL)));

    ANSC_STATUS ret = PsmSysFolderToXMLHandle(testSysIraIf, hSysFolder, testNode);
    ASSERT_EQ(ret, ANSC_STATUS_RESOURCES);

    free(testSysIraIf);
    free(testNode);
}

TEST_F(CcspPsmTestFixture, PsmSysFolderFromXMLHandle_Test_EarlyFailure) {

    ANSC_HANDLE                 hSysIraIf = NULL;
    ANSC_HANDLE                 hSysFolder = NULL;
    ANSC_HANDLE                 hXMLHandle  = NULL;

    ANSC_STATUS ret = PsmSysFolderFromXMLHandle(hSysIraIf, hSysFolder, hXMLHandle);
    ASSERT_EQ(ret, ANSC_STATUS_FAILURE);
}
