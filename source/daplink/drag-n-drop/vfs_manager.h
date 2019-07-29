/**
 * @file    vfs_manager.h
 * @brief   Methods that build and manipulate a virtual file system
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VFS_MANAGER_USER_H
#define VFS_MANAGER_USER_H

#include "stdint.h"
#include "stdbool.h"

#include "virtual_fs.h"
#include "error.h"
#include "file_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const vfs_filename_t daplink_mode_file_name;
extern const vfs_filename_t daplink_drive_name;
extern const vfs_filename_t daplink_url_name;
extern const char *const daplink_target_url;


/* Callable from anywhere */

// Enable or disable the virtual filesystem
void vfs_mngr_fs_enable(bool enabled);

// Remount the virtual filesystem
void vfs_mngr_fs_remount(void);


/* Callable only from the thread running the virtual fs */

// Initialize the VFS manager
// Must be called after USB has been initialized (usbd_init())
// Notes: Must only be called from the thread runnning USB
void vfs_mngr_init(bool enabled);

// Run the vfs manager state machine
// Notes: Must only be called from the thread runnning USB
void vfs_mngr_periodic(uint32_t elapsed_ms);

// Return the status of the last transfer or ERROR_SUCCESS
// if none have been performed yet
error_t vfs_mngr_get_transfer_status(void);


/* Use functions */

// Build the filesystem by calling vfs_init and then adding files with vfs_create_file
void vfs_user_build_filesystem(void);

// Called when a file on the filesystem changes
void vfs_user_file_change_handler(const vfs_filename_t filename, vfs_file_change_t change, vfs_file_t file, vfs_file_t new_file_data);

// Called when VFS is disconnecting
void vfs_user_disconnecting(void);


// Set to 1 to enable debugging
#define DEBUG_VFS_MANAGER     0

#if DEBUG_VFS_MANAGER
#define vfs_mngr_printf    debug_msg
#else
#define vfs_mngr_printf(...)
#endif

#define INVALID_TIMEOUT_MS  0xFFFFFFFF
#define MAX_EVENT_TIME_MS   60000

#define CONNECT_DELAY_MS 0
#define RECONNECT_DELAY_MS 2500    // Must be above 1s for windows (more for linux)
// TRANSFER_IN_PROGRESS
#define DISCONNECT_DELAY_TRANSFER_TIMEOUT_MS 20000
// TRANSFER_CAN_BE_FINISHED
#define DISCONNECT_DELAY_TRANSFER_IDLE_MS 500
// TRANSFER_NOT_STARTED || TRASNFER_FINISHED
#define DISCONNECT_DELAY_MS 500

// Make sure none of the delays exceed the max time
COMPILER_ASSERT(CONNECT_DELAY_MS < MAX_EVENT_TIME_MS);
COMPILER_ASSERT(RECONNECT_DELAY_MS < MAX_EVENT_TIME_MS);
COMPILER_ASSERT(DISCONNECT_DELAY_TRANSFER_TIMEOUT_MS < MAX_EVENT_TIME_MS);
COMPILER_ASSERT(DISCONNECT_DELAY_TRANSFER_IDLE_MS < MAX_EVENT_TIME_MS);
COMPILER_ASSERT(DISCONNECT_DELAY_MS < MAX_EVENT_TIME_MS);

typedef enum {
    TRANSFER_NOT_STARTED,
    TRANSFER_IN_PROGRESS,
    TRANSFER_CAN_BE_FINISHED,
    TRASNFER_FINISHED,
} transfer_state_t;

typedef struct {
    vfs_file_t file_to_program;     // A pointer to the directory entry of the file being programmed
    vfs_sector_t start_sector;      // Start sector of the file being programmed by stream
    vfs_sector_t file_start_sector; // Start sector of the file being programmed by vfs
    vfs_sector_t file_next_sector;  // Expected next sector of the file
    vfs_sector_t last_ooo_sector;   // Last out of order sector within the file
    uint32_t size_processed;        // The number of bytes processed by the stream
    uint32_t file_size;             // Size of the file indicated by root dir.  Only allowed to increase
    uint32_t size_transferred;      // The number of bytes transferred
    transfer_state_t transfer_state;// Transfer state
    bool stream_open;               // State of the stream
    bool stream_started;            // Stream processing started. This only gets reset remount
    bool stream_finished;           // Stream processing is done. This only gets reset remount
    bool stream_optional_finish;    // True if the stream processing can be considered done
    bool file_info_optional_finish; // True if the file transfer can be considered done
    bool transfer_timeout;          // Set if the transfer was finished because of a timeout. This only gets reset remount
    stream_type_t stream;           // Current stream or STREAM_TYPE_NONE is stream is closed.  This only gets reset remount
} file_transfer_state_t;

typedef enum {
    VFS_MNGR_STATE_DISCONNECTED,
    VFS_MNGR_STATE_RECONNECTING,
    VFS_MNGR_STATE_CONNECTED
} vfs_mngr_state_t;

static const file_transfer_state_t default_transfer_state = {
    VFS_FILE_INVALID,
    VFS_INVALID_SECTOR,
    VFS_INVALID_SECTOR,
    VFS_INVALID_SECTOR,
    VFS_INVALID_SECTOR,
    0,
    0,
    0,
    TRANSFER_NOT_STARTED,
    false,
    false,
    false,
    false,
    false,
    false,
    STREAM_TYPE_NONE,
};

static uint32_t usb_buffer[VFS_SECTOR_SIZE / sizeof(uint32_t)];
static error_t fail_reason = ERROR_SUCCESS;
static file_transfer_state_t file_transfer_state;

// These variables can be access from multiple threads
// so access to them must be synchronized
static vfs_mngr_state_t vfs_state;
static vfs_mngr_state_t vfs_state_next;
static uint32_t time_usb_idle;

static OS_MUT sync_mutex;
static OS_TID sync_thread = 0;

// Synchronization functions
static void sync_init(void);
static void sync_assert_usb_thread(void);
static void sync_lock(void);
static void sync_unlock(void);

static bool changing_state(void);
static void build_filesystem(void);
static void file_change_handler(const vfs_filename_t filename, vfs_file_change_t change, vfs_file_t file, vfs_file_t new_file_data);
static void file_data_handler(uint32_t sector, const uint8_t *buf, uint32_t num_of_sectors);
static bool ready_for_state_change(void);
static void abort_remount(void);

static void transfer_update_file_info(vfs_file_t file, uint32_t start_sector, uint32_t size, stream_type_t stream);
static void transfer_reset_file_info(void);
static void transfer_stream_open(stream_type_t stream, uint32_t start_sector);
static void transfer_stream_data(uint32_t sector, const uint8_t *data, uint32_t size);
static void transfer_update_state(error_t status);

void sendDATA(uint8_t *buf, uint32_t num_of_sectors);


#ifdef __cplusplus
}
#endif

#endif
