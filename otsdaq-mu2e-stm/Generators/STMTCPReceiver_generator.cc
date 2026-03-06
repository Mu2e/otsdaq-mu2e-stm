// STMTCPReceiver_generator.cc
// Author - George Sweetmore

// =====================================================================================
// CPP Includes
// =====================================================================================
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <tuple>
#include <chrono>
#include <poll.h>
#include <boost/lockfree/spsc_queue.hpp>

// =====================================================================================
// ARTDAQ includes
// =====================================================================================
#include "artdaq/DAQdata/Globals.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "artdaq/Generators/GeneratorMacros.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/ContainerFragmentLoader.hh"
#include "artdaq-core-mu2e/Overlays/FragmentType.hh"
#include "artdaq-core-mu2e/Overlays/STMFragment.hh"
#include "fhiclcpp/fwd.h"
#include "fhiclcpp/ParameterSet.h"

// =====================================================================================
// Header includes
// =====================================================================================
#include "STMTCPReceiver.hh"

//////////////////////////////// - STMTCPReceiver - ////////////////////////////////////

namespace mu2e {

  // =====================================================================================
  // Constructor & Destructor
  // =====================================================================================
  STMTCPReceiver::STMTCPReceiver(fhicl::ParameterSet const& ps)
    : artdaq::CommandableFragmentGenerator(ps)
    , chan_(ps.get<int>("channelNumber", 0)) // Data channel (0=HPGE, 1=LaBr3)
    , host_(ps.get<std::string>("ip_address", "127.0.0.2")) // I.P. host of TCP socket
    , port_(ps.get<int>("port", 10025)) // Port of TCP socket
    , rcvbuf_bytes_(ps.get<int>("rcvbuf_bytes", 314572800)) // Buffer bytes (Default: 300*1024*1024)
    , recv_buffer_size_(ps.get<int>("recv_buffer_size", 10485760)) // Size of the temporary TCP receive buffer (Default: 10*1024*1024)
    , event_anchor_word_(ps.get<uint16_t>("event_anchor_word", 51966)) // Anchor word in Event header (0xCAFE)
    , start_fragment_id_(ps.get<uint64_t>("start_fragment_id", 100)) // Base fragment id
    , raw_stream_id_(ps.get<uint64_t>("raw_stream_id", 0)) // RAW fragment id
    , zs_stream_id_(ps.get<uint64_t>("zs_stream_id", 1)) // ZS fragment id
    , ph_stream_id_(ps.get<uint64_t>("ph_stream_id", 2)) // PH fragment id
    , container_stream_id_(ps.get<uint64_t>("container_stream_id", 3)) // Container fragment id
    , debug_level_(ps.get<int>("debug_level", 0)) // Set debug levels
    , pin_threads_(ps.get<bool>("pin_threads", true)) // Pin thread to specific cores
    , events_per_container_(ps.get<size_t>("events_per_container", 500)) // Default number of events per container in getNext()
    , offspill_events_per_container_(ps.get<size_t>("offspill_events_per_container", 100)) // Number of off-spill events per container in getNext()
    , use_spill_condition_(ps.get<bool>("use_spill_condition", false)) // Condition for batching on spill flags
  {
    // Log configuration
    TLOG_DEBUG(1) << "STMTCPReceiver: Channel = " << chan_ << " | Host = " << host_ << " | Port = " << port_
                  << " rcvbuf=" << rcvbuf_bytes_;
    TLOG_DEBUG(1) << "STMTCPReceiver: recv_to_parser queue capacity=" << DEFAULT_RECV_QCAP
                  << ", parser_to_cons queue capacity=" << DEFAULT_BATCH_QCAP;  

    // Create lockfree SPSC queues
    recv_to_parser_queue_     = std::make_unique<RecvToParserQ>();
    batcher_to_builder_queue_ = std::make_unique<BatcherToBuildQ>();
    builder_to_getNext_queue_ = std::make_unique<BuilderToGNQ>();

    // Create ring buffer
    ring_ = std::make_unique<EventRingBuffer>(1024ULL * 1024 * 1024); // 1GB
    
  }

  STMTCPReceiver::~STMTCPReceiver() {
    // Signal end of datastream
    receiver_done_.store(true);
    batcher_cv_.notify_all();
    builder_cv_.notify_all();

    // Wake up blocking calls
    if (listen_fd_ >= 0) ::shutdown(listen_fd_, SHUT_RDWR);
    if (client_fd_ >= 0) ::shutdown(client_fd_, SHUT_RDWR);

    // Close all threads and socket
    if (receiver_thread_.joinable()) receiver_thread_.join();
    if (parser_thread_.joinable()) parser_thread_.join();
    if (builder_thread_.joinable()) builder_thread_.join();
    if (listen_fd_ >= 0) ::close(listen_fd_);
    if (client_fd_ >= 0) ::close(client_fd_);
  }

  // =====================================================================================
  // Start function
  // =====================================================================================
  void STMTCPReceiver::start() {
    // NOTE:
    // start() MUST NOT block in ART. We ONLY create the listening socket here.
    // accept() and recv() happen in the receiver thread.
    
    TLOG(TLVL_INFO) << "Starting STMTCPReceiver, opening listening socket... host="
                    << host_ << " port=" << port_;

    // Start global timer
    run_start_time_ = clock::now();

    // Setup socket
    listen_fd_ = setup_tcp_socket(host_, port_, rcvbuf_bytes_);
    if (listen_fd_ < 0) {
      TLOG(TLVL_ERROR) << "Failed to set up TCP listening socket";
      throw cet::exception("STMTCPReceiver") << "Failed to set up TCP listening socket";
    }

    receiver_done_.store(false);

    // Start receiver thread (handles accept + recv)
    receiver_thread_ = std::thread(&STMTCPReceiver::receiverThread_, this);
    if (pin_threads_) pin_thread_to_least_busy_core(receiver_thread_, "[RECEIVER]");

    // Start parser thread
    parser_thread_ = std::thread(&STMTCPReceiver::parserThread_, this);
    if (pin_threads_) pin_thread_to_least_busy_core(parser_thread_, "[PARSER]");

    // Start batcher thread
    builder_thread_ = std::thread(&STMTCPReceiver::builderThread_, this);
    if (pin_threads_) pin_thread_to_least_busy_core(builder_thread_, "[BUILDER]");
  }

  // =====================================================================================
  // Receiver Thread: Accepts TCP connection and reads data with idle timeout
  // =====================================================================================
  void mu2e::STMTCPReceiver::receiverThread_()
  {
    TLOG(TLVL_INFO) << "[RECEIVER] Receiver thread started";
    TLOG(tcpDebugLevel()) << "[RECEIVER] Waiting for TCP client connection...";

    struct pollfd lfd{};
    lfd.fd = listen_fd_;
    lfd.events = POLLIN;

    while (!receiver_done_.load()) {

      int ret = poll(&lfd, 1, 100);

      if (ret < 0) {
        if (errno == EINTR) continue;
        perror("poll(listen_fd)");
        return;
      }

      if (ret > 0 && (lfd.revents & POLLIN)) {
        client_fd_ = accept(listen_fd_, nullptr, nullptr);
        break;
      }
    }

    if (client_fd_ < 0) {
      TLOG(TLVL_ERROR) << "[RECEIVER] accept failed";
      receiver_done_.store(true);
      return;
    }

    TLOG(TLVL_INFO) << "[RECEIVER] Client connected (fd=" << client_fd_ << ")";

    int flags = fcntl(client_fd_, F_GETFL, 0);
    fcntl(client_fd_, F_SETFL, flags | O_NONBLOCK);

    size_t rmem_max = get_rmem_max();

    if (rmem_max > 0) {

      int bufsize = static_cast<int>(rmem_max);

      if (setsockopt(client_fd_, SOL_SOCKET,
                     SO_RCVBUF, &bufsize,
                     sizeof(bufsize)) < 0) {
        perror("setsockopt(SO_RCVBUF)");
      }

      socklen_t len = sizeof(bufsize);

      getsockopt(client_fd_,
                 SOL_SOCKET,
                 SO_RCVBUF,
                 &bufsize,
                 &len);

      TLOG(TLVL_INFO)
        << "[RECEIVER] SO_RCVBUF set to "
        << bufsize << " bytes";
    }

    std::vector<uint8_t> byte_buffer(recv_buffer_size_);

    bool received_first_packet = false;
    bool eof = false;

    struct pollfd pfd{};
    pfd.fd = client_fd_;
    pfd.events = POLLIN;

    while (!receiver_done_.load() && !eof) {

      int ret = poll(&pfd, 1, 100);

      if (ret == 0) continue;

      if (ret < 0) {
        if (errno == EINTR) continue;
        perror("poll(client_fd)");
        break;
      }

      if (pfd.revents & POLLIN) {

        auto work_start = clock::now();

        for (;;) {

          ssize_t n = recv(client_fd_,
                           byte_buffer.data(),
                           byte_buffer.size(),
                           0);

          if (n > 0) {

            if (!received_first_packet) {
              received_first_packet = true;
              TLOG(tcpDebugLevel())
                << "[RECEIVER] First packet received";
            }

            static uint64_t total_bytes = 0;
            static uint64_t total_calls = 0;

            total_bytes += n;
            total_calls++;

            if (total_calls % 10000 == 0) {
              TLOG(TLVL_INFO)
                << "[RECV] avg recv size = "
                << (total_bytes / total_calls);
            }

            total_bytes_received_.fetch_add(
                                            n, std::memory_order_relaxed);

            // -----------------------------
            // SAFE RING BUFFER WRITE
            // -----------------------------

            size_t remaining = n;
            size_t copied = 0;

            while (remaining > 0) {

              size_t writable = ring_->writable();

              if (writable == 0) {
                std::this_thread::sleep_for(
                                            std::chrono::microseconds(1));
                continue;
              }

              size_t contiguous =
                ring_->contiguousWritable();

              size_t to_copy =
                std::min({remaining,
                    contiguous,
                    writable});

              uint8_t* dst = ring_->writePtr();

              std::memcpy(dst,
                          byte_buffer.data() + copied,
                          to_copy);

              ring_->advanceWrite(to_copy);

              copied += to_copy;
              remaining -= to_copy;
            }

            continue; // drain socket
          }

          if (n == 0) {
            TLOG(TLVL_INFO)
              << "[RECEIVER] Client closed connection (EOF)";
            eof = true;
            break;
          }

          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
          }

          perror("recv");
          eof = true;
          break;
        }

        receiver_active_ +=
          (clock::now() - work_start);

        receiver_loops_.fetch_add(
                                  1, std::memory_order_relaxed);
      }

      if (pfd.revents &
          (POLLHUP | POLLERR | POLLNVAL)) {

        TLOG(TLVL_INFO)
          << "[RECEIVER] Socket hangup/error";

        eof = true;
      }
    }

    if (client_fd_ >= 0) {
      shutdown(client_fd_, SHUT_RDWR);
      close(client_fd_);
      client_fd_ = -1;
    }

    receiver_done_.store(true,std::memory_order_release);

    TLOG(TLVL_INFO)
      << "[RECEIVER] Receiver thread exiting";
  }
  
  // =====================================================================================
  // Parser Thread: Assembles events across chunk boundaries puts into batches
  // =====================================================================================
  void mu2e::STMTCPReceiver::parserThread_()
  {
    TLOG(TLVL_INFO) << "[PARSER] Parser+Batcher thread started";

    auto make_new_batch = [&] {
      EventBatch b;
      b.events.reserve(events_per_container_);
      b.container_frag_id = start_fragment_id_ + container_stream_id_;
      b.container_seq_id  = 0;
      return b;
    };

    EventBatch batch = make_new_batch();
    uint16_t batch_spill_flag = 0;

    alignas(64) uint8_t wrap_buffer[65536];
    uint8_t header_tmp[EVENT_HEADER_BYTES];

    while (true){
        size_t available = ring_->readable();

        if (available < EVENT_HEADER_BYTES){
            if (receiver_done_.load()) break;
            _mm_pause();
            continue;
          }

        const uint8_t* ptr = ring_->readPtr();
        size_t contiguous = ring_->contiguousReadable();

        // -------------------------
        // Safe header read
        // -------------------------

        const uint16_t* hdr;

        if (contiguous >= EVENT_HEADER_BYTES){
            hdr = reinterpret_cast<const uint16_t*>(ptr);
          }
        else{
            std::memcpy(header_tmp, ptr, contiguous);

            std::memcpy(header_tmp + contiguous,
                        ring_->begin(),
                        EVENT_HEADER_BYTES - contiguous);

            hdr = reinterpret_cast<const uint16_t*>(header_tmp);
          }

        // -------------------------
        // Anchor validation
        // -------------------------
        if (hdr[anchor_start] != event_anchor_word_ ||
            hdr[anchor_end] != event_anchor_word_)
          {
            ring_->advanceRead(2);
            continue;
          }

        // -------------------------
        // Decode header
        // -------------------------

        uint16_t ps = hdr[PRESCALE];

        bool raw_prescaled = (ps >> 15) & 1;
        bool zs_prescaled  = (ps >> 7) & 1;

        uint16_t raw_len = hdr[RAW_LEN];
        uint16_t zs_regs = hdr[ZS_REGIONS];
        uint16_t zs_len  = hdr[ZS_LEN] + zs_regs * 2;
        uint16_t ph_num  = hdr[PH_NUM];
        uint16_t ph_len  = ph_num * 2;

        if (raw_prescaled) raw_len = 0;

        if (zs_prescaled){
            zs_regs = 0;
            zs_len  = 0;
          }

        size_t evt_total_bytes =
          EVENT_HEADER_BYTES +
          size_t(raw_len + zs_len + ph_len) * sizeof(uint16_t);

        if (available < evt_total_bytes){
            _mm_pause();
            continue;
          }

        // -------------------------
        // Handle ring wrap
        // -------------------------

        bool wrapped = contiguous < evt_total_bytes;

        const uint8_t* event_ptr;

        if (!wrapped){
            event_ptr = ptr;
          }
        else{
            size_t first = contiguous;
            size_t second = evt_total_bytes - first;

            std::memcpy(wrap_buffer, ptr, first);

            std::memcpy(wrap_buffer + first,
                        ring_->begin(),
                        second);

            event_ptr = wrap_buffer;
          }

        const uint16_t* event_hdr =
          reinterpret_cast<const uint16_t*>(event_ptr);

        // -------------------------
        // Build EventView
        // -------------------------

        EventView evt;

        evt.data       = event_ptr;
        evt.header     = event_hdr;
        evt.size_bytes = evt_total_bytes;

        evt.event_num =
          int64_t(event_hdr[EWT_0]) |
          (int64_t(event_hdr[EWT_1]) << 16) |
          (int64_t(event_hdr[EWT_2]) << 32);

        uint16_t EM2 = event_hdr[EM_2_DRTDC] & 0xFF;

        uint64_t EM =
          uint64_t(event_hdr[EM_0]) |
          (uint64_t(event_hdr[EM_1]) << 16) |
          (uint64_t(EM2) << 32);

        evt.spill_flag = EM;

        if (!computeDatasetView(evt))
          {
            TLOG(TLVL_ERROR) << "[PARSER] computeDatasetView failed";
            ring_->advanceRead(2);
            continue;
          }

        event_count_.fetch_add(1, std::memory_order_relaxed);

        // -------------------------
        // Batching logic
        // -------------------------

        if (batch.events.empty()){
            batch.container_seq_id = evt.event_num;
            batch_spill_flag = evt.spill_flag;
          }
        else if (use_spill_condition_ &&
                 evt.spill_flag != batch_spill_flag){
            EventBatch flushed = std::move(batch);
            batch = make_new_batch();

            while (!batcher_to_builder_queue_->push(std::move(flushed)))
              _mm_pause();

            batcher_cv_.notify_one();

            batch.container_seq_id = evt.event_num;
            batch_spill_flag = evt.spill_flag;
          }

        batch.events.emplace_back(evt);

        const size_t target_batch_size =
          (batch_spill_flag == 0)
          ? offspill_events_per_container_
          : events_per_container_;

        if (batch.events.size() >= target_batch_size){
            EventBatch full_batch = std::move(batch);
            batch = make_new_batch();

            while (!batcher_to_builder_queue_->push(std::move(full_batch)))
              _mm_pause();

            batcher_cv_.notify_one();
          }

        // -------------------------
        // Advance ring
        // -------------------------
        ring_->advanceRead(evt_total_bytes);
      }

    // -------------------------
    // Final flush
    // -------------------------

    if (!batch.events.empty()){
        EventBatch final_batch = std::move(batch);

        while (!batcher_to_builder_queue_->push(std::move(final_batch)))
          _mm_pause();

        batcher_cv_.notify_one();
      }

    batcher_done_.store(true, std::memory_order_release);
    batcher_cv_.notify_all();

    TLOG(TLVL_INFO) << "[PARSER] Parser+Batcher exiting";
  }
  
  // =====================================================================================
  // Builder Thread: Assembles event batchers into fragment containers
  // =====================================================================================
  void mu2e::STMTCPReceiver::builderThread_()
  {
    TLOG(TLVL_INFO) << "[BUILDER] Builder thread started";

    EventBatch batch;

    while (true)
      {
        if (!batcher_to_builder_queue_->pop(batch))
          {
            if (batcher_done_.load(std::memory_order_acquire) &&
                batcher_to_builder_queue_->empty())
              break;

            _mm_pause();
            continue;
          }

        auto work_start = clock::now();

        // ------------------------------------------------
        // Create container fragment
        // ------------------------------------------------

        auto* container_frag = new artdaq::Fragment();

        container_frag->setSequenceID(batch.container_seq_id);
        container_frag->setFragmentID(batch.container_frag_id);

        artdaq::ContainerFragmentLoader loader(
                                               *container_frag,
                                               FragmentType::STM);

        // ------------------------------------------------
        // Collect fragments for this batch
        // ------------------------------------------------

        artdaq::Fragments batch_frags;
        batch_frags.reserve(batch.events.size() * 3);

        for (const auto& e : batch.events)
          {
            const uint64_t seq = e.event_num;

            auto process =
              [&](const DatasetView& ds,
                  uint64_t stream_id,
                  std::atomic<size_t>& counter)
              {
                const uint64_t frag_id =
                  start_fragment_id_ + stream_id;

                std::unique_ptr<artdaq::Fragment> frag;

                if (ds.size == 0)
                  {
                    frag = makeFragment_(frag_id, seq, nullptr, 0);
                  }
                else
                  {
                    const int16_t* ptr = reinterpret_cast<const int16_t*>(e.data + ds.offset);

                    frag = makeFragment_(
                                         frag_id,
                                         seq,
                                         ptr,
                                         ds.size);
                  }

                if (frag)
                  {
                    batch_frags.emplace_back(*frag);
                    counter.fetch_add(1, std::memory_order_relaxed);
                  }
              };

            process(e.raw, raw_stream_id_, raw_frag_count_);
            process(e.zs,  zs_stream_id_,  zs_frag_count_);
            process(e.ph,  ph_stream_id_,  ph_frag_count_);
          }

        // ------------------------------------------------
        // Add fragments to container
        // ------------------------------------------------

        loader.addFragments(batch_frags);

        // ------------------------------------------------
        // Send container to getNext
        // ------------------------------------------------

        while (!builder_to_getNext_queue_->push(container_frag))
          _mm_pause();

        builder_active_ += (clock::now() - work_start);
        builder_loops_.fetch_add(1, std::memory_order_relaxed);
      }

    builder_done_.store(true, std::memory_order_release);

    TLOG(TLVL_INFO) << "[BUILDER] Builder exiting";
  }

  // =====================================================================================
  // getNext_: Send container of fragments to EB
  // =====================================================================================
  bool mu2e::STMTCPReceiver::getNext_(artdaq::FragmentPtrs& frags)
  {

    artdaq::Fragment* raw_ptr = nullptr;
  
    while (true) {
      if (builder_to_getNext_queue_->pop(raw_ptr)){

        // Check queue capacity after successful pull
        /*static uint64_t pull_counter = 0;
          if (++pull_counter % 50 == 0) {
          TLOG(TLVL_INFO)
          << "[getNext] builder_to_getNext_queue fill="
          << builder_to_getNext_queue_->read_available();
          }*/
        
        break;
      }
      
      if (builder_done_.load(std::memory_order_acquire) &&
          builder_to_getNext_queue_->empty())
        return false;
    
      std::this_thread::yield();
    }

    frags.emplace_back(std::unique_ptr<artdaq::Fragment>(raw_ptr));
    return true;
  }
  
  // =====================================================================================
  // Stop function
  // =====================================================================================
  void mu2e::STMTCPReceiver::stop()
  {
    TLOG(TLVL_INFO) << "[STOP] STMTCPReceiver stop() called";

    // Signal threads to exit
    receiver_done_.store(true);

    // Wake up getNext_() if it is waiting
    batcher_cv_.notify_all();
    builder_cv_.notify_all();
    
    // Close sockets to unblock poll()/recv()/accept()
    if (client_fd_ >= 0) {
      ::shutdown(client_fd_, SHUT_RDWR);
      ::close(client_fd_);
      client_fd_ = -1;
    }

    if (listen_fd_ >= 0) {
      ::shutdown(listen_fd_, SHUT_RDWR);
      ::close(listen_fd_);
      listen_fd_ = -1;
    }

    // Join threads safely
    if (receiver_thread_.joinable()) receiver_thread_.join();
    if (parser_thread_.joinable()) parser_thread_.join();
    if (builder_thread_.joinable()) builder_thread_.join();

    // Print summary
    TLOG(TLVL_INFO)
      << "[STOP] Summary: events=" << event_count_.load()
      << " bytes_received=" << total_bytes_received_.load()
      << " | produced RAW/ZS/PH frags= " << raw_frag_count_.load() << " / "
      << zs_frag_count_.load() << " / " << ph_frag_count_.load();

    // Print throughput
    auto total_runtime = clock::now() - run_start_time_;

    double runtime_sec =
      std::chrono::duration<double>(total_runtime).count();

    double receiver_sec =
      std::chrono::duration<double>(receiver_active_).count();

    double parser_sec =
      std::chrono::duration<double>(parser_active_).count();

    double builder_sec =
      std::chrono::duration<double>(builder_active_).count();

    TLOG(TLVL_INFO)
      << "\n===== [STOP] PERFORMANCE REPORT =====\n"
      << "Total runtime: " << runtime_sec << " s\n"
      << "Events: " << event_count_.load() << "\n"
      << "Throughput: "
      << (event_count_.load() / runtime_sec) << " events/sec\n\n"
      << "Receiver active: " << receiver_sec
      << " s (" << (receiver_sec/runtime_sec*100.0) << "%)\n"
      << "Parser active:   " << parser_sec
      << " s (" << (parser_sec/runtime_sec*100.0) << "%)\n"
      << "Builder active:  " << builder_sec
      << " s (" << (builder_sec/runtime_sec*100.0) << "%)\n"
      << "Avg parse time per event: "
      << (parser_sec / event_count_.load()) * 1e6
      << " us\n"
      << "Avg build time per event: "
      << (builder_sec / event_count_.load()) * 1e6
      << " us\n"
      << "=====================================\n";
  }
  
} // namespace mu2e

  // =====================================================================================
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(mu2e::STMTCPReceiver)
