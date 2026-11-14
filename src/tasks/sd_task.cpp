#include "../../include/patient_monitor/sd_task.h"
#include <string.h>

namespace patient_monitor {

static uint32_t calculateCrc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

SdTask::SdTask(ISdStorageBackend& backend)
    : backend_(backend),
      current_patient_id_(1),
      current_session_id_(1),
      sequence_counter_(0),
      write_success_count_(0),
      misattribution_drops_(0) {}

bool SdTask::switchSession(uint32_t new_patient_id, uint32_t new_session_id) {
    if (new_patient_id == 0 || new_session_id == 0) {
        return false;
    }
    backend_.sync();
    if (backend_.openSessionDirectory(new_patient_id, new_session_id)) {
        current_patient_id_ = new_patient_id;
        current_session_id_ = new_session_id;
        sequence_counter_ = 0;
        return true;
    }
    return false;
}

bool SdTask::writeEcgBatch(const EcgFrame* frames, size_t count) {
    if (!frames || count == 0) {
        return false;
    }
    for (size_t i = 0; i < count; ++i) {
        if (frames[i].envelope.patient_id != current_patient_id_ ||
            frames[i].envelope.session_id != current_session_id_) {
            misattribution_drops_++;
            return false;
        }
    }

    size_t payload_size = sizeof(EcgFrame) * count;
    RecordHeader header;
    header.magic = 0x504D4F4E;
    header.version = 1;
    header.stream_type = static_cast<uint8_t>(StreamType::ECG_BATCH);
    header.reserved = 0;
    header.patient_id = current_patient_id_;
    header.session_id = current_session_id_;
    header.sequence = sequence_counter_++;
    header.timestamp_us = frames[0].envelope.monotonic_timestamp_us;
    header.payload_length = static_cast<uint32_t>(payload_size);
    header.checksum = calculateCrc32(reinterpret_cast<const uint8_t*>(frames), payload_size);

    uint8_t write_buf[sizeof(RecordHeader) + sizeof(EcgFrame) * 16];
    if (sizeof(header) + payload_size <= sizeof(write_buf)) {
        memcpy(write_buf, &header, sizeof(header));
        memcpy(write_buf + sizeof(header), frames, payload_size);
        if (backend_.appendRecord(StreamType::ECG_BATCH, write_buf, sizeof(header) + payload_size)) {
            write_success_count_++;
            return true;
        }
    }
    return false;
}

bool SdTask::writeSpo2Sample(const Spo2Sample& sample) {
    if (sample.envelope.patient_id != current_patient_id_ ||
        sample.envelope.session_id != current_session_id_) {
        misattribution_drops_++;
        return false;
    }

    RecordHeader header;
    header.magic = 0x504D4F4E;
    header.version = 1;
    header.stream_type = static_cast<uint8_t>(StreamType::SPO2_RECORD);
    header.reserved = 0;
    header.patient_id = current_patient_id_;
    header.session_id = current_session_id_;
    header.sequence = sequence_counter_++;
    header.timestamp_us = sample.envelope.monotonic_timestamp_us;
    header.payload_length = sizeof(Spo2Sample);
    header.checksum = calculateCrc32(reinterpret_cast<const uint8_t*>(&sample), sizeof(Spo2Sample));

    uint8_t write_buf[sizeof(RecordHeader) + sizeof(Spo2Sample)];
    memcpy(write_buf, &header, sizeof(header));
    memcpy(write_buf + sizeof(header), &sample, sizeof(Spo2Sample));

    if (backend_.appendRecord(StreamType::SPO2_RECORD, write_buf, sizeof(write_buf))) {
        write_success_count_++;
        return true;
    }
    return false;
}

bool SdTask::writeBpRecord(const BpRecord& record) {
    if (record.envelope.patient_id != current_patient_id_ ||
        record.envelope.session_id != current_session_id_) {
        misattribution_drops_++;
        return false;
    }

    RecordHeader header;
    header.magic = 0x504D4F4E;
    header.version = 1;
    header.stream_type = static_cast<uint8_t>(StreamType::BP_RECORD);
    header.reserved = 0;
    header.patient_id = current_patient_id_;
    header.session_id = current_session_id_;
    header.sequence = sequence_counter_++;
    header.timestamp_us = record.envelope.monotonic_timestamp_us;
    header.payload_length = sizeof(BpRecord);
    header.checksum = calculateCrc32(reinterpret_cast<const uint8_t*>(&record), sizeof(BpRecord));

    uint8_t write_buf[sizeof(RecordHeader) + sizeof(BpRecord)];
    memcpy(write_buf, &header, sizeof(header));
    memcpy(write_buf + sizeof(header), &record, sizeof(BpRecord));

    if (backend_.appendRecord(StreamType::BP_RECORD, write_buf, sizeof(write_buf))) {
        write_success_count_++;
        return true;
    }
    return false;
}

bool SdTask::writeEvent(const DeviceEvent& event) {
    RecordHeader header;
    header.magic = 0x504D4F4E;
    header.version = 1;
    header.stream_type = static_cast<uint8_t>(StreamType::EVENT_RECORD);
    header.reserved = 0;
    header.patient_id = current_patient_id_;
    header.session_id = current_session_id_;
    header.sequence = sequence_counter_++;
    header.timestamp_us = event.timestamp_us;
    header.payload_length = sizeof(DeviceEvent);
    header.checksum = calculateCrc32(reinterpret_cast<const uint8_t*>(&event), sizeof(DeviceEvent));

    uint8_t write_buf[sizeof(RecordHeader) + sizeof(DeviceEvent)];
    memcpy(write_buf, &header, sizeof(header));
    memcpy(write_buf + sizeof(header), &event, sizeof(DeviceEvent));

    if (backend_.appendRecord(StreamType::EVENT_RECORD, write_buf, sizeof(write_buf))) {
        write_success_count_++;
        return true;
    }
    return false;
}

void startSdTask(void* /*pvParameters*/) {
}

} // namespace patient_monitor
