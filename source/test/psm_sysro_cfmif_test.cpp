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
extern AnscFileIOMock* g_anscFileIOMock;

using ::testing::_;
using ::testing::StrEq;
using ::testing::Return;
using ::testing::Eq;
using ::testing::Pointee;

// Stub functions
static ULONG MockTestRegFileSuccess(ANSC_HANDLE hThisObject, void* pCfgBuffer, ULONG ulCfgSize) {
    return PSM_FLO_ERROR_CODE_noError;
}

// Actaul test cases
TEST_F(CcspPsmTestFixture, ReadCurConfig_SuccessCase) 
{
    // Initialize test object
   PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName));
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName));
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName));

    // Allocate and initialize the file loader object with the MockTestRegFileSuccess function
    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)calloc(1, sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

for (int i = 0; i < 3; ++i) 
{
        switch (i) {
            case 0: {

                 EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
                 .WillRepeatedly(Return(EOK));

                 EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
                 .WillRepeatedly(Return(EOK));
             
                break;
            }
            case 1: {

                 EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
                 .WillRepeatedly(Return(EOK));

                 EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
                 .WillRepeatedly(Return(EOK));

                 EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_,_,_))
                .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));
                break;
            }
            case 2: {

                EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
               .WillRepeatedly(Return(EOK)); 

                EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
               .WillRepeatedly(Return(EOK));

                EXPECT_CALL(*g_anscFileIOMock,AnscCopyFile(_,_,_))
                .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));
                break;
            }
            default:
                break;
        }
}  
   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_,_,_))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillOnce(Return(1024));

    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));


    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadCurConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(test_Object);
    free(pCfgBuffer); 
    free(psmFileLoader);
}


//Test cases for PsmSysroCfmReadDefConfig
TEST_F(CcspPsmTestFixture, ReadDefConfig_Success)
{
    ULONG expectedFileSize = 1024; 

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_READ, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillOnce(Return(expectedFileSize));

    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);
    EXPECT_EQ(ulCfgSize, expectedFileSize);
    EXPECT_NE(pCfgBuffer, nullptr);

    // Clean up
    free(pCfgBuffer);
    free(test_Object);
}


TEST_F(CcspPsmTestFixture, ReadDefConfig_FailureStrcpy)
{
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP)); 
   
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ReadDefConfig_FailureStrcat)
{
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP)); 

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ReadDefConfig_FailureOpenFile)
{
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_READ, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return((ANSC_HANDLE)NULL)); 

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ReadDefConfig_FailureGetFileSize)
{
    
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_READ, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillOnce(Return((ULONG)-1)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ReadDefConfig_FailureNegativeFileSize)
{

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_READ, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillOnce(Return((ULONG)-1));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ReadDefConfig_FileSizeZero)
{
   
    ULONG expectedFileSize = 0;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_READ, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillOnce(Return(expectedFileSize));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);

    // Variables to hold output
    void* pCfgBuffer = nullptr;
    ULONG ulCfgSize = 0;

    ANSC_STATUS status = PsmSysroCfmReadDefConfig((ANSC_HANDLE)test_Object, &pCfgBuffer, &ulCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);
    EXPECT_EQ(ulCfgSize, 0);
    EXPECT_EQ(pCfgBuffer, nullptr);

    // Clean up
    free(test_Object);
}

//Test cases for PsmSysroCfmSaveCurConfig
TEST_F(CcspPsmTestFixture, SaveCurConfig_Success)
{
    ULONG expectedCfgSize = 1024;
    void* expectedCfgBuffer = malloc(expectedCfgSize); 

    ASSERT_NE(expectedCfgBuffer, nullptr);
    memset(expectedCfgBuffer, 0xAB, expectedCfgSize); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_WRITE | ANSC_FILE_MODE_TRUNC, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, expectedCfgBuffer, Pointee(Eq(expectedCfgSize))))
        .WillOnce(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS)); 


   EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(EOK)); 

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(EOK)); 

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);

    ANSC_STATUS status = PsmSysroCfmSaveCurConfig((ANSC_HANDLE)test_Object, expectedCfgBuffer, expectedCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(expectedCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, SaveCurConfig_FirstStrcpyS_Fail)
{
 
    ULONG expectedCfgSize = 1024; 
    void* expectedCfgBuffer = malloc(expectedCfgSize); 

    ASSERT_NE(expectedCfgBuffer, nullptr);

    memset(expectedCfgBuffer, 0xAB, expectedCfgSize);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP)); 

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);

    ANSC_STATUS status = PsmSysroCfmSaveCurConfig((ANSC_HANDLE)test_Object, expectedCfgBuffer, expectedCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(expectedCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, SaveCurConfig_FirstStrcatS_Fail)
{
   
    ULONG expectedCfgSize = 1024; 
    void* expectedCfgBuffer = malloc(expectedCfgSize); 

    ASSERT_NE(expectedCfgBuffer, nullptr);

    memset(expectedCfgBuffer, 0xAB, expectedCfgSize); 

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP));     

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);

    ANSC_STATUS status = PsmSysroCfmSaveCurConfig((ANSC_HANDLE)test_Object, expectedCfgBuffer, expectedCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(expectedCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, SaveCurConfig_SecondStrcpyS_Fail)
{

    ULONG expectedCfgSize = 1024; 
    void* expectedCfgBuffer = malloc(expectedCfgSize); 
    ASSERT_NE(expectedCfgBuffer, nullptr);
    memset(expectedCfgBuffer, 0xAB, expectedCfgSize);     

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(ESNULLP));

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);


    ANSC_STATUS status = PsmSysroCfmSaveCurConfig((ANSC_HANDLE)test_Object, expectedCfgBuffer, expectedCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(expectedCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, SaveCurConfig_SecondStrcatS_Fail)
{
    
    ULONG expectedCfgSize = 1024; 
    void* expectedCfgBuffer = malloc(expectedCfgSize); 

    ASSERT_NE(expectedCfgBuffer, nullptr);
    memset(expectedCfgBuffer, 0xAB, expectedCfgSize); 

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(ESNULLP)); 
      
    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);


    ANSC_STATUS status = PsmSysroCfmSaveCurConfig((ANSC_HANDLE)test_Object, expectedCfgBuffer, expectedCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(expectedCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, SaveCurConfig_AnscOpenFile_Fail)
{
    ULONG expectedCfgSize = 1024; 
    void* expectedCfgBuffer = malloc(expectedCfgSize); 

    ASSERT_NE(expectedCfgBuffer, nullptr);

    memset(expectedCfgBuffer, 0xAB, expectedCfgSize); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_WRITE | ANSC_FILE_MODE_TRUNC, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(NULL))); 

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(EOK)); 
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK))
        .WillOnce(Return(EOK)); 

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);

    ANSC_STATUS status = PsmSysroCfmSaveCurConfig((ANSC_HANDLE)test_Object, expectedCfgBuffer, expectedCfgSize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(expectedCfgBuffer);
    free(test_Object);
}

