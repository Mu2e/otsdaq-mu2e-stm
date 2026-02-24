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
    , recv_buffer_size_(ps.get<int>("recv_buffer_size", 512000)) // Size of the temporary TCP receive buffer (Default: 2*1024*1024)
    , event_anchor_word_(ps.get<uint16_t>("event_anchor_word", 51966)) // Anchor word in Event header (0xCAFE)
    , start_fragment_id_(ps.get<uint64_t>("start_fragment_id", 100)) // Base fragment id
    , raw_stream_id_(ps.get<uint64_t>("raw_stream_id", 0)) // RAW fragment id
    , zs_stream_id_(ps.get<uint64_t>("zs_stream_id", 1)) // ZS fragment id
    , ph_stream_id_(ps.get<uint64_t>("ph_stream_id", 2)) // PH fragment id
    , container_stream_id_(ps.get<uint64_t>("container_stream_id", 3)) // Container fragment id
    , debug_level_(ps.get<int>("debug_level", 0)) // Set debug levels
    , pin_threads_(ps.get<bool>("pin_threads", true)) // Pin thread to specific cores
    , idle_timeout_ms_(ps.get<int>("idle_timeout_ms", 5000)) // Idle timeout AFTER first packet (ms)
    , events_per_container_(ps.get<size_t>("events_per_container", 600)) // Default number of events per container in getNext()
    , offspill_events_per_container_(ps.get<size_t>("offspill_events_per_container", 100)) // Number of off-spill events per container in getNext()
    , use_spill_condition_(ps.get<bool>("use_spill_condition", false)) // Condition for batching on spill flags
  {
    // Log configuration
    TLOG_DEBUG(1) << "STMTCPReceiver: Channel = " << chan_ << " | Host = " << host_ << " | Port = " << port_
                  << " rcvbuf=" << rcvbuf_bytes_;
    TLOG_DEBUG(1) << "STMTCPReceiver: recv_to_parser queue capacity=" << DEFAULT_RECV_QCAP
                  << ", parser_to_cons queue capacity=" << DEFAULT_EVT_QCAP;  

    // Create lockfree SPSC queues
    recv_to_parser_queue_     = std::make_unique<RecvToParserQ>();
    parser_to_consumer_queue_ = std::make_unique<ParserToConsQ>();
    batcher_to_getNext_queue_ = std::make_unique<BatcherToGNQ>();
  }

  STMTCPReceiver::~STMTCPReceiver() {
    // Signal end of datastream
    receiver_done_.store(true);
    parser_cv_.notify_all();
    batcher_cv_.notify_all();

    // Wake up blocking calls
    if (listen_fd_ >= 0) ::shutdown(listen_fd_, SHUT_RDWR);
    if (client_fd_ >= 0) ::shutdown(client_fd_, SHUT_RDWR);

    // Close all threads and socket
    if (receiver_thread_.joinable()) receiver_thread_.join();
    if (parser_thread_.joinable()) parser_thread_.join();
    if (batcher_thread_.joinable()) batcher_thread_.join();
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
    batcher_thread_ = std::thread(&STMTCPReceiver::batcherThread_, this);
    if (pin_threads_) pin_thread_to_least_busy_core(batcher_thread_, "[BATCHER]");
  }

  // =====================================================================================
  // Receiver Thread: Accepts TCP connection and reads data with idle timeout
  // =====================================================================================
  void mu2e::STMTCPReceiver::receiverThread_()
  {
    TLOG(TLVL_INFO) << "[RECEIVER] Receiver thread started";
    TLOG(tcpDebugLevel()) << "[RECEIVER] Waiting for TCP client connection...";

    // --- Poll-based accept (interruptible) ---
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

    // --- Non-blocking socket ---
    int flags = fcntl(client_fd_, F_GETFL, 0);
    fcntl(client_fd_, F_SETFL, flags | O_NONBLOCK);

    // Get RMEM from server
    size_t rmem_max = get_rmem_max();

    if (rmem_max > 0) {

      int bufsize = static_cast<int>(rmem_max);

      if (setsockopt(client_fd_,
                     SOL_SOCKET,
                     SO_RCVBUF,
                     &bufsize,
                     sizeof(bufsize)) < 0){
        perror("setsockopt(SO_RCVBUF)");
      }

      // Verify actual size
      socklen_t len = sizeof(bufsize);
      getsockopt(client_fd_,
                 SOL_SOCKET,
                 SO_RCVBUF,
                 &bufsize,
                 &len);

      TLOG(TLVL_INFO)
        << "[RECEIVER] SO_RCVBUF set to "
        << bufsize << " bytes (kernel may double this internally)";
    } else {
      int bufsize = rcvbuf_bytes_;
      setsockopt(client_fd_, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
    }
    
    std::vector<uint8_t> byte_buffer(recv_buffer_size_);
    bool received_first_packet = false;
    auto last_recv_time = std::chrono::steady_clock::now();

    struct pollfd pfd{};
    pfd.fd = client_fd_;
    pfd.events = POLLIN;

    bool eof = false;

    while (!receiver_done_.load() && !eof) {
      int ret = poll(&pfd, 1, 100);

      if (ret == 0) continue;

      if (ret < 0) {
        if (errno == EINTR) continue;
        perror("poll(client_fd)");
        break;
      }

      if (pfd.revents & POLLIN) {
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

            last_recv_time = std::chrono::steady_clock::now();
            total_bytes_received_.fetch_add(n, std::memory_order_relaxed);

            auto bulk = std::make_shared<std::vector<uint8_t>>(
                                                               byte_buffer.begin(),
                                                               byte_buffer.begin() + n);

            while (!recv_to_parser_queue_->push(bulk)) {
              if (receiver_done_.load()) break;
              std::this_thread::sleep_for(std::chrono::microseconds(1));
            }

            continue; // try draining socket
          }

          if (n == 0) {
            TLOG(TLVL_INFO) << "[RECEIVER] Client closed connection (EOF)";
            eof = true;
            break;
          }

          // n < 0
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break; // no more data right now
          }

          perror("recv");
          eof = true;
          break;
        }
      }

      if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
        TLOG(TLVL_INFO) << "[RECEIVER] Socket hangup/error";
        eof = true;
      }
    }

    // --- Clean shutdown ---
    if (client_fd_ >= 0) {
      ::shutdown(client_fd_, SHUT_RDWR);
      ::close(client_fd_);
      client_fd_ = -1;
    }

    // --- Tell parser we're done ---
    // Sentinel: nullptr means end-of-stream
    while (!recv_to_parser_queue_->push(nullptr)) {
      if (receiver_done_.load()) break;
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    receiver_done_.store(true, std::memory_order_release);
    parser_cv_.notify_all();

    TLOG(TLVL_INFO) << "[RECEIVER] Receiver thread exiting";
  }
  
  // =====================================================================================
  // Parser Thread: Assembles events across chunk boundaries
  // =====================================================================================
  void mu2e::STMTCPReceiver::parserThread_()
  {
    TLOG(TLVL_INFO) << "[PARSER] Parser thread started";
 
    std::deque<std::shared_ptr<std::vector<uint8_t>>> chunks;
    size_t offset_in_first_chunk = 0;
    std::shared_ptr<std::vector<uint8_t>> incoming;

    while (true) {

      // ------------------------------------------------------------
      // Try to get new input from receiver
      // ------------------------------------------------------------
      if (!recv_to_parser_queue_->pop(incoming)) {

        // Receiver is finished AND no more input possible
        if (receiver_done_.load(std::memory_order_acquire) &&
            recv_to_parser_queue_->empty() &&
            chunks.empty()) {
          break; }

        std::this_thread::yield();
        continue;
      }

      // ------------------------------------------------------------
      // Sentinel from receiver = no more chunks will arrive
      // ------------------------------------------------------------
      if (!incoming) {
        // Receiver is done; no more chunks will arrive
        continue;
      }

      chunks.push_back(incoming);

      // ------------------------------------------------------------
      // Assemble events from accumulated chunks
      // ------------------------------------------------------------
      while (!chunks.empty()) {

        size_t total_bytes = 0;
        for (auto& c : chunks) total_bytes += c->size();
        total_bytes -= offset_in_first_chunk;

        if (total_bytes < EVENT_HEADER_BYTES) {
          break;
        }

        const uint8_t* header_ptr = nullptr;
        uint8_t header_tmp[EVENT_HEADER_BYTES];

        if (!getEventHeaderPtr(header_ptr, header_tmp,
                               chunks, offset_in_first_chunk)) {          
          break;
        }

        const uint16_t* hdr_words =
          reinterpret_cast<const uint16_t*>(header_ptr);

        // --- Decode prescale flags FIRST ---
        uint16_t ps = hdr_words[PRESCALE];
      
        bool raw_prescaled = (ps >> 15) & 0x1;
        bool zs_prescaled  = (ps >> 7)  & 0x1;
      
        // --- Read header lengths ---
        uint16_t raw_len = hdr_words[RAW_LEN];
        uint16_t zs_regs = hdr_words[ZS_REGIONS];
        uint16_t zs_len  = hdr_words[ZS_LEN] + static_cast<uint16_t>(zs_regs * 2);
        uint16_t ph_num  = hdr_words[PH_NUM];
        uint16_t ph_len  = static_cast<uint16_t>(ph_num * 2);
      
        // --- Apply prescale overrides ---
        if (raw_prescaled) {
          raw_len = 0;
        }
      
        if (zs_prescaled) {
          zs_regs = 0;
          zs_len  = 0;
        }

        size_t evt_total_bytes =
          EVENT_HEADER_BYTES +
          static_cast<size_t>(raw_len + zs_len + ph_len) *
          sizeof(uint16_t);

        if (total_bytes < evt_total_bytes) {
          break;
        }

        // ----------------------------------------------------------
        // Build EventView across chunks
        // ----------------------------------------------------------
        std::vector<ChunkRef> temp_refs;
        size_t bytes_needed = evt_total_bytes;
        size_t off = offset_in_first_chunk;

        for (size_t i = 0; i < chunks.size() && bytes_needed > 0; ++i) {
          const auto& chunk = chunks[i];

          if (off >= chunk->size()) {
            off = 0;
            continue;
          }

          size_t avail = chunk->size() - off;
          size_t take  = std::min(avail, bytes_needed);

          temp_refs.push_back({chunk, off, take});
          bytes_needed -= take;
          off = 0;
        }

        if (bytes_needed != 0) {
          break;
        }

        EventView evt;
        evt.buffer_chunks = std::move(temp_refs);
        evt.size_bytes    = evt_total_bytes;

        if (header_ptr == header_tmp) {
          std::memcpy(evt.header_copy.data(),
                      header_tmp,
                      EVENT_HEADER_BYTES);
          evt.header = evt.header_copy.data();
        } else {
          evt.header = reinterpret_cast<const uint16_t*>(header_ptr);
        }

        const uint16_t* hdr = evt.header;

        evt.event_num =
          int64_t(hdr[EWT_0]) |
          (int64_t(hdr[EWT_1]) << 16) |
          (int64_t(hdr[EWT_2]) << 32);

        evt.template_id = hdr[EM_0];
        evt.spill_flag  = hdr[Ch_DTCclk_0] & 0x1;
        /*TLOG(TLVL_DEBUG) << "[PARSER] event=" << evt.event_num
          << " spill_flag=" << evt.spill_flag
          << " hdr[Ch_DTCclk_0]=" << hdr[Ch_DTCclk_0];*/

        if (!computeDatasetView(evt)) {
          TLOG(TLVL_ERROR) << "[PARSER] computeDatasetView failed, aborting parser";
          parser_done_.store(true, std::memory_order_release);
          parser_cv_.notify_all();
          return;  // ← exit thread completely
        }

        // ----------------------------------------------------------
        // Push event downstream
        // ----------------------------------------------------------
        while (!parser_to_consumer_queue_->push(evt)) {
          std::this_thread::yield();
        }

        parser_cv_.notify_one();

        // ----------------------------------------------------------
        // Advance chunk deque
        // ----------------------------------------------------------
        offset_in_first_chunk += evt_total_bytes;

        auto it = chunks.begin();
        while (it != chunks.end() &&
               offset_in_first_chunk >= (*it)->size()) {
          offset_in_first_chunk -= (*it)->size();
          it = chunks.erase(it);
        }
      }
    }

    // ------------------------------------------------------------
    // Parser finished completely
    // ------------------------------------------------------------
    parser_done_.store(true, std::memory_order_release);
    parser_cv_.notify_all();

    TLOG(TLVL_INFO) << "[PARSER] Parser thread exiting";
  }

  void mu2e::STMTCPReceiver::batcherThread_()
  {
    TLOG(TLVL_INFO) << "[BATCHER] Batcher thread started";

    auto make_new_batch = [&] {
      EventBatch b;
      b.events.reserve(events_per_container_);
      b.container_frag_id = start_fragment_id_ + container_stream_id_;
      b.container_seq_id  = 0;
      return b;
    };

    EventBatch batch = make_new_batch();
    uint16_t batch_spill_flag = 0;

    EventView evt;

    while (true) {

      // ------------------------------------------------------------
      // Consume parsed events
      // ------------------------------------------------------------
      if (parser_to_consumer_queue_->pop(evt)) {

        // First event in a new batch
        if (batch.events.empty()) {
          batch.container_seq_id = evt.event_num;
          batch_spill_flag       = evt.spill_flag;
        }
        // ----------------------------------------------------------
        // Spill transition -> flush immediately
        // ----------------------------------------------------------
        else if (use_spill_condition_ && evt.spill_flag != batch_spill_flag) {

          EventBatch flushed = std::move(batch);
          batch = make_new_batch();

          if (!batcher_to_getNext_queue_->push(std::move(flushed))) {
            TLOG(TLVL_ERROR)
              << "[BATCHER] spill-transition flush failed: output queue full";
            batcher_done_.store(true, std::memory_order_release);
            batcher_cv_.notify_all();
            return;
          }

          batcher_cv_.notify_one();

          // Start new batch with this event
          batch.container_seq_id = evt.event_num;
          batch_spill_flag       = evt.spill_flag;
        }

        batch.events.emplace_back(std::move(evt));

        // ----------------------------------------------------------
        // Spill-dependent batch size
        // ----------------------------------------------------------
        const size_t target_batch_size =
          (batch_spill_flag == 0)
          ? offspill_events_per_container_
          : events_per_container_;

        // ----------------------------------------------------------
        // Emit full batch (NON-BLOCKING)
        // ----------------------------------------------------------
        if (batch.events.size() >= target_batch_size) {

          EventBatch full_batch = std::move(batch);
          batch = make_new_batch();
          batch_spill_flag = 0;

          if (!batcher_to_getNext_queue_->push(std::move(full_batch))) {
            TLOG(TLVL_ERROR)
              << "[BATCHER] batcher_to_getNext_queue FULL - stopping batcher";
            std::this_thread::yield();
          }

          batcher_cv_.notify_one();
        }

        continue;
      }

      // ------------------------------------------------------------
      // Parser finished and no more events
      // ------------------------------------------------------------
      if (parser_done_.load(std::memory_order_acquire) &&
          parser_to_consumer_queue_->empty()) {

        if (!batch.events.empty()) {
          EventBatch final_batch = std::move(batch);

          if (!batcher_to_getNext_queue_->push(std::move(final_batch))) {
            TLOG(TLVL_ERROR)
              << "[BATCHER] final batch lost: output queue full";
          } else {
            batcher_cv_.notify_one();
          }
        }

        batcher_done_.store(true, std::memory_order_release);
        batcher_cv_.notify_all();
        return;
      }

      // ------------------------------------------------------------
      // Wait
      // ------------------------------------------------------------
      std::unique_lock<std::mutex> lk(parser_cv_mutex_);
      parser_cv_.wait(lk, [&] {
        return parser_done_.load(std::memory_order_acquire) ||
          !parser_to_consumer_queue_->empty();
      });
    }
  }

  // =====================================================================================
  // getNext_: Convert batch of parsed events to artdaq::Fragments
  // =====================================================================================
  bool mu2e::STMTCPReceiver::getNext_(artdaq::FragmentPtrs& frags)
  {
    TLOG(TLVL_ERROR)
      << "[getNext_] getNext entered";
    
    EventBatch batch;

    // ------------------------------------------------------------
    // Get a batch (block if needed)
    // ------------------------------------------------------------
    while (true) {

      if (batcher_to_getNext_queue_->pop(batch)) {
        break;  // got a real batch
      }

      if (batcher_done_.load(std::memory_order_acquire) &&
          batcher_to_getNext_queue_->empty()) {
        return false;  // real end of stream
      }

      std::unique_lock<std::mutex> lk(batcher_cv_mutex_);
      batcher_cv_.wait(lk);
    }

    TLOG(TLVL_ERROR)
      << "[getNext_] got batch! Batch(#) = "<< batch_count_.load() << " with size = " << batch.events.size() << " and seqID = " << batch.container_seq_id;
    batch_count_.fetch_add(1);
    
    // ------------------------------------------------------------
    // Build container fragment
    // ------------------------------------------------------------
    auto container_frag = std::make_unique<artdaq::Fragment>();
    container_frag->setSequenceID(batch.container_seq_id);
    container_frag->setFragmentID(batch.container_frag_id);

    artdaq::ContainerFragmentLoader loader(*container_frag,
                                           FragmentType::STM);

    // ------------------------------------------------------------
    // Loop over events in batch
    // ------------------------------------------------------------
    for (const auto& e : batch.events) {

      const uint64_t seq = e.event_num;

      auto process = [&](const DatasetView& ds, uint64_t stream_id) {

        const uint64_t frag_id = start_fragment_id_ + stream_id;

        if (ds.size == 0) {
          if (auto frag = makeFragment_(frag_id, seq, nullptr, 0)) {
            loader.addFragment(*frag);
          }
          return;
        }

        auto p = try_get_single_chunk_ptr(e, ds);
        if (p.second) {
          if (auto frag = makeFragment_(frag_id, seq, p.first, ds.size)) {
            loader.addFragment(*frag);
          }
        } else {
          auto tmp = copy_dataset_to_temp(e, ds);
          if (auto frag = makeFragment_(frag_id, seq, tmp.data(), tmp.size())) {
            loader.addFragment(*frag);
          }
        }
      };

      process(e.raw, raw_stream_id_);
      process(e.zs,  zs_stream_id_);
      process(e.ph,  ph_stream_id_);

      event_count_.fetch_add(1);
      raw_frag_count_.fetch_add(1);
      zs_frag_count_.fetch_add(1);
      ph_frag_count_.fetch_add(1);
    }

    frags.emplace_back(std::move(container_frag));
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
    parser_cv_.notify_all();
    batcher_cv_.notify_all();
    
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
    if (batcher_thread_.joinable()) batcher_thread_.join();

    TLOG(TLVL_INFO)
      << "[STOP] Summary: events=" << event_count_.load()
      << " bytes_received=" << total_bytes_received_.load()
      << " | produced RAW/ZS/PH frags= " << raw_frag_count_.load() << " / "
      << zs_frag_count_.load() << " / " << ph_frag_count_.load();
  }

} // namespace mu2e

  // =====================================================================================
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(mu2e::STMTCPReceiver)
