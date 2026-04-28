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

/**
 * psm_api_perf_test.c
 *
 * Performance test for PSM_Set_Record_Value2 and PSM_Get_Record_Value2.
 *
 * Usage:
 *   psm_api_perf_test <iterations> [subsystem_prefix]
 *
 *   iterations       : number of SET and GET calls each (required, > 0)
 *   subsystem_prefix : CCSP subsystem prefix, e.g. "eRT." (default: "")
 *
 * Compile example (cross-build or on-device):
 *   gcc -std=c99 -O2 -o psm_api_perf_test psm_api_perf_test.c \
 *       -I/usr/include/ccsp \
 *       -lccsp_common -lpthread
 *
 * The program:
 *   1. Initialises the CCSP message bus.
 *   2. Runs N iterations of PSM_Set_Record_Value2 on dmsb.test.PerfTestKey.
 *   3. Runs N iterations of PSM_Get_Record_Value2 on the same key and checks
 *      that the returned value matches the expected string "perftest123".
 *   4. Prints a results table with:
 *        - SET: total time, avg per call, pass/fail count
 *        - GET: total time, avg per call, pass/fail count, value mismatch count
 *        - COMBINED: total time for both phases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ccsp_message_bus.h"   /* CCSP_Message_Bus_Init/Exit, CCSP_MESSAGE_BUS_INFO */
#include "ccsp_base_api.h"      /* CCSP_SUCCESS, ccsp_string                         */
#include "ccsp_psm_helper.h"    /* PSM_Set_Record_Value2, PSM_Get_Record_Value2       */
#include "ccsp_custom.h"        /* CCSP_MSG_BUS_CFG                                  */

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

#define TEST_COMPONENT_ID   "com.cisco.spvtg.ccsp.psm.perftest"
#define TEST_KEY            "dmsb.test.PerfTestKey"
#define TEST_VALUE          "perftest123"

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */

/** Returns wall-clock time in nanoseconds (monotonic). */
static long long get_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

/** Prints a formatted time breakdown: ms and us. */
static void print_time(const char *label, long long ns)
{
    printf("   %-20s: %lld ms  (%lld us)\n",
           label, ns / 1000000LL, ns / 1000LL);
}

/** Prints avg-per-call breakdown in us and ns. */
static void print_avg(const char *label, long long ns_per_call)
{
    printf("   %-20s: %lld us  (%lld ns)\n",
           label, ns_per_call / 1000LL, ns_per_call);
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    /* ---- Argument parsing ----------------------------------------- */
    if (argc < 2) {
        fprintf(stderr,
                "Usage: %s <iterations> [subsystem_prefix]\n"
                "  iterations      : number of SET/GET calls each (> 0)\n"
                "  subsystem_prefix: e.g. \"eRT.\" or \"\" (default: \"\")\n",
                argv[0]);
        return 1;
    }

    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        fprintf(stderr, "ERROR: iterations must be a positive integer\n");
        return 1;
    }

    const char *subsystem = (argc >= 3) ? argv[2] : "";

    /* ---- Bus initialisation --------------------------------------- */
    void *bus_handle = NULL;
    int ret = CCSP_SUCCESS;
#ifndef CORD_ENABLED
    ret = CCSP_Message_Bus_Init(
        TEST_COMPONENT_ID,
        CCSP_MSG_BUS_CFG,   /* /tmp/ccsp_msg.cfg */
        &bus_handle,
        NULL,               /* use default malloc */
        NULL                /* use default free   */
    );
    if (ret != CCSP_SUCCESS) {
        fprintf(stderr, "ERROR: CCSP_Message_Bus_Init failed (ret=%d)\n", ret);
        return 1;
    }
#else
    cord_open();
#endif /* CORD_ENABLED */

    printf("========================================\n");
    printf(" PSM API Performance Test\n");
    printf("========================================\n");
    printf(" Component   : %s\n", TEST_COMPONENT_ID);
    printf(" Subsystem   : \"%s\"\n", subsystem);
    printf(" Key         : %s\n", TEST_KEY);
    printf(" Value       : %s\n", TEST_VALUE);
    printf(" Iterations  : %d\n", iterations);
    printf("========================================\n\n");

    /* ================================================================
     * Phase 1 – SET benchmark
     * ================================================================ */
    printf("[1/2] Running SET x%d ...\n", iterations);

    int set_pass = 0;
    int set_fail = 0;

    long long set_start = get_time_ns();

    for (int i = 0; i < iterations; i++) {
        ret = PSM_Set_Record_Value2(bus_handle, subsystem,
                                   TEST_KEY, ccsp_string, TEST_VALUE);
        if (ret == CCSP_SUCCESS)
            set_pass++;
        else
            set_fail++;
    }

    long long set_ns = get_time_ns() - set_start;

    /* ================================================================
     * Phase 2 – GET benchmark
     * ================================================================ */
    printf("[2/2] Running GET x%d ...\n", iterations);

    int  get_pass     = 0;
    int  get_fail     = 0;
    int  get_mismatch = 0;

    long long get_start = get_time_ns();

    for (int i = 0; i < iterations; i++) {
        char        *value = NULL;
        unsigned int type  = 0;

        ret = PSM_Get_Record_Value2(bus_handle, subsystem,
                                   TEST_KEY, &type, &value);
        if (ret == CCSP_SUCCESS) {
            get_pass++;
            if (value == NULL || strcmp(value, TEST_VALUE) != 0)
                get_mismatch++;
            /* Free using the bus allocator – matches how the library allocated it */
            if (value)
                ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(value);
        } else {
            get_fail++;
        }
    }

    long long get_ns = get_time_ns() - get_start;

    /* ================================================================
     * Compute summary values
     * ================================================================ */
    long long set_avg_ns = (iterations > 0) ? (set_ns / iterations) : 0;
    long long get_avg_ns = (iterations > 0) ? (get_ns / iterations) : 0;
    long long total_ns   = set_ns + get_ns;

    int set_ok  = (set_fail == 0);
    int get_ok  = (get_fail == 0 && get_mismatch == 0);

    /* ================================================================
     * Print results
     * ================================================================ */
    printf("\n========================================\n");
    printf(" RESULTS\n");
    printf("========================================\n");

    printf("\n SET (%d iterations):\n", iterations);
    print_time("Total time",    set_ns);
    print_avg ("Avg per call",  set_avg_ns);
    printf("   %-20s: %s\n", "Status",  set_ok ? "PASSED" : "FAILED");
    printf("   %-20s: %d\n", "Passed",  set_pass);
    printf("   %-20s: %d\n", "Failed",  set_fail);

    printf("\n GET (%d iterations):\n", iterations);
    print_time("Total time",    get_ns);
    print_avg ("Avg per call",  get_avg_ns);
    printf("   %-20s: %s\n", "Status",        get_ok ? "PASSED" : "FAILED");
    printf("   %-20s: %d\n", "Passed",         get_pass);
    printf("   %-20s: %d\n", "Failed",         get_fail);
    printf("   %-20s: %d\n", "Value mismatch", get_mismatch);

    printf("\n COMBINED:\n");
    print_time("Total time (set+get)", total_ns);

    printf("\n========================================\n");

    /* ---- Clean up ------------------------------------------------- */
#ifndef CORD_ENABLED
    CCSP_Message_Bus_Exit(bus_handle);
#else
    cord_close();
#endif /* CORD_ENABLED */

    return (set_ok && get_ok) ? 0 : 1;
}
