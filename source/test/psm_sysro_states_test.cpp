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

using ::testing::_;
using ::testing::StrEq;
using ::testing::Return;

//Test case for PsmSysroGetPsmSseIf
TEST_F(CcspPsmTestFixture, GetPsmSseIf) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));
    
     ANSC_HANDLE expectedHandle = reinterpret_cast<ANSC_HANDLE>(0xDEADBEEF);
     test_Object->hPsmSseIf = expectedHandle;

    ANSC_HANDLE result = PsmSysroGetPsmSseIf((ANSC_HANDLE)test_Object);
    EXPECT_EQ(result, expectedHandle);

    free(test_Object);
}

//Test case for PsmSysroSetPsmSseIf
TEST_F(CcspPsmTestFixture, SetPsmSseIf) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Define a mock handle value
    ANSC_HANDLE new_hPsmSseIf = reinterpret_cast<ANSC_HANDLE>(0xDEADBEEF);

    // Call the function to set the hPsmSseIf member
    ANSC_STATUS status = PsmSysroSetPsmSseIf((ANSC_HANDLE)test_Object, new_hPsmSseIf);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Check if the member was set correctly
    EXPECT_EQ(test_Object->hPsmSseIf, new_hPsmSseIf);

    // Clean up
    free(test_Object);
}

//Test case for PsmSysroGetPsmFileLoader
TEST_F(CcspPsmTestFixture, GetPsmFileLoader) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    ANSC_HANDLE expectedHandle = reinterpret_cast<ANSC_HANDLE>(0xBEEFDEAD);
    test_Object->hPsmFileLoader = expectedHandle;

    ANSC_HANDLE result = PsmSysroGetPsmFileLoader((ANSC_HANDLE)test_Object);
    EXPECT_EQ(result, expectedHandle);

    // Clean up
    free(test_Object);
}

//Test case for PsmSysroGetSysInfoRepository
TEST_F(CcspPsmTestFixture, GetSysInfoRepository) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    ANSC_HANDLE expectedHandle = reinterpret_cast<ANSC_HANDLE>(0xCAFEBABE);
    test_Object->hSysInfoRepository = expectedHandle;

    ANSC_HANDLE result = PsmSysroGetSysInfoRepository((ANSC_HANDLE)test_Object);
    EXPECT_EQ(result, expectedHandle);

    // Clean up
    free(test_Object);
}

//Test case for PsmSysroGetProperty
TEST_F(CcspPsmTestFixture, GetProperty) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Set up known values in the Property structure
    strncpy(test_Object->Property.SysFilePath, PSM_DEF_SYS_FILE_PATH, sizeof(test_Object->Property.SysFilePath) - 1);
    strncpy(test_Object->Property.DefFileName, PSM_DEF_DEF_FILE_NAME, sizeof(test_Object->Property.DefFileName) - 1);
    strncpy(test_Object->Property.CurFileName, PSM_DEF_CUR_FILE_NAME, sizeof(test_Object->Property.CurFileName) - 1);
    strncpy(test_Object->Property.BakFileName, PSM_DEF_BAK_FILE_NAME, sizeof(test_Object->Property.BakFileName) - 1);
    strncpy(test_Object->Property.TmpFileName, PSM_DEF_TMP_FILE_NAME, sizeof(test_Object->Property.TmpFileName) - 1);

    // Allocate memory for the property to be retrieved
    PPSM_SYS_REGISTRY_PROPERTY retrievedProperty = (PPSM_SYS_REGISTRY_PROPERTY)malloc(sizeof(PSM_SYS_REGISTRY_PROPERTY));
    if (retrievedProperty == nullptr) {
        free(test_Object);
        return;
    }

    // Call the function to get the Property
    ANSC_STATUS status = PsmSysroGetProperty((ANSC_HANDLE)test_Object, (ANSC_HANDLE)retrievedProperty);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Check if the retrieved property matches the expected values
    EXPECT_STREQ(retrievedProperty->SysFilePath, PSM_DEF_SYS_FILE_PATH);
    EXPECT_STREQ(retrievedProperty->DefFileName, PSM_DEF_DEF_FILE_NAME);
    EXPECT_STREQ(retrievedProperty->CurFileName, PSM_DEF_CUR_FILE_NAME);
    EXPECT_STREQ(retrievedProperty->BakFileName, PSM_DEF_BAK_FILE_NAME);
    EXPECT_STREQ(retrievedProperty->TmpFileName, PSM_DEF_TMP_FILE_NAME);

    // Clean up
    free(retrievedProperty);
    free(test_Object);
}


// Test cases for PsmSysroSetProperty
TEST_F(CcspPsmTestFixture, SetProperty_Success) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Allocate and initialize the PSM_SYS_REGISTRY_PROPERTY
    PPSM_SYS_REGISTRY_PROPERTY property = (PPSM_SYS_REGISTRY_PROPERTY)malloc(sizeof(PSM_SYS_REGISTRY_PROPERTY));
    ASSERT_NE(property, nullptr) << "Memory allocation failed";
    memset(property, 0, sizeof(PSM_SYS_REGISTRY_PROPERTY));

    // Initialize property values
    strncpy(property->SysFilePath, PSM_DEF_SYS_FILE_PATH, PSM_SYS_FILE_PATH_SIZE);
    strncpy(property->DefFileName, PSM_DEF_DEF_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->CurFileName, PSM_DEF_CUR_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->BakFileName,"", PSM_SYS_FILE_NAME_SIZE);  
    strncpy(property->TmpFileName,"", PSM_SYS_FILE_NAME_SIZE);  

    // Expect calls to the mock functions
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_BAK_FILE_NAME), _))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_TMP_FILE_NAME), _))
        .WillOnce(Return(EOK));

    // Call the function to set the Property
    ANSC_STATUS status = PsmSysroSetProperty((ANSC_HANDLE)test_Object, (ANSC_HANDLE)property);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(property);
    free(test_Object);
}

TEST_F(CcspPsmTestFixture, SetProperty_Failure_BakFileName) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Allocate and initialize the PSM_SYS_REGISTRY_PROPERTY
    PPSM_SYS_REGISTRY_PROPERTY property = (PPSM_SYS_REGISTRY_PROPERTY)malloc(sizeof(PSM_SYS_REGISTRY_PROPERTY));
    ASSERT_NE(property, nullptr) << "Memory allocation failed";
    memset(property, 0, sizeof(PSM_SYS_REGISTRY_PROPERTY));

    // Initialize property values
    strncpy(property->SysFilePath, PSM_DEF_SYS_FILE_PATH, PSM_SYS_FILE_PATH_SIZE);
    strncpy(property->DefFileName, PSM_DEF_DEF_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->CurFileName, PSM_DEF_CUR_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->BakFileName,"", PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->TmpFileName,"", PSM_SYS_FILE_NAME_SIZE);  

    // Expect strcpy_s to return ESNULLP for BakFileName
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,StrEq(PSM_DEF_BAK_FILE_NAME),_))
        .WillOnce(Return(ESNULLP)); 

    // Call the function to set the property
    ANSC_STATUS status = PsmSysroSetProperty((ANSC_HANDLE)test_Object, (ANSC_HANDLE)property);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(property);
    free(test_Object);
}


TEST_F(CcspPsmTestFixture, SetProperty_Failure_TmpFileName) {

    // Allocate and initialize the PSM_SYS_REGISTRY_OBJECT
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Allocate and initialize the PSM_SYS_REGISTRY_PROPERTY
    PPSM_SYS_REGISTRY_PROPERTY property = (PPSM_SYS_REGISTRY_PROPERTY)malloc(sizeof(PSM_SYS_REGISTRY_PROPERTY));
    ASSERT_NE(property, nullptr) << "Memory allocation failed";
    memset(property, 0, sizeof(PSM_SYS_REGISTRY_PROPERTY));

    // Initialize property values
    strncpy(property->SysFilePath, PSM_DEF_SYS_FILE_PATH, PSM_SYS_FILE_PATH_SIZE);
    strncpy(property->DefFileName, PSM_DEF_DEF_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->CurFileName, PSM_DEF_CUR_FILE_NAME, PSM_SYS_FILE_NAME_SIZE);
    strncpy(property->BakFileName, "", PSM_SYS_FILE_NAME_SIZE);  
    strncpy(property->TmpFileName, "", PSM_SYS_FILE_NAME_SIZE); 

    // Expect strcpy_s to succeed for BakFileName and fail for TmpFileName
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,StrEq(PSM_DEF_BAK_FILE_NAME),_))
        .WillOnce(Return(EOK));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_,_,StrEq(PSM_DEF_TMP_FILE_NAME),_))
        .WillOnce(Return(ESNULLP)); 

    // Call the function to set the property
    ANSC_STATUS status = PsmSysroSetProperty((ANSC_HANDLE)test_Object, (ANSC_HANDLE)property);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(property);
    free(test_Object);
}

//Test cases for PsmSysroResetProperty
TEST_F(CcspPsmTestFixture, ResetProperty_Success) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, _, _)).WillRepeatedly(Return(EOK));

    // Call the function to reset the property
    ANSC_STATUS status = PsmSysroResetProperty((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(test_Object);
}

// Failure test case for SysFilePath
TEST_F(CcspPsmTestFixture, ResetProperty_Failure_SysFilePath) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_SYS_FILE_PATH), _)).WillOnce(Return(ESNULLP));

    // Call the function to reset the property
    ANSC_STATUS status = PsmSysroResetProperty((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

// Failure test case for DefFileName
TEST_F(CcspPsmTestFixture, ResetProperty_Failure_DefFileName) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_SYS_FILE_PATH), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_DEF_FILE_NAME), _)).WillOnce(Return(ESNULLP));

    // Call the function to reset the property
    ANSC_STATUS status = PsmSysroResetProperty((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

// Failure test case for CurFileName
TEST_F(CcspPsmTestFixture, ResetProperty_Failure_CurFileName) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_SYS_FILE_PATH), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_DEF_FILE_NAME), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_CUR_FILE_NAME), _)).WillOnce(Return(ESNULLP));

    // Call the function to reset the property
    ANSC_STATUS status = PsmSysroResetProperty((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

// Failure test case for BakFileName
TEST_F(CcspPsmTestFixture, ResetProperty_Failure_BakFileName) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_SYS_FILE_PATH), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_DEF_FILE_NAME), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_CUR_FILE_NAME), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_BAK_FILE_NAME), _)).WillOnce(Return(ESNULLP));

    // Call the function to reset the property
    ANSC_STATUS status = PsmSysroResetProperty((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

// Failure test case for TmpFileName
TEST_F(CcspPsmTestFixture, ResetProperty_Failure_TmpFileName) {
    
    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_SYS_FILE_PATH), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_DEF_FILE_NAME), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_CUR_FILE_NAME), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_BAK_FILE_NAME), _)).WillOnce(Return(EOK));
    EXPECT_CALL(*g_safecLibMock, _strcpy_s_chk(_, _, StrEq(PSM_DEF_TMP_FILE_NAME), _)).WillOnce(Return(ESNULLP));

    // Call the function to reset the property
    ANSC_STATUS status = PsmSysroResetProperty((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_FAILURE);

    // Clean up
    free(test_Object);
}

// Test case for PsmSysroReset
TEST_F(CcspPsmTestFixture, Reset) {

    PPSM_SYS_REGISTRY_OBJECT test_Object = (PPSM_SYS_REGISTRY_OBJECT)malloc(sizeof(PSM_SYS_REGISTRY_OBJECT));
    ASSERT_NE(test_Object, nullptr) << "Memory allocation failed";
    memset(test_Object, 0, sizeof(PSM_SYS_REGISTRY_OBJECT));

    // Call the function to reset the object
    ANSC_STATUS status = PsmSysroReset((ANSC_HANDLE)test_Object);
    EXPECT_EQ(status, ANSC_STATUS_SUCCESS);

    // Clean up
    free(test_Object);
}