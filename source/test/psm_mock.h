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

#ifndef PSM_MOCK_H
#define PSM_MOCK_H

#include "gtest/gtest.h"

#include <mocks/mock_safec_lib.h>
#include <mocks/mock_usertime.h>
#include <mocks/mock_syscfg.h>
#include <mocks/mock_ansc_memory.h>
#include <mocks/mock_trace.h>
#include <mocks/mock_cJSON.h>
#include <mocks/mock_SysInfoRepository.h>
#include <mocks/mock_ansc_co.h>
#include <mocks/mock_ansc_crypto.h>
#include <mocks/mock_ansc_file_io.h>
#include <mocks/mock_ansc_timer_scheduler.h>
#include <mocks/mock_ansc_xml.h>
#include <mocks/mock_file_io.h>

class CcspPsmTestFixture : public ::testing::Test {
  protected:
        SafecLibMock mockedSafecLibMock;
        UserTimeMock mockedUserTime;
        SyscfgMock mockedSyscfg;
        AnscMemoryMock mockedAnscMemory;
        TraceMock mockedTrace;
        cjsonMock mockedCjson;
        SysInfoRepositoryMock mockedSysInfoRepository;
        AnscCoMock mockedAnscCo;
        AnscCryptoMock mockedAnscCrypto;
        AnscFileIOMock mockedAnscFileIO;
        AnscTimerSchedulerMock mockedAnscTimerScheduler;
        AnscXmlMock mockedAnscXml;
        FileIOMock mockedFileIO;

        CcspPsmTestFixture();
        virtual ~CcspPsmTestFixture();
        virtual void SetUp() override;
        virtual void TearDown() override;

        void TestBody() override;
};

#endif // PSM_MOCK_H

