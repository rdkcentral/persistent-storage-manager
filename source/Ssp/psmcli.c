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
 * psmcli.c — lightweight PSM command-line tool (Phase 2).
 *
 * Replaces the old psmcli that communicated with the PsmSsp daemon via RBUS.
 * Now operates directly on the SQLite database so it works even though the
 * PSM daemon no longer runs after boot initialisation.
 *
 * Usage:
 *   psmcli get   <key>
 *   psmcli set   <key> <value>            (type defaults to astr/ccsp_string)
 *   psmcli set   <key> <type> <value>     (type: astr|sint|uint|bool)
 *   psmcli setdetail <key> <type> <value>
 *   psmcli del   <key>
 *   psmcli getwildcard <prefix>           (print all keys matching prefix%)
 *   psmcli factoryreset                   (delete psm.db — device must reboot)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sqlite3.h>

#ifndef PSM_DB_PATH
#define PSM_DB_PATH "/nvram/psm.db"
#endif

/* -----------------------------------------------------------------------
 * Type string → integer mapping (matches ccsp dataType_e in ccsp_base_api.h)
 * --------------------------------------------------------------------- */
static int type_from_string(const char *s)
{
    if (!s) return 0;
    if (strcasecmp(s, "sint") == 0) return 1;  /* ccsp_int         */
    if (strcasecmp(s, "uint") == 0) return 2;  /* ccsp_unsignedInt */
    if (strcasecmp(s, "bool") == 0) return 3;  /* ccsp_boolean     */
    return 0;                                   /* ccsp_string (astr / default) */
}

/* -----------------------------------------------------------------------
 * Commands
 * --------------------------------------------------------------------- */
static int cmd_get(sqlite3 *db, const char *key)
{
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db,
                 "SELECT value FROM psm_records WHERE name = ?1 LIMIT 1;",
                 -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "psmcli: prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const char *val = (const char *)sqlite3_column_text(stmt, 0);
        printf("%s\n", val ? val : "");
        sqlite3_finalize(stmt);
        return 0;
    }
    sqlite3_finalize(stmt);
    /* Key not found — print nothing, return non-zero so scripts can check */
    return 1;
}

static int cmd_set(sqlite3 *db, const char *key, const char *type_str, const char *value)
{
    sqlite3_stmt *stmt = NULL;
    int type = type_from_string(type_str);

    int rc = sqlite3_prepare_v2(db,
                 "INSERT OR REPLACE INTO psm_records (name, type, value)"
                 " VALUES (?1, ?2, ?3);",
                 -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "psmcli: prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, key,   -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, type);
    sqlite3_bind_text(stmt, 3, value, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "psmcli: set failed for '%s': %s\n", key, sqlite3_errmsg(db));
        return 1;
    }
    return 0;
}

static int cmd_del(sqlite3 *db, const char *key)
{
    sqlite3_stmt *stmt = NULL;
    const char *sql;
    char pattern[512];
    int rc;

    if (key[strlen(key) - 1] == '.')
    {
        snprintf(pattern, sizeof(pattern), "%s%%", key);
        sql = "DELETE FROM psm_records WHERE name LIKE ?1;";
    }
    else
    {
        sql = "DELETE FROM psm_records WHERE name = ?1;";
    }

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "psmcli: prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1,
                      (key[strlen(key) - 1] == '.') ? pattern : key,
                      -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "psmcli: del failed for '%s': %s\n", key, sqlite3_errmsg(db));
        return 1;
    }
    return 0;
}

static int cmd_factoryreset(void)
{
    if (unlink(PSM_DB_PATH) != 0 && errno != ENOENT)
    {
        fprintf(stderr, "psmcli: factoryreset: cannot remove %s: %s\n",
                PSM_DB_PATH, strerror(errno));
        return 1;
    }
    return 0;
}

static int cmd_getwildcard(sqlite3 *db, const char *prefix)
{
    sqlite3_stmt *stmt = NULL;
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s%%", prefix);

    int rc = sqlite3_prepare_v2(db,
                 "SELECT name, value FROM psm_records WHERE name LIKE ?1 ORDER BY name;",
                 -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "psmcli: prepare failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_bind_text(stmt, 1, pattern, -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *name = (const char *)sqlite3_column_text(stmt, 0);
        const char *val  = (const char *)sqlite3_column_text(stmt, 1);
        printf("%s %s\n", name ? name : "", val ? val : "");
    }
    sqlite3_finalize(stmt);
    return 0;
}

/* -----------------------------------------------------------------------
 * main
 * --------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,
            "Usage:\n"
            "  psmcli get <key>\n"
            "  psmcli set <key> [<type>] <value>\n"
            "  psmcli setdetail <key> <type> <value>\n"
            "  psmcli del <key>\n"
            "  psmcli getwildcard <prefix>\n"
            "  psmcli factoryreset\n");
        return 1;
    }

    const char *cmd = argv[1];

    /* factoryreset needs no key argument and no DB open */
    if (strcmp(cmd, "factoryreset") == 0)
        return cmd_factoryreset();

    if (argc < 3)
    {
        fprintf(stderr,
            "Usage:\n"
            "  psmcli get <key>\n"
            "  psmcli set <key> [<type>] <value>\n"
            "  psmcli setdetail <key> <type> <value>\n"
            "  psmcli del <key>\n"
            "  psmcli getwildcard <prefix>\n"
            "  psmcli factoryreset\n");
        return 1;
    }

    sqlite3 *db = NULL;
    int flags = SQLITE_OPEN_FULLMUTEX;

    /* del and set need write access; get/getwildcard are read-only */
    if (strcmp(cmd, "get") == 0 || strcmp(cmd, "getwildcard") == 0)
        flags |= SQLITE_OPEN_READONLY;
    else
        flags |= SQLITE_OPEN_READWRITE;

    int rc = sqlite3_open_v2(PSM_DB_PATH, &db, flags, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "psmcli: cannot open %s: %s\n",
                PSM_DB_PATH, db ? sqlite3_errmsg(db) : "unknown");
        if (db) sqlite3_close(db);
        return 1;
    }

    /* Busy timeout: wait up to 5 s if another process holds a write lock */
    sqlite3_busy_timeout(db, 5000);

    int ret = 1;

    if (strcmp(cmd, "get") == 0)
    {
        ret = cmd_get(db, argv[2]);
    }
    else if (strcmp(cmd, "set") == 0 || strcmp(cmd, "setdetail") == 0)
    {
        /*
         * set accepts two forms for backward compatibility with scripts:
         *   psmcli set <key> <value>              (argc==4, type defaults to astr)
         *   psmcli set <key> <type> <value>       (argc==5)
         * setdetail always requires the type:
         *   psmcli setdetail <key> <type> <value> (argc==5)
         */
        if (strcmp(cmd, "set") == 0 && argc == 4)
        {
            /* 2-arg form: no type specified, default to ccsp_string (0) */
            ret = cmd_set(db, argv[2], NULL, argv[3]);
        }
        else if (argc >= 5)
        {
            ret = cmd_set(db, argv[2], argv[3], argv[4]);
        }
        else
        {
            fprintf(stderr, "psmcli: %s requires <key> <type> <value>\n", cmd);
        }
    }
    else if (strcmp(cmd, "del") == 0)
    {
        ret = cmd_del(db, argv[2]);
    }
    else if (strcmp(cmd, "getwildcard") == 0)
    {
        ret = cmd_getwildcard(db, argv[2]);
    }
    else
    {
        fprintf(stderr, "psmcli: unknown command '%s'\n", cmd);
    }

    sqlite3_close(db);
    return ret;
}
