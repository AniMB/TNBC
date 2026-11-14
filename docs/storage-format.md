# SD Storage Format & Session Attribution Specification

## 1. Directory Structure

The SD card organizes records hierarchically by patient ID and session ID:

```text
SD_CARD/
├── schema.json               # Manifest describing binary record version
├── patients.idx              # Index mapping patient IDs to session counts
├── PAT0001/
│   ├── profile.bin           # Demographic profile metadata
│   ├── SESSION0001/
│   │   ├── session.meta      # Session start/stop timestamps and stream info
│   │   ├── ecg_0001.bin      # Continuous binary ECG frames
│   │   ├── spo2_0001.bin     # Optical SpO2 records
│   │   ├── bp.bin            # Blood pressure cycles and results
│   │   └── events.log        # System mode transitions and safety trips
│   └── SESSION0002/
└── PAT0002/
```

## 2. Binary Record Header (`RecordHeader`)

All files consist of append-only binary records prefixed by a 32-byte packed header:

```cpp
#pragma pack(push, 1)
struct RecordHeader {
    uint32_t magic;           // 0x504D4F4E ("PMON" in little-endian)
    uint16_t version;         // Record format version (1)
    uint8_t  stream_type;     // StreamType (ECG, SpO2, BP, EVENT)
    uint8_t  reserved;        // Padding / alignment
    uint32_t patient_id;      // Immutable patient identifier
    uint32_t session_id;      // Immutable session identifier
    uint32_t sequence;        // Monotonically increasing record sequence
    uint64_t timestamp_us;    // Monotonic microsecond timestamp
    uint32_t payload_length;  // Byte length of payload immediately following
    uint32_t checksum;        // IEEE 802.3 CRC32 of payload data
};
#pragma pack(pop)
```

## 3. Session Misattribution Prevention

Only one patient session is active at a time. The storage worker (`SdTask`) maintains `current_patient_id_` and `current_session_id_`.
- When any batch arrives from a queue, the header IDs are checked against active session IDs.
- If a mismatch is detected, the batch is rejected and `misattribution_drops_` is incremented.
- When switching sessions via `switchSession(new_pid, new_sid)`, pending writes are flushed and synchronized before the new directory is opened.

## 4. File Rotation & Disk-Full Policy

- Maximum individual file size is clamped to **512 MiB**, well below the FAT32 4 GiB single-file limit.
- When file offset exceeds 512 MiB, the file is closed and a new sequence file is opened (e.g. `ecg_0002.bin`).
- If the SD card reports less than 10 MiB free space, a `STORAGE_ERROR` event is emitted and recording transitions to graceful fallback mode.
