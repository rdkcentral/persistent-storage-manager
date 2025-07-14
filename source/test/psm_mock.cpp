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

#include <gmock/gmock.h>
#include "psm_mock.h"

SafecLibMock * g_safecLibMock = NULL;
UserTimeMock * g_usertimeMock = NULL;
SyscfgMock * g_syscfgMock = NULL;
AnscMemoryMock * g_anscMemoryMock = NULL;
TraceMock * g_traceMock = NULL;
cjsonMock *g_cjsonMock = NULL;
SysInfoRepositoryMock * g_sysInfoRepositoryMock = NULL;
AnscCoMock* g_anscCoMock = NULL;
AnscCryptoMock* g_anscCryptoMock = NULL;
AnscFileIOMock* g_anscFileIOMock = NULL;
AnscTimerSchedulerMock * g_anscTimerSchedulerMock = NULL;
AnscXmlMock* g_anscXmlMock = NULL;
FileIOMock * g_fileIOMock = NULL;

CcspPsmTestFixture::CcspPsmTestFixture()
{
    g_safecLibMock = new SafecLibMock;
    g_usertimeMock = new UserTimeMock;
    g_syscfgMock = new SyscfgMock;
    g_anscMemoryMock = new AnscMemoryMock;
    g_traceMock = new TraceMock;
    g_cjsonMock = new cjsonMock;
    g_sysInfoRepositoryMock = new SysInfoRepositoryMock;
    g_anscCoMock = new AnscCoMock;
    g_anscCryptoMock = new AnscCryptoMock;
    g_anscFileIOMock = new AnscFileIOMock;
    g_anscTimerSchedulerMock = new AnscTimerSchedulerMock;
    g_anscXmlMock = new AnscXmlMock;
    g_fileIOMock = new FileIOMock;
}

CcspPsmTestFixture::~CcspPsmTestFixture()
{
    delete g_safecLibMock;
    delete g_usertimeMock;
    delete g_syscfgMock;
    delete g_anscMemoryMock;
    delete g_traceMock;
    delete g_cjsonMock;
    delete g_sysInfoRepositoryMock;
    delete g_anscCoMock;
    delete g_anscCryptoMock;
    delete g_anscFileIOMock;
    delete g_anscTimerSchedulerMock;
    delete g_anscXmlMock;
    delete g_fileIOMock;

    g_safecLibMock = nullptr;
    g_usertimeMock = nullptr;
    g_syscfgMock = nullptr;
    g_anscMemoryMock = nullptr;
    g_traceMock = nullptr;
    g_cjsonMock = nullptr;
    g_sysInfoRepositoryMock = nullptr;
    g_anscCoMock = nullptr;
    g_anscCryptoMock = nullptr;
    g_anscFileIOMock = nullptr;
    g_anscTimerSchedulerMock = nullptr;
    g_anscXmlMock = nullptr;
    g_fileIOMock = nullptr;
}

void CcspPsmTestFixture::SetUp() {}
void CcspPsmTestFixture::TearDown() {}
void CcspPsmTestFixture::TestBody() {}

// end of file
