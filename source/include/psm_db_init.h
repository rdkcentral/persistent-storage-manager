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

#ifndef PSM_DB_INIT_H
#define PSM_DB_INIT_H

/**
 * @brief PSM SQLite database path.
 *
 * The common library (ccsp_base_api.c) opens the same path when performing
 * direct SQLite Get/Set operations.  Both sides must agree on this constant.
 */
#define PSM_DB_PATH   "/nvram/psm.db"

/**
 * @brief Initialize the PSM SQLite database.
 *
 * Must be called during PSM daemon startup, before sd_notify(READY=1) and
 * before any consumer component can issue a PSM Get or Set request.
 *
 * The function will:
 *  - Open (or create) the SQLite database at PSM_DB_PATH.
 *  - Apply PRAGMA settings (WAL journal mode, NORMAL synchronous level).
 *  - Create the psm_records table and its index with IF NOT EXISTS guards.
 *  - Run PRAGMA integrity_check; on failure, delete and recreate the database.
 *  - Set file ownership/permissions (0640) on the database file.
 *
 * @return  0 on success, -1 on unrecoverable failure.
 */
int psm_db_init(void);

#endif /* PSM_DB_INIT_H */
