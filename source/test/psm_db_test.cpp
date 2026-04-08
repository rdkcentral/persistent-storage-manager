/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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

/*
 * psm_db_test.cpp — Unit tests for:
 *   - psm_db_init()              (tasks 4.3)
 *   - PSM_Get_Record_Value2()    (task  4.1)
 *   - PSM_Set_Record_Value2()    (task  4.2)
 *
 * These tests use a temporary SQLite database file under /tmp to avoid
 * touching /nvram.  The test binary overrides PSM_DB_PATH at build time
 * (see Makefile.am) or via the TEST_PSM_DB_PATH environment variable.
 */

#include <gtest/gtest.h>
#include <sqlite3.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

extern "C" {
#include "ccsp_base_api.h"
#include "ccsp_psm_helper.h"
}

/* Path used by all tests — must match what the helpers open */
#define TEST_DB_PATH "/tmp/psm_test.db"

/* -------------------------------------------------------------------------
 * Minimal bus_handle stub that provides malloc/free callbacks
 * needed by the SQLite helper allocation paths.
 * -------------------------------------------------------------------------*/
struct TestBusInfo {
    char component_id[256];
    CCSP_MESSAGE_BUS_MALLOC  mallocfunc;
    CCSP_MESSAGE_BUS_FREE    freefunc;
};

static void *test_malloc(size_t n) { return malloc(n); }
static void  test_free(void *p)    { free(p); }

/* -------------------------------------------------------------------------
 * Fixture: sets up a fresh temporary database before each test case.
 * -------------------------------------------------------------------------*/
class PsmDbTest : public ::testing::Test {
protected:
    void *bus_handle;
    TestBusInfo bus_info;

    void SetUp() override {
        /* Remove any leftover test database */
        unlink(TEST_DB_PATH);

        /* Seed the database with the same schema psm_db_init() creates */
        sqlite3 *db = nullptr;
        ASSERT_EQ(SQLITE_OK, sqlite3_open_v2(TEST_DB_PATH, &db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL));

        const char *ddl =
            "PRAGMA journal_mode=WAL;"
            "PRAGMA synchronous=NORMAL;"
            "CREATE TABLE IF NOT EXISTS psm_records ("
            "  name  TEXT    NOT NULL PRIMARY KEY,"
            "  type  INTEGER NOT NULL,"
            "  value TEXT    NOT NULL"
            ");"
            "CREATE INDEX IF NOT EXISTS idx_psm_records_name ON psm_records(name);";
        char *errmsg = nullptr;
        ASSERT_EQ(SQLITE_OK, sqlite3_exec(db, ddl, nullptr, nullptr, &errmsg))
            << "Schema setup failed: " << (errmsg ? errmsg : "");
        sqlite3_free(errmsg);
        sqlite3_close(db);

        /* Initialise stub bus handle */
        memset(&bus_info, 0, sizeof(bus_info));
        bus_info.mallocfunc = test_malloc;
        bus_info.freefunc   = test_free;
        bus_handle = &bus_info;
    }

    void TearDown() override {
        unlink(TEST_DB_PATH);
    }

    /* Helper: insert a record directly into the test database */
    void SeedRecord(const char *name, int type, const char *value) {
        sqlite3 *db = nullptr;
        ASSERT_EQ(SQLITE_OK, sqlite3_open(TEST_DB_PATH, &db));
        sqlite3_stmt *stmt = nullptr;
        ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
            "INSERT OR REPLACE INTO psm_records(name,type,value) VALUES(?1,?2,?3);",
            -1, &stmt, nullptr));
        sqlite3_bind_text(stmt, 1, name,  -1, SQLITE_STATIC);
        sqlite3_bind_int (stmt, 2, type);
        sqlite3_bind_text(stmt, 3, value, -1, SQLITE_STATIC);
        ASSERT_EQ(SQLITE_DONE, sqlite3_step(stmt));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
};

/* =========================================================================
 * Task 4.1 — PSM_Get_Record_Value2 tests
 * =========================================================================*/

/* 4.1a: Record exists → CCSP_SUCCESS, correct value returned */
TEST_F(PsmDbTest, GetRecordExists)
{
    SeedRecord("Device.Test.Param", ccsp_string, "hello_world");

    char *pValue = nullptr;
    unsigned int type = 0;
    int ret = PSM_Get_Record_Value2(bus_handle, "eRT.", "Device.Test.Param", &type, &pValue);

    EXPECT_EQ(CCSP_SUCCESS, ret);
    EXPECT_NE(nullptr, pValue);
    EXPECT_STREQ("hello_world", pValue);
    EXPECT_EQ((unsigned int)ccsp_string, type);

    free(pValue);
}

/* 4.1b: Record missing → non-success, *pValue stays NULL */
TEST_F(PsmDbTest, GetRecordMissing)
{
    char *pValue = nullptr;
    unsigned int type = 0;
    int ret = PSM_Get_Record_Value2(bus_handle, "eRT.", "Device.Missing.Param", &type, &pValue);

    EXPECT_NE(CCSP_SUCCESS, ret);
    EXPECT_EQ(nullptr, pValue);
}

/* 4.1c: Database unavailable → non-success */
TEST_F(PsmDbTest, GetRecordDatabaseUnavailable)
{
    /* Remove the database after setup */
    unlink(TEST_DB_PATH);

    char *pValue = nullptr;
    unsigned int type = 0;
    int ret = PSM_Get_Record_Value2(bus_handle, "eRT.", "Device.Test.Param", &type, &pValue);

    EXPECT_NE(CCSP_SUCCESS, ret);
    EXPECT_EQ(nullptr, pValue);
}

/* 4.1d: pRecordName is NULL → CCSP_CR_ERR_INVALID_PARAM */
TEST_F(PsmDbTest, GetRecordNullName)
{
    char *pValue = nullptr;
    unsigned int type = 0;
    int ret = PSM_Get_Record_Value2(bus_handle, "eRT.", nullptr, &type, &pValue);

    EXPECT_EQ(CCSP_CR_ERR_INVALID_PARAM, ret);
    EXPECT_EQ(nullptr, pValue);
}

/* =========================================================================
 * Task 4.2 — PSM_Set_Record_Value2 tests
 * =========================================================================*/

/* 4.2a: Insert new record → CCSP_SUCCESS, row appears in DB */
TEST_F(PsmDbTest, SetRecordInsertNew)
{
    int ret = PSM_Set_Record_Value2(bus_handle, "eRT.",
                                    "Device.New.Param", ccsp_string, "new_value");
    EXPECT_EQ(CCSP_SUCCESS, ret);

    /* Verify with a direct read-back */
    char *pValue = nullptr;
    unsigned int type = 0;
    ret = PSM_Get_Record_Value2(bus_handle, "eRT.", "Device.New.Param", &type, &pValue);
    EXPECT_EQ(CCSP_SUCCESS, ret);
    EXPECT_STREQ("new_value", pValue);
    free(pValue);
}

/* 4.2b: Update existing record → CCSP_SUCCESS, value updated */
TEST_F(PsmDbTest, SetRecordUpdateExisting)
{
    SeedRecord("Device.Update.Param", ccsp_string, "old_value");

    int ret = PSM_Set_Record_Value2(bus_handle, "eRT.",
                                    "Device.Update.Param", ccsp_string, "new_value");
    EXPECT_EQ(CCSP_SUCCESS, ret);

    char *pValue = nullptr;
    unsigned int type = 0;
    ret = PSM_Get_Record_Value2(bus_handle, "eRT.", "Device.Update.Param", &type, &pValue);
    EXPECT_EQ(CCSP_SUCCESS, ret);
    EXPECT_STREQ("new_value", pValue);
    free(pValue);
}

/* 4.2c: Invalid boolean value → CCSP_CR_ERR_INVALID_PARAM, no write */
TEST_F(PsmDbTest, SetRecordInvalidBoolean)
{
    int ret = PSM_Set_Record_Value2(bus_handle, "eRT.",
                                    "Device.Bool.Param", ccsp_boolean, "yes");
    EXPECT_EQ(CCSP_CR_ERR_INVALID_PARAM, ret);

    /* Confirm the record was NOT written */
    char *pValue = nullptr;
    unsigned int type = 0;
    ret = PSM_Get_Record_Value2(bus_handle, "eRT.", "Device.Bool.Param", &type, &pValue);
    EXPECT_NE(CCSP_SUCCESS, ret);
    EXPECT_EQ(nullptr, pValue);
}

/* 4.2d: Valid boolean "1" → CCSP_SUCCESS */
TEST_F(PsmDbTest, SetRecordValidBooleanTrue)
{
    int ret = PSM_Set_Record_Value2(bus_handle, "eRT.",
                                    "Device.Bool.True", ccsp_boolean, PSM_TRUE);
    EXPECT_EQ(CCSP_SUCCESS, ret);
}

/* 4.2e: Valid boolean "0" → CCSP_SUCCESS */
TEST_F(PsmDbTest, SetRecordValidBooleanFalse)
{
    int ret = PSM_Set_Record_Value2(bus_handle, "eRT.",
                                    "Device.Bool.False", ccsp_boolean, PSM_FALSE);
    EXPECT_EQ(CCSP_SUCCESS, ret);
}

/* 4.2f: Database unavailable → non-success */
TEST_F(PsmDbTest, SetRecordDatabaseUnavailable)
{
    unlink(TEST_DB_PATH);

    int ret = PSM_Set_Record_Value2(bus_handle, "eRT.",
                                    "Device.NoDb.Param", ccsp_string, "val");
    EXPECT_NE(CCSP_SUCCESS, ret);
}

/* =========================================================================
 * Task 4.3 — psm_db_init() tests
 * =========================================================================*/

#include "psm_db_init.h"

class PsmDbInitTest : public ::testing::Test {
protected:
    void SetUp() override  { unlink(TEST_DB_PATH); }
    void TearDown() override { unlink(TEST_DB_PATH); }
};

/* 4.3a: Fresh DB creation — psm_db_init() succeeds and file exists */
TEST_F(PsmDbInitTest, FreshDatabaseCreation)
{
    int ret = psm_db_init();
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, access(TEST_DB_PATH, F_OK));

    /* Verify schema created correctly */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(TEST_DB_PATH, &db));
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='psm_records';",
        -1, &stmt, nullptr);
    ASSERT_EQ(SQLITE_OK, rc);
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* 4.3b: Existing valid DB — psm_db_init() succeeds, data intact */
TEST_F(PsmDbInitTest, ExistingValidDatabase)
{
    /* First init to create */
    ASSERT_EQ(0, psm_db_init());

    /* Seed a record */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(TEST_DB_PATH, &db));
    sqlite3_exec(db,
        "INSERT INTO psm_records(name,type,value) VALUES('persistent.key',1,'persistent.val');",
        nullptr, nullptr, nullptr);
    sqlite3_close(db);

    /* Second init — should leave data intact */
    EXPECT_EQ(0, psm_db_init());

    db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(TEST_DB_PATH, &db));
    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT value FROM psm_records WHERE name='persistent.key';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    EXPECT_STREQ("persistent.val", (const char *)sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* 4.3c: Corrupt DB — psm_db_init() recovers (deletes and recreates) */
TEST_F(PsmDbInitTest, CorruptDatabaseRecovery)
{
    /* Write garbage to the DB file to corrupt it */
    FILE *f = fopen(TEST_DB_PATH, "wb");
    ASSERT_NE(nullptr, f);
    const char garbage[] = "not a sqlite database at all !!##$$";
    fwrite(garbage, 1, sizeof(garbage), f);
    fclose(f);

    /* psm_db_init() must successfully recover */
    int ret = psm_db_init();
    EXPECT_EQ(0, ret);
    EXPECT_EQ(0, access(TEST_DB_PATH, F_OK));

    /* Verify the recovered schema is valid */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(TEST_DB_PATH, &db));
    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='psm_records';",
        -1, &stmt, nullptr));
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
