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


#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "psm_mock.h"

extern "C" {
    #include "psm_sysro_global.h"
}

extern FileIOMock* g_fileIOMock;
extern AnscFileIOMock* g_anscFileIOMock;
extern SafecLibMock * g_safecLibMock;
extern AnscCryptoMock* g_anscCryptoMock;
extern AnscMemoryMock * g_anscMemoryMock;
extern UserTimeMock * g_usertimeMock;

using ::testing::_;
using ::testing::StrEq;
using ::testing::Return;

// Stub functions for mocking
static ANSC_STATUS MockStop(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS;
}

static ULONG MockTestRegFileSuccess(ANSC_HANDLE hThisObject, void* pCfgBuffer, ULONG ulCfgSize) {
    return PSM_FLO_ERROR_CODE_noError;
}

static ULONG MockTestRegFileFailure(ANSC_HANDLE hThisObject, void* pCfgBuffer, ULONG ulCfgSize) {
    return PSM_FLO_ERROR_CODE_invalidFormat;
}
static ANSC_STATUS MockCfmSaveCurConfigSuccess(ANSC_HANDLE hThisObject, void* pCfgBuffer, ULONG ulCfgSize) {
    return ANSC_STATUS_SUCCESS;
}

static ULONG MockSaveRegFileSuccess(ANSC_HANDLE hThisObject) {
    return ANSC_STATUS_SUCCESS; 
}

//Test cases for PsmSysroResetToFactoryDefault
TEST_F(CcspPsmTestFixture, ResetToFactoryDefault_Success) {
    
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, PSM_SYS_FILE_PATH_SIZE);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);

    PANSC_TIMER_DESCRIPTOR_OBJECT timerObject = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(timerObject, nullptr);
    memset(timerObject, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    timerObject->Stop = MockStop;
    test_Object->hRegTimerObj = timerObject;

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillRepeatedly(Return(0));

    // Call the function to reset to factory default
    ANSC_STATUS status = PsmSysroResetToFactoryDefault((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(timerObject);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ResetToFactoryDefault_Failure_FirstUnlink) {
    
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Initialize the properties
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, PSM_SYS_FILE_PATH_SIZE);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);

    PANSC_TIMER_DESCRIPTOR_OBJECT timerObject = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(timerObject, nullptr);
    memset(timerObject, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    timerObject->Stop = MockStop;
    test_Object->hRegTimerObj = timerObject;

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillOnce(Return(-1)).WillOnce(Return(-1));

    // Call the function to reset to factory default
    ANSC_STATUS status = PsmSysroResetToFactoryDefault((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);
 
    // Clean up
    free(timerObject);
    free(test_Object);
}


TEST_F(CcspPsmTestFixture, ResetToFactoryDefault_Failure_SecondUnlink) {
   
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, PSM_SYS_FILE_PATH_SIZE);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);

    PANSC_TIMER_DESCRIPTOR_OBJECT timerObject = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(timerObject, nullptr);
    memset(timerObject, 0, sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    timerObject->Stop = MockStop;
    test_Object->hRegTimerObj = timerObject;

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillOnce(Return(0)).WillOnce(Return(-1));

    // Call the function to reset to factory default
    ANSC_STATUS status = PsmSysroResetToFactoryDefault((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(timerObject);
    free(test_Object);
}

//Test cases for PsmSysroExportConfig
TEST_F(CcspPsmTestFixture, ExportConfig_Success) {
    char cfgBuffer[128] = {0};
    ULONG cfgSize = 128;
    char encryptKey[16] = {0};

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_,ANSC_FILE_MODE_RDWR, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1)));
    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));
    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));
    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillOnce(Return(cfgSize));

    ANSC_STATUS status = PsmSysroExportConfig((ANSC_HANDLE)test_Object, cfgBuffer, &cfgSize, encryptKey,sizeof(encryptKey));
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ExportConfig_strcpy_s_Failure) {

    char cfgBuffer[128] = {0};
    ULONG cfgSize = 128;
    char encryptKey[16] = {0};

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP)); 

    ANSC_STATUS status = PsmSysroExportConfig((ANSC_HANDLE)test_Object, cfgBuffer, &cfgSize, encryptKey, sizeof(encryptKey));
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ExportConfig_strcat_s_Failure) {
    char cfgBuffer[128] = {0};
    ULONG cfgSize = 128;
    char encryptKey[16] = {0};

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(ESNULLP)); 

    ANSC_STATUS status = PsmSysroExportConfig((ANSC_HANDLE)test_Object, cfgBuffer, &cfgSize, encryptKey, sizeof(encryptKey));
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ExportConfig_AnscOpenFile_Failure) {
    char cfgBuffer[128] = {0};
    ULONG cfgSize = 128;
    char encryptKey[16] = {0};

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_RDWR, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return((ANSC_HANDLE)NULL)); 

    ANSC_STATUS status = PsmSysroExportConfig((ANSC_HANDLE)test_Object, cfgBuffer, &cfgSize, encryptKey, sizeof(encryptKey));
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, ExportConfig_BlockSize_Failure) {
    char cfgBuffer[128] = {0};
    ULONG cfgSize = 130;  // Set to a non-multiple of ANSC_DES_BLOCK_SIZE (which is usually 8)
    char encryptKey[16] = {0};

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_, _, _, _))
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_RDWR, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1)));
    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));
    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    ANSC_STATUS status = PsmSysroExportConfig((ANSC_HANDLE)test_Object, cfgBuffer, &cfgSize, encryptKey, sizeof(encryptKey));
    EXPECT_EQ(status, ANSC_STATUS_BAD_SIZE);

    // Clean up
    free(test_Object);
}


//Test cases for PsmSysroGetConfigSize
TEST_F(CcspPsmTestFixture, GetConfigSize_Success)
{
    ULONG expectedFileSize = 1024;

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, ANSC_FILE_MODE_RDWR, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillOnce(Return(expectedFileSize));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillOnce(Return(ANSC_STATUS_SUCCESS));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    ULONG size = PsmSysroGetConfigSize((ANSC_HANDLE)test_Object);
    EXPECT_EQ(size, expectedFileSize);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, GetConfigSize_strcpy_s_Failure)
{
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))
        .WillOnce(Return(ESNULLP));

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    ULONG size = PsmSysroGetConfigSize((ANSC_HANDLE)test_Object);
    EXPECT_EQ(size, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, GetConfigSize_strcat_s_Failure)
{
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_))
        .WillOnce(Return(ESNULLP));  

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));


    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    ULONG size = PsmSysroGetConfigSize((ANSC_HANDLE)test_Object);
    EXPECT_EQ(size, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, GetConfigSize_AnscOpenFile_Failure)
{
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_,ANSC_FILE_MODE_RDWR, ANSC_FILE_TYPE_RDWR))
        .WillOnce(Return((ANSC_HANDLE)NULL)); 

    // Initialize test object
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set property values
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);

    ULONG size = PsmSysroGetConfigSize((ANSC_HANDLE)test_Object);
    EXPECT_EQ(size, 0UL);

    // Clean up
    free(test_Object);
}

//Test cases for PsmSysroImportConfig
TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_SuccessCase) {


    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, _, _))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));  

    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillRepeatedly(Return(0));          

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024);
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object,pCfgBuffer,1024,decryptKey,ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(pCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_BadSize) {
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1025); // Ensure ulCfgSize is not a multiple of ANSC_DES_BLOCK_SIZE
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1025);

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1025, decryptKey, ulKeySize); //sending 1025 as ulCfgSize for testing the failure case
    EXPECT_EQ(status, ANSC_STATUS_BAD_SIZE);

    free(pCfgBuffer);
    free(test_Object);
}


TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_Failure_strcpy) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  // checking the failure case of _strcpy_s_chk
        .WillOnce(Return(ESNULLP));

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}


TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_Failure_strcat) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) // checking the failure case of _strcat_s_chk
        .WillOnce(Return(ESNULLP));    


    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_TmpFileOpenFailure) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024);
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) 
        .WillOnce(Return(EOK));   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(NULL))); //checking the failure case of AnscOpenFile.

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}


TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_AnscWriteFileFailure) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) 
        .WillOnce(Return(EOK));   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, _,_))  //checking the failure case of AnscWriteFile
        .WillRepeatedly(Return(ANSC_STATUS_FAILURE));

     EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));        

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_SecondAnscOpenFileFailure) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) 
        .WillOnce(Return(EOK));   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, _,_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(NULL)));     //checking the failure case of 2nd AnscOpenFile.     

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_FileSizeNonPositive) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) 
        .WillOnce(Return(EOK));   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, _,_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillRepeatedly(Return(0)); // or use a negative value like -1  

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillRepeatedly(Return(0));           

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_AnscReadFileFailure) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    ULONG expectedFileSize = 1024;

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) 
        .WillOnce(Return(EOK));   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, _,_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillRepeatedly(Return(expectedFileSize));

    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillRepeatedly(Return(ANSC_STATUS_FAILURE));     // checking the failure case of AnscReadFile

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .WillRepeatedly(Return());  

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillRepeatedly(Return(0));    

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    free(pCfgBuffer);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, PsmSysroImportConfig_TestRegFileFailure) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    test_Object->CfmSaveCurConfig = MockCfmSaveCurConfigSuccess;

    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath));
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName));

    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader,0,sizeof(PPSM_FILE_LOADER_OBJECT));
    
    psmFileLoader->TestRegFile = MockTestRegFileFailure;  //passing MockTestRegFileFailure stub function for checking the failure case of psmFileLoader->TestRegFile.
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    unsigned char decryptKey[ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE] = {0};
    unsigned long ulKeySize = ANSC_DES_KEY_SIZE + ANSC_DES_IV_SIZE;

    // Initialize pCfgBuffer with some data
    void* pCfgBuffer = malloc(1024); 
    ASSERT_NE(pCfgBuffer, nullptr);
    memset(pCfgBuffer, 'A', 1024);

    ULONG expectedFileSize = 1024;

    EXPECT_CALL(*g_anscCryptoMock, AnscCryptoDesDecrypt(_, _, _, _, _))
        .WillRepeatedly(Return(1024));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,_,_))  
        .WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcat_s_chk(_,_,_,_)) 
        .WillOnce(Return(EOK));   
    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1)));

    EXPECT_CALL(*g_anscFileIOMock, AnscWriteFile(_, _,_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));

    EXPECT_CALL(*g_anscFileIOMock, AnscCloseFile(_))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS)); 

    EXPECT_CALL(*g_anscFileIOMock, AnscOpenFile(_, _, _))
        .WillRepeatedly(Return(reinterpret_cast<ANSC_HANDLE>(1))); 

    EXPECT_CALL(*g_anscFileIOMock, AnscGetFileSize(_))
        .WillRepeatedly(Return(expectedFileSize));

    EXPECT_CALL(*g_anscFileIOMock, AnscReadFile(_, _, _))
        .WillRepeatedly(Return(ANSC_STATUS_SUCCESS));     

    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(*g_fileIOMock, unlink(_)).WillRepeatedly(Return(0));    

    ANSC_STATUS status = PsmSysroImportConfig((ANSC_HANDLE)test_Object, pCfgBuffer, 1024, decryptKey, ulKeySize);
    EXPECT_EQ(status, ANSC_STATUS_BAD_MEDIA);

    free(pCfgBuffer);
    free(test_Object);
}

//Test case for PsmSysroSaveConfigToFlash
TEST_F(CcspPsmTestFixture, PsmSysroSaveConfigToFlash_SuccessCase) {
   
    EXPECT_CALL(*g_anscMemoryMock, AnscFreeMemoryOrig(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(*g_usertimeMock, UserGetTickInSeconds2())
        .WillOnce(Return(12345));

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr);
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Allocate and initialize the PSM_FILE_LOADER_OBJECT
    PPSM_FILE_LOADER_OBJECT psmFileLoader = (PPSM_FILE_LOADER_OBJECT)malloc(sizeof(PSM_FILE_LOADER_OBJECT));
    ASSERT_NE(psmFileLoader, nullptr);
    memset(psmFileLoader, 0, sizeof(PSM_FILE_LOADER_OBJECT));
    psmFileLoader->SaveRegFile = MockSaveRegFileSuccess;
    test_Object->hPsmFileLoader = (ANSC_HANDLE)psmFileLoader;

    // Allocate and initialize the PANSC_TIMER_DESCRIPTOR_OBJECT
    PANSC_TIMER_DESCRIPTOR_OBJECT pRegTimerObj = (PANSC_TIMER_DESCRIPTOR_OBJECT)malloc(sizeof(ANSC_TIMER_DESCRIPTOR_OBJECT));
    ASSERT_NE(pRegTimerObj, nullptr);
    pRegTimerObj->Stop = MockStop;
    test_Object->hRegTimerObj = (ANSC_HANDLE)pRegTimerObj;

    ANSC_STATUS status = PsmSysroSaveConfigToFlash((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(pRegTimerObj);
    free(psmFileLoader);
    free(test_Object);
}
