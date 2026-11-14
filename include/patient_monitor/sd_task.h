#pragma once

#include <stdint.h>
#include <stddef.h>
#include "patient_session.h"
#include "events.h"

namespace patient_monitor {

enum class StreamType : uint8_t {
    MANIFEST = 0x01,
    ECG_BATCH = 0x02,
    SPO2_RECORD = 0x03,
    BP_RECORD = 0x04,
    EVENT_RECORD = 0x05,
    SESSION_META = 0x06
};

#pragma pack(push, 1)
struct RecordHeader {
    uint32_t magic;           // 0x504D4F4E ("PMON")
    uint16_t version;         // Format version (1)
    uint8_t  stream_type;     // StreamType
    uint8_t  reserved;
    uint32_t patient_id;
    uint32_t session_id;
    uint32_t sequence;
    uint64_t timestamp_us;
    uint32_t payload_length;
    uint32_t checksum;        // CRC32 of payload
};
#pragma pack(pop)

class ISdStorageBackend {
public:
    virtual ~ISdStorageBackend() = default;
    virtual bool isCardPresent() = 0;
    virtual bool openSessionDirectory(uint32_t patient_id, uint32_t session_id) = 0;
    virtual bool appendRecord(StreamType stream, const uint8_t* data, size_t length) = 0;
    virtual bool sync() = 0;
    virtual size_t getCurrentFileOffset(StreamType stream) = 0;
};

class SdTask {
public:
    explicit SdTask(ISdStorageBackend& backend);

    bool switchSession(uint32_t new_patient_id, uint32_t new_session_id);
    bool writeEcgBatch(const EcgFrame* frames, size_t count);
    bool writeSpo2Sample(const Spo2Sample& sample);
    bool writeBpRecord(const BpRecord& record);
    bool writeEvent(const DeviceEvent& event);

    uint32_t getActivePatientId() const { return current_patient_id_; }
    uint32_t getActiveSessionId() const { return current_session_id_; }
    uint32_t getWriteSuccessCount() const { return write_success_count_; }
    uint32_t getMisattributionDrops() const { return misattribution_drops_; }

private:
    ISdStorageBackend& backend_;
    uint32_t current_patient_id_;
    uint32_t current_session_id_;
    uint32_t sequence_counter_;
    uint32_t write_success_count_;
    uint32_t misattribution_drops_;
};

void startSdTask(void* pvParameters);

} // namespace patient_monitor
