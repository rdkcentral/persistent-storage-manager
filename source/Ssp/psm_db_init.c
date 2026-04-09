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
        /* page_size must be set before journal_mode so it takes effect
         * when creating a brand-new database file. */
        "PRAGMA page_size=4096;"
        "PRAGMA journal_mode=WAL;"
        "PRAGMA synchronous=NORMAL;";

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, pragmas, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: PRAGMA setup failed: %s\n",
                        errmsg ? errmsg : "unknown error"));
        sqlite3_free(errmsg);
        return -1;
    }
    CcspTraceInfo(("psm_db_init: PRAGMA setup succeeded (WAL mode, NORMAL sync, 4096 page size)\n"));
    return 0;
}

static int psm_db_create_schema(sqlite3 *db)
{
    const char *ddl =
        "CREATE TABLE IF NOT EXISTS psm_records ("
        "  name  TEXT    NOT NULL PRIMARY KEY,"
        "  type  INTEGER NOT NULL,"
        "  value TEXT    NOT NULL"
        ");";

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, ddl, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: schema creation failed: %s\n",
                        errmsg ? errmsg : "unknown error"));
        sqlite3_free(errmsg);
        return -1;
    }
    CcspTraceInfo(("psm_db_init: psm_records table schema ready\n"));
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
            CcspTraceInfo(("psm_db_init: integrity_check passed\n"));
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
 * XML → SQLite migration helpers
 * --------------------------------------------------------------------- */

/*
 * Map PSM XML contentType (semantic) or type (storage) strings to
 * the CCSP dataType_e integer stored in the 'type' column.
 *
 * The contentType attribute takes precedence because it carries the
 * semantic CCSP type.  Unrecognised values default to ccsp_string (0).
 */
static int psm_xml_ccsp_type(const char *type_attr, const char *ctype_attr)
{
    /* Check contentType first (semantic CCSP type) */
    if (ctype_attr && *ctype_attr)
    {
        if (strcasecmp(ctype_attr, "bool")     == 0) return 3; /* ccsp_boolean     */
        if (strcasecmp(ctype_attr, "int")      == 0) return 1; /* ccsp_int         */
        if (strcasecmp(ctype_attr, "uint")     == 0) return 2; /* ccsp_unsignedInt */
        if (strcasecmp(ctype_attr, "datetime") == 0) return 4; /* ccsp_dateTime    */
        if (strcasecmp(ctype_attr, "base64")   == 0) return 5; /* ccsp_base64      */
        if (strcasecmp(ctype_attr, "long")     == 0) return 6; /* ccsp_long        */
        if (strcasecmp(ctype_attr, "ulong")    == 0) return 7; /* ccsp_unsignedLong*/
        if (strcasecmp(ctype_attr, "float")    == 0) return 8; /* ccsp_float       */
        if (strcasecmp(ctype_attr, "double")   == 0) return 9; /* ccsp_double      */
        if (strcasecmp(ctype_attr, "byte")     == 0) return 10;/* ccsp_byte        */
        /* ip4Addr, macAddr, and other semantic subtypes are stored as strings */
    }
    /* Fall back to storage type attribute */
    if (type_attr && *type_attr)
    {
        if (strcasecmp(type_attr, "bool") == 0) return 3; /* ccsp_boolean     */
        if (strcasecmp(type_attr, "sint") == 0) return 1; /* ccsp_int         */
        if (strcasecmp(type_attr, "uint") == 0) return 2; /* ccsp_unsignedInt */
    }
    return 0; /* ccsp_string — default for "astr" and everything else */
}

/*
 * Strip surrounding double-quotes from an XML attribute value in-place.
 * E.g. given ptr pointing to: "astr"  →  ptr is adjusted to: astr\0
 * Returns the (possibly adjusted) pointer.
 */
static char *psm_xml_strip_quotes(char *s)
{
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '"')
    {
        s++;
        len -= 2;
        s[len] = '\0';
    }
    return s;
}

/*
 * Parse one line of a PSM XML config file into its component fields.
 * The function modifies 'line' in-place (NUL-termination of tokens).
 *
 * On success, *pname / *ptype / *pctype / *pvalue point into 'line'.
 * *pctype and *pvalue may be NULL for self-closing or attribute-less records.
 *
 * Returns 1 if a valid Record element was found, 0 otherwise.
 */
static int psm_xml_parse_record(char  *line,
                                char **pname,
                                char **ptype,
                                char **pctype,
                                char **pvalue)
{
    char *t_start, *t_end, *v_start, *v_end;
    char *tok, *sp;
    const char *delim = " \t\r\n";

    *pname = *ptype = *pctype = *pvalue = NULL;

    if ((t_start = strstr(line, "<Record")) == NULL)
        return 0;
    t_start += strlen("<Record");

    if ((t_end = strchr(t_start, '>')) == NULL)
        return 0;

    *t_end = '\0';
    if (t_end > t_start && t_end[-1] == '/')
    {
        /* Self-closing element — no value */
        t_end[-1] = '\0';
        v_start    = NULL;
    }
    else
    {
        v_start = t_end + 1;
        if ((v_end = strstr(v_start, "</Record>")) == NULL)
            return 0;
        *v_end = '\0';
    }

    /* Parse attribute tokens from the opening tag */
    for (; (tok = strtok_r(t_start, delim, &sp)) != NULL; t_start = NULL)
    {
        if (strncmp(tok, "name=", 5) == 0)
            *pname  = psm_xml_strip_quotes(tok + 5);
        else if (strncmp(tok, "type=", 5) == 0)
            *ptype  = psm_xml_strip_quotes(tok + 5);
        else if (strncmp(tok, "contentType=", 12) == 0)
            *pctype = psm_xml_strip_quotes(tok + 12);
    }

    if (*pname == NULL || *pname[0] == '\0')
        return 0; /* name is mandatory */

    *pvalue = v_start; /* may be NULL for self-closing */
    return 1;
}

/*
 * Populate the psm_records SQLite table from the PSM XML config file.
 *
 * Runs on every boot using INSERT OR IGNORE so that:
 *   - First boot: all records from the merged XML are inserted into the DB.
 *   - Subsequent boots: records already in the DB (including those modified
 *     by Set operations) are preserved; only new keys missing from the DB are
 *     added.  This naturally handles firmware upgrades that introduce new
 *     PSM default parameters — the new params appear in the merged XML
 *     produced by ssp_CfmReadCurConfig() and are inserted on the next boot.
 *
 * The XML at xml_path is the merged config written by ssp_CfmReadCurConfig()
 * during PSM Engage(), so it is guaranteed to exist before psm_db_init() runs.
 *
 * Returns 0 on success or if the XML file is absent, -1 on hard error.
 * Failure is non-fatal for callers — existing callers will simply
 * receive CCSP_CR_ERR_INVALID_PARAM until a Set populates the row.
 */
static int psm_db_migrate_from_xml(sqlite3 *db, const char *xml_path)
{
    sqlite3_stmt *stmt  = NULL;
    int           rc;
    FILE         *fp    = NULL;
    char         *errmsg= NULL;
    char          line[8192];
    int           inserted = 0;
    int           ret   = 0;

    /* ------------------------------------------------------------------
     * Open the XML file — absent file is not an error (first-ever boot
     * before any XML config exists, or after factory reset).
     * ------------------------------------------------------------------ */
    fp = fopen(xml_path, "r");
    if (fp == NULL)
    {
        CcspTraceInfo(("psm_db_init: XML config %s not present, skipping migration\n",
                       xml_path));
        return 0;
    }

    /* ------------------------------------------------------------------
     * Bulk-insert inside a single transaction
     * ------------------------------------------------------------------ */
    rc = sqlite3_exec(db, "BEGIN;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: BEGIN failed: %s\n", errmsg ? errmsg : "?"));
        sqlite3_free(errmsg);
        fclose(fp);
        return -1;
    }

    rc = sqlite3_prepare_v2(db,
             "INSERT OR IGNORE INTO psm_records (name, type, value)"
             " VALUES (?1, ?2, ?3);",
             -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: migration INSERT prepare failed: %s\n",
                        sqlite3_errmsg(db)));
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        fclose(fp);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char *name, *type_attr, *ctype_attr, *value;

        if (!psm_xml_parse_record(line, &name, &type_attr, &ctype_attr, &value))
            continue;

        int ccsp_type = psm_xml_ccsp_type(type_attr, ctype_attr);

        sqlite3_bind_text(stmt, 1, name,  -1, SQLITE_TRANSIENT);
        sqlite3_bind_int (stmt, 2, ccsp_type);
        sqlite3_bind_text(stmt, 3, value ? value : "", -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            CcspTraceWarning(("psm_db_init: migration INSERT failed for '%s': %s\n",
                              name, sqlite3_errmsg(db)));
        }
        else
        {
            inserted++;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
    fclose(fp);

    rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK)
    {
        CcspTraceError(("psm_db_init: migration COMMIT failed: %s\n",
                        errmsg ? errmsg : "?"));
        sqlite3_free(errmsg);
        ret = -1;
    }
    else
    {
        CcspTraceInfo(("psm_db_init: migrated %d records from %s into SQLite DB\n",
                       inserted, xml_path));
    }

    return ret;
}

/* -----------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */

int psm_db_init(void)
{
    sqlite3 *db = NULL;
    int rc;
    int attempt;

    CcspTraceInfo(("psm_db_init: opening SQLite PSM database at %s (new SQLite flow)\n", PSM_DB_PATH));

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

        CcspTraceInfo(("psm_db_init: database file opened (attempt %d)\n", attempt + 1));

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

    /* ------------------------------------------------------------------
     * Sync XML defaults into SQLite using INSERT OR IGNORE.
     * Existing DB records are never overwritten; new keys from the XML
     * (e.g. added by a firmware upgrade) are inserted.
     * ------------------------------------------------------------------ */
    if (psm_db_migrate_from_xml(db, PSM_XML_CONFIG_PATH) != 0)
    {
        CcspTraceWarning(("psm_db_init: XML migration failed — DB starts empty\n"));
        /* Non-fatal: PSM Get calls will return NOT_FOUND until Sets populate rows */
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
