/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/


#include "SourceComponentBuffer.h"
namespace traact::buffer {
SourceComponentBuffer::SourceComponentBuffer(ComponentBuffer &output_buffer)
    : local_output_buffer_(output_buffer) {

}

Timestamp SourceComponentBuffer::getTimestamp() {
    return local_output_buffer_.getTimestamp();
}

size_t SourceComponentBuffer::getOutputCount() {
    return local_output_buffer_.getOutputCount();
}

void SourceComponentBuffer::commit(bool valid) {
    SPDLOG_TRACE("commit value, component: {0} time step: {1} ts: {2} valid: {3}", local_output_buffer_.getComponentIndex(), local_output_buffer_.getTimeStepIndex(), getTimestamp(), valid);

    source_lock_.set_value(valid);
    lock_set_ = true;
    //commit_callback_(this, valid);
}

std::future<bool> SourceComponentBuffer::getSourceLock() {
    return source_lock_.get_future();
}

void SourceComponentBuffer::resetLock() {
    source_lock_ = {};
    lock_set_ = false;
}

void SourceComponentBuffer::cancel() {
    if (lock_set_.exchange(true)) {
        source_lock_.set_value(false);
    }

}
}