void processHeartbeat(const HeartBeat* heartbeat) {
    auto now = std::chrono::high_resolution_clock::now();
    auto system_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    logger_->info("Heartbeat Message Details:");
    logger_->info("  Message Length: {} bytes", heartbeat->header.body_len);
    logger_->info("  Template ID: {} (HEART_BEAT)", heartbeat->header.template_id);
    logger_->info("  Message Sequence: {}", heartbeat->header.msg_seq_num);
    logger_->info("  Last Processed Sequence: {}", heartbeat->last_msg_seq_num_processed);
    logger_->info("Timing Information:");
    logger_->info("  System Time: {} ns", system_ns);
    logger_->info("  Time Difference: {} ns", 
                 system_ns - static_cast<int64_t>(last_exchange_time_));

    last_exchange_time_ = system_ns;
    checkSequenceGap(heartbeat->header.msg_seq_num);
}


