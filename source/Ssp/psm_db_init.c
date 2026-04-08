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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sqlite3.h>

#include "ccsp_trace.h"
#include "psm_db_init.h"

/* -----------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------- */

static int psm_db_apply_pragmas(sqlite3 *db)
{
    const char *pragmas =
        "PRAGMA journal_mode=WAL;"
        "PRAGMA synchronous=NORMAL;"
        "PRAGMA page_size=4096;";

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, pragmas, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: PRAGMA setup failed: %s\n",
                        errmsg ? errmsg : "unknown error"));
        sqlite3_free(errmsg);
        return -1;
    }
    return 0;
}

static int psm_db_create_schema(sqlite3 *db)
{
    const char *ddl =
        "CREATE TABLE IF NOT EXISTS psm_records ("
        "  name  TEXT    NOT NULL PRIMARY KEY,"
        "  type  INTEGER NOT NULL,"
        "  value TEXT    NOT NULL"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_psm_records_name"
        "  ON psm_records (name);";

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, ddl, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: schema creation failed: %s\n",
                        errmsg ? errmsg : "unknown error"));
        sqlite3_free(errmsg);
        return -1;
    }
    return 0;
}

static int psm_db_integrity_check(sqlite3 *db)
{
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, "PRAGMA integrity_check;", -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: integrity_check prepare failed: %s\n",
                        sqlite3_errmsg(db)));
        return -1;
    }

    int ok = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char *result = sqlite3_column_text(stmt, 0);
        if (result && strcmp((const char *)result, "ok") == 0)
        {
            ok = 0;
        }
        else
        {
            CcspTraceError(("psm_db_init: integrity_check result: %s\n",
                            result ? (const char *)result : "NULL"));
        }
    }
    sqlite3_finalize(stmt);
    return ok;
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

int psm_db_init(void)
{
    sqlite3 *db = NULL;
    int rc;
    int attempt;

    for (attempt = 0; attempt < 2; attempt++)
    {
        rc = sqlite3_open_v2(PSM_DB_PATH, &db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                             SQLITE_OPEN_FULLMUTEX,
                             NULL);
        if (rc != SQLITE_OK)
        {
            CcspTraceError(("psm_db_init: sqlite3_open_v2 failed (%s): %s\n",
                            PSM_DB_PATH, sqlite3_errmsg(db)));
            if (db)
            {
                sqlite3_close(db);
                db = NULL;
            }
            return -1;
        }

        if (psm_db_apply_pragmas(db) != 0)
        {
            sqlite3_close(db);
            return -1;
        }

        if (psm_db_create_schema(db) != 0)
        {
            sqlite3_close(db);
            return -1;
        }

        /* Integrity check — only on first attempt */
        if (attempt == 0 && psm_db_integrity_check(db) != 0)
        {
            CcspTraceError(("psm_db_init: database corrupt, attempting recovery\n"));
            sqlite3_close(db);
            db = NULL;

            if (unlink(PSM_DB_PATH) != 0)
            {
                CcspTraceError(("psm_db_init: failed to remove corrupt database: %s\n",
                                PSM_DB_PATH));
                return -1;
            }
            CcspTraceWarning(("psm_db_init: corrupt database removed, recreating\n"));
            continue; /* retry */
        }

        break; /* success */
    }

    sqlite3_close(db);

    /* Set file permissions: owner rw, group r (0640) */
    if (chmod(PSM_DB_PATH, S_IRUSR | S_IWUSR | S_IRGRP) != 0)
    {
        CcspTraceWarning(("psm_db_init: chmod(%s) failed\n", PSM_DB_PATH));
        /* Non-fatal: continue */
    }

    CcspTraceInfo(("psm_db_init: database ready at %s\n", PSM_DB_PATH));
    return 0;
}
