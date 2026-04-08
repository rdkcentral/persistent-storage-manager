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
 * psm_db_test.cpp — Unit tests for psm_db_init() (task 4.3).
 *
 * These tests exercise the boot-time database initialization routine directly.
 * The PSM SQLite Get/Set helper tests (tasks 4.1–4.2) live in the
 * common-library test binary where ccsp_base_api.c is compiled in full.
 *
 * All tests use PSM_DB_PATH which is overridden to /tmp/psm_test.db
 * via the -DPSM_DB_PATH build flag so /nvram is never touched.
 */

#include <gtest/gtest.h>
#include <sqlite3.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

extern "C" {
#include "psm_db_init.h"
}

/* =========================================================================
 * Fixture: removes the test database around each test
 * =========================================================================*/
class PsmDbInitTest : public ::testing::Test {
protected:
    void SetUp()    override { unlink(PSM_DB_PATH); }
    void TearDown() override { unlink(PSM_DB_PATH); }
};

/* -------------------------------------------------------------------------
 * 4.3a: Fresh database — psm_db_init() creates the file and schema
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, FreshDatabaseCreation)
{
    ASSERT_EQ(0, psm_db_init());

    /* File must exist */
    EXPECT_EQ(0, access(PSM_DB_PATH, F_OK));

    /* Schema must contain the psm_records table */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='psm_records';",
        -1, &stmt, nullptr));
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(stmt))
        << "psm_records table should exist after fresh init";

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * 4.3a cont: Schema must include the index too
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, FreshDatabaseIndexExists)
{
    ASSERT_EQ(0, psm_db_init());

    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT name FROM sqlite_master WHERE type='index' AND name='idx_psm_records_name';",
        -1, &stmt, nullptr));
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(stmt))
        << "idx_psm_records_name index should exist after fresh init";

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * 4.3b: Existing valid database — psm_db_init() succeeds, data is preserved
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, ExistingValidDatabasePreservesData)
{
    /* First init */
    ASSERT_EQ(0, psm_db_init());

    /* Insert a sentinel record */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));
    char *errmsg = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_exec(db,
        "INSERT INTO psm_records(name,type,value) VALUES('Device.Persistent.Key',1,'keep_me');",
        nullptr, nullptr, &errmsg))
        << "Seed insert failed: " << (errmsg ? errmsg : "");
    sqlite3_free(errmsg);
    sqlite3_close(db);

    /* Second init — must not wipe data */
    EXPECT_EQ(0, psm_db_init());

    db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));
    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT value FROM psm_records WHERE name='Device.Persistent.Key';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt))
        << "Existing record must survive re-init";
    EXPECT_STREQ("keep_me", (const char *)sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * 4.3b cont: Re-init is idempotent — no duplicate tables/indices
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, ExistingValidDatabaseNoDuplicateSchema)
{
    ASSERT_EQ(0, psm_db_init());
    /* A second call must also succeed (IF NOT EXISTS guards work) */
    EXPECT_EQ(0, psm_db_init());
}

/* -------------------------------------------------------------------------
 * 4.3c: Corrupt database — psm_db_init() recovers (deletes and recreates)
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, CorruptDatabaseRecovery)
{
    /* Write binary garbage as the "database" */
    FILE *f = fopen(PSM_DB_PATH, "wb");
    ASSERT_NE(nullptr, f);
    const char garbage[] = "\x00\x01\x02not-a-sqlite-db!!!";
    fwrite(garbage, 1, sizeof(garbage) - 1, f);
    fclose(f);

    /* Must recover successfully */
    ASSERT_EQ(0, psm_db_init());

    /* Resulting file must be a valid SQLite database with the correct schema */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='psm_records';",
        -1, &stmt, nullptr));
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(stmt))
        << "psm_records table should exist after corruption recovery";

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * 4.3c cont: Recovered database — WAL mode is applied correctly
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, CorruptDatabaseRecoveryPragmasApplied)
{
    FILE *f = fopen(PSM_DB_PATH, "wb");
    ASSERT_NE(nullptr, f);
    fputs("garbage", f);
    fclose(f);

    ASSERT_EQ(0, psm_db_init());

    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "PRAGMA journal_mode;", -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    EXPECT_STREQ("wal", (const char *)sqlite3_column_text(stmt, 0));

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * 4.3 extra: File permissions are set to 0640 after creation
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbInitTest, FilePermissionsAfterCreation)
{
    ASSERT_EQ(0, psm_db_init());

    struct stat st;
    ASSERT_EQ(0, stat(PSM_DB_PATH, &st));
    mode_t perms = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    EXPECT_EQ((mode_t)(S_IRUSR | S_IWUSR | S_IRGRP), perms)
        << "Expected 0640 permissions on database file";
}

/* =========================================================================
 * Fixture: migration tests — clean up both DB and XML around each test
 * =========================================================================*/
class PsmDbMigrateTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        unlink(PSM_DB_PATH);
        unlink(PSM_XML_CONFIG_PATH);
    }
    void TearDown() override
    {
        unlink(PSM_DB_PATH);
        unlink(PSM_XML_CONFIG_PATH);
    }

    /* Helper: write a minimal PSM XML to PSM_XML_CONFIG_PATH */
    static void WriteXml(const std::string &content)
    {
        FILE *fp = fopen(PSM_XML_CONFIG_PATH, "w");
        ASSERT_NE(nullptr, fp);
        fputs(content.c_str(), fp);
        fclose(fp);
    }
};

/* -------------------------------------------------------------------------
 * Migration: records from XML are imported into the SQLite DB
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbMigrateTest, RecordsImportedFromXml)
{
    WriteXml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<Provision>\n"
        "  <Record name=\"dmsb.devinfo.X_test.Foo\" type=\"astr\">hello</Record>\n"
        "  <Record name=\"dmsb.devinfo.X_test.Bar\" type=\"astr\">world</Record>\n"
        "</Provision>\n"
    );

    ASSERT_EQ(0, psm_db_init());

    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT value FROM psm_records WHERE name='dmsb.devinfo.X_test.Foo';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt))
        << "Migrated record dmsb.devinfo.X_test.Foo should exist in DB";
    EXPECT_STREQ("hello", (const char *)sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);

    stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT value FROM psm_records WHERE name='dmsb.devinfo.X_test.Bar';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt))
        << "Migrated record dmsb.devinfo.X_test.Bar should exist in DB";
    EXPECT_STREQ("world", (const char *)sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);

    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * Migration: contentType="bool" maps to ccsp_boolean (3)
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbMigrateTest, ContentTypeBoolMappedToCcspBoolean)
{
    WriteXml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<Provision>\n"
        "  <Record name=\"eRT.test.BoolParam\" type=\"astr\" contentType=\"bool\">true</Record>\n"
        "</Provision>\n"
    );

    ASSERT_EQ(0, psm_db_init());

    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT type FROM psm_records WHERE name='eRT.test.BoolParam';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    EXPECT_EQ(3, sqlite3_column_int(stmt, 0))
        << "contentType=bool should map to ccsp_boolean=3";
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * Migration: astr with no contentType maps to ccsp_string (0)
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbMigrateTest, TypeAstrMappedToCcspString)
{
    WriteXml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<Provision>\n"
        "  <Record name=\"eRT.test.StrParam\" type=\"astr\">somevalue</Record>\n"
        "</Provision>\n"
    );

    ASSERT_EQ(0, psm_db_init());

    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));

    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT type FROM psm_records WHERE name='eRT.test.StrParam';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    EXPECT_EQ(0, sqlite3_column_int(stmt, 0))
        << "type=astr with no contentType should map to ccsp_string=0";
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * Migration: pre-populated DB is NOT overwritten on second init
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbMigrateTest, ExistingDbSkipsMigration)
{
    /* Write an XML file with one record */
    WriteXml(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<Provision>\n"
        "  <Record name=\"eRT.test.XmlOnly\" type=\"astr\">from_xml</Record>\n"
        "</Provision>\n"
    );

    /* First init: XML is migrated */
    ASSERT_EQ(0, psm_db_init());

    /* Manually update the row to a different value via the DB */
    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));
    sqlite3_exec(db,
        "UPDATE psm_records SET value='overridden' WHERE name='eRT.test.XmlOnly';",
        nullptr, nullptr, nullptr);
    sqlite3_close(db);

    /* Second init: migration must be skipped because DB is non-empty */
    ASSERT_EQ(0, psm_db_init());

    db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));
    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT value FROM psm_records WHERE name='eRT.test.XmlOnly';",
        -1, &stmt, nullptr));
    ASSERT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    EXPECT_STREQ("overridden", (const char *)sqlite3_column_text(stmt, 0))
        << "Manually updated value must survive re-init (migration not repeated)";
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

/* -------------------------------------------------------------------------
 * Migration: missing XML file is non-fatal — DB is created but empty
 * -------------------------------------------------------------------------*/
TEST_F(PsmDbMigrateTest, MissingXmlFileNonFatal)
{
    /* Ensure XML file does not exist */
    unlink(PSM_XML_CONFIG_PATH);

    /* psm_db_init() must still succeed */
    EXPECT_EQ(0, psm_db_init());

    /* DB must exist with valid schema */
    EXPECT_EQ(0, access(PSM_DB_PATH, F_OK));

    sqlite3 *db = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_open(PSM_DB_PATH, &db));
    sqlite3_stmt *stmt = nullptr;
    ASSERT_EQ(SQLITE_OK, sqlite3_prepare_v2(db,
        "SELECT name FROM sqlite_master WHERE type='table' AND name='psm_records';",
        -1, &stmt, nullptr));
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

