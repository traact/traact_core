/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/


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
}

std::future<bool> SourceComponentBuffer::getSourceLock() {
    return source_lock_.get_future();
}

void SourceComponentBuffer::resetLock() {
    source_lock_ = {};
}

void SourceComponentBuffer::cancel() {

    try {
        SPDLOG_TRACE("cancel value, component: {0} time step: {1} ts: {2} ",
                     local_output_buffer_.getComponentIndex(),
                     local_output_buffer_.getTimeStepIndex(),
                     getTimestamp());
        source_lock_.set_value(true);
    }catch (std::future_error& e){
        SPDLOG_ERROR(e.what());
        // TODO this should not happen, but any operation in the task flow should not throw, otherwise everything stops
        //throw e;
    } catch (...){
        SPDLOG_ERROR("unknown exception when trying to cancel source component buffer");
    }


}
}