// STMTCPReceiver_generator.cc

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
    , port_(ps.get<int>("port", 10010)) // Port of TCP socket
    , rcvbuf_bytes_(ps.get<int>("rcvbuf_bytes", 33554432)) // Buffer bytes
    , recv_buffer_size_(ps.get<int>("recv_buffer_size", 256000)) // Size of the temporary TCP receive buffer
    , raw_anchor_word_(ps.get<uint16_t>("raw_anchor_word", 48879)) // Anchor word in RAW header (0xBEEF)
    , zs_anchor_word_(ps.get<uint16_t>("zs_anchor_word", 57005)) // Anchor word in ZS header (0xDEAD)
    , mwd_anchor_word_(ps.get<uint16_t>("mwd_anchor_word", 65261)) // Anchor word in MWD header (0xFEED)
    , start_fragment_id_(ps.get<uint64_t>("start_fragment_id", 100)) // Base fragment id
    , raw_stream_id_(ps.get<uint64_t>("raw_stream_id", 0)) // RAW fragment id
    , zs_stream_id_(ps.get<uint64_t>("zs_stream_id", 1)) // ZS fragment id
    , mwd_stream_id_(ps.get<uint64_t>("mwd_stream_id", 2)) // MWD fragment id
    , recv_queue_capacity_(ps.get<size_t>("recv_queue_capacity", DEFAULT_RECV_QCAP)) // Capacity of the recv queue
    , evt_queue_capacity_(ps.get<size_t>("evt_queue_capacity", DEFAULT_EVT_QCAP)) // Capacity of the event queue
    , debug_level_(ps.get<int>("debug_level", 0)) // Set debug levels
    , pin_threads_(ps.get<bool>("pin_threads", true)) // Pin thread to specific cores
    , idle_timeout_ms_(ps.get<int>("idle_timeout_ms", 5000)) // Idle timeout AFTER first packet (ms)
  {
    // Log configuration
    TLOG_DEBUG(1) << "STMTCPReceiver: Channel = " << chan_ << " | Host = " << host_ << " | Port = " << port_
		  << " rcvbuf=" << rcvbuf_bytes_;

    // Create lockfree SPSC queues
    recv_to_parser_queue_     = std::make_unique<RecvToParserQ>();
    parser_to_consumer_queue_ = std::make_unique<ParserToConsQ>();
  }

  STMTCPReceiver::~STMTCPReceiver() {
    // Signal end of datastream
    done_.store(true);

    // Wake up blocking calls
    if (listen_fd_ >= 0) ::shutdown(listen_fd_, SHUT_RDWR);
    if (client_fd_ >= 0) ::shutdown(client_fd_, SHUT_RDWR);

    // Close all threads and socket
    if (receiver_thread_.joinable()) receiver_thread_.join();
    if (parser_thread_.joinable()) parser_thread_.join();
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

    done_.store(false);

    // Start receiver thread (handles accept + recv)
    receiver_thread_ = std::thread(&STMTCPReceiver::receiverThread_, this);
    if (pin_threads_) pin_thread_to_least_busy_core(receiver_thread_, "[RECEIVER]");

    // Start parser thread
    parser_thread_ = std::thread(&STMTCPReceiver::parserThread_, this);
    if (pin_threads_) pin_thread_to_least_busy_core(parser_thread_, "[PARSER]");
  }

  // =====================================================================================
  // Receiver Thread: Accepts TCP connection and reads data with idle timeout
  // =====================================================================================
  void mu2e::STMTCPReceiver::receiverThread_()
  {
    TLOG(TLVL_INFO) << "[RECEIVER] Receiver thread started";
    TLOG(tcpDebugLevel()) << "[RECEIVER] Waiting for TCP client connection...";

    // --- Poll-based accept (interruptible) ---
    struct pollfd lfd;
    lfd.fd = listen_fd_;
    lfd.events = POLLIN;

    while (!done_.load()) {
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

    if (done_.load() || client_fd_ < 0) {
      TLOG(TLVL_INFO) << "[RECEIVER] Exiting before accept completed";
      return;
    }

    TLOG(TLVL_INFO) << "[RECEIVER] Client connected (fd=" << client_fd_ << ")";

    // Make non-blocking
    int flags = fcntl(client_fd_, F_GETFL, 0);
    fcntl(client_fd_, F_SETFL, flags | O_NONBLOCK);

    int bufsize = rcvbuf_bytes_;
    setsockopt(client_fd_, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

    std::vector<uint8_t> byte_buffer(recv_buffer_size_);
    bool received_first_packet = false;
    auto last_recv_time = std::chrono::steady_clock::now();

    struct pollfd pfd;
    pfd.fd = client_fd_;
    pfd.events = POLLIN;

    while (!done_.load()) {
      int ret = poll(&pfd, 1, 50);

      if (ret == 0) {
	if (received_first_packet && idle_timeout_ms_ > 0) {
	  auto now = std::chrono::steady_clock::now();
	  auto elapsed =
	    std::chrono::duration_cast<std::chrono::milliseconds>(
								  now - last_recv_time).count();

	  if (elapsed > idle_timeout_ms_) {
	    TLOG(TLVL_INFO) << "[RECEIVER] Idle timeout reached, closing client";
	    ::shutdown(client_fd_, SHUT_RDWR);
	    ::close(client_fd_);
	    client_fd_ = -1;
	    break;
	  }
	}
	continue;
      }

      if (ret < 0) {
	if (errno == EINTR) continue;
	perror("poll(client_fd)");
	break;
      }

      if (pfd.revents & POLLIN) {
	ssize_t n = recv(client_fd_, byte_buffer.data(), byte_buffer.size(), 0);

	if (n <= 0) {
	  TLOG(TLVL_INFO) << "[RECEIVER] Client closed connection";
	  break;
	}

	if (!received_first_packet) {
	  received_first_packet = true;
	  TLOG(tcpDebugLevel())
	    << "[RECEIVER] First packet received, starting idle timeout monitoring";
	}

	last_recv_time = std::chrono::steady_clock::now();
	total_bytes_received_.fetch_add(n, std::memory_order_relaxed);

	auto bulk =
	  std::make_shared<std::vector<uint8_t>>(byte_buffer.begin(),
						 byte_buffer.begin() + n);

	while (!recv_to_parser_queue_->push(bulk)) {
	  if (done_.load()) break;
	  std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
      }

      if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
	TLOG(TLVL_INFO) << "[RECEIVER] Socket error/hangup";
	break;
      }
    }

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

    while (!done_.load() || !recv_to_parser_queue_->empty()) {
      if (!recv_to_parser_queue_->pop(incoming)) {
	std::this_thread::yield();
	continue;
      }

      chunks.push_back(incoming);

      if (chunks.empty()) {
	offset_in_first_chunk = 0;
	continue;
      }

      // --- Assemble events ---
      while (!chunks.empty()) {
	size_t total_bytes = 0;
	for (auto& c : chunks) total_bytes += c->size();
	total_bytes -= offset_in_first_chunk;

	if (total_bytes < EVENT_HEADER_BYTES) {
	  TLOG(TLVL_DEBUG) << "[PARSER] Not enough bytes for header: total_bytes="
			   << total_bytes << ", waiting for more chunks";
	  break; // wait for more TCP data
	}

	const uint8_t* header_ptr = nullptr;
	uint8_t header_tmp[EVENT_HEADER_BYTES];

	if (!getEventHeaderPtr(header_ptr, header_tmp, chunks, offset_in_first_chunk)) {
	  TLOG(TLVL_DEBUG) << "[PARSER] Incomplete header, waiting for more data";
	  break;
	}

	const uint16_t* len_words = reinterpret_cast<const uint16_t*>(header_ptr);
	size_t event_len_bytes = build_len_from_3_words(len_words[0], len_words[1], len_words[2]);
	size_t evt_total_bytes = EVENT_HEADER_BYTES + event_len_bytes;

	TLOG(TLVL_DEBUG) << "[PARSER] total_bytes=" << total_bytes
			 << " evt_total_bytes=" << evt_total_bytes;

	if (total_bytes < evt_total_bytes) {
	  TLOG(TLVL_DEBUG) << "[PARSER] Not enough bytes for full event, need "
			   << evt_total_bytes << " bytes, have " << total_bytes;
	  break; // wait for more chunks
	}

	// --- Build EventView ---
	std::vector<ChunkRef> temp_refs;
	size_t bytes_needed = evt_total_bytes;
	size_t off = offset_in_first_chunk;
	size_t total_collected = 0;

	for (size_t i = 0; i < chunks.size() && bytes_needed > 0; ++i) {
	  const auto& chunk = chunks[i];
	  if (off >= chunk->size()) {
	    off = 0;
	    continue;
	  }

	  size_t avail = chunk->size() - off;
	  size_t take  = std::min(avail, bytes_needed);
	  temp_refs.push_back({chunk, off, take});
	  total_collected += take;
	  bytes_needed -= take;
	  off = 0; // only first chunk has offset
	}

	if (bytes_needed != 0) {
	  TLOG(TLVL_ERROR) << "[PARSER] Could not collect full event, collected "
			   << total_collected << " bytes, needed " << evt_total_bytes;
	  break;
	}

	EventView evt;
	uint16_t w11, w12, w13, w16, w19;

	if (!readWordAtEventOffset(temp_refs, 11, w11) ||
	    !readWordAtEventOffset(temp_refs, 12, w12) ||
	    !readWordAtEventOffset(temp_refs, 13, w13) ||
	    !readWordAtEventOffset(temp_refs, 16, w16) ||
	    !readWordAtEventOffset(temp_refs, 19, w19)) {
	  TLOG(TLVL_ERROR) << "[PARSER] Failed to read event ID/spill flag";
	  done_.store(true);
	  return;
	}

	evt.buffer_chunks = std::move(temp_refs);
	evt.event_num = (int64_t)w11 | ((int64_t)w12 << 16) | ((int64_t)w13 << 32);
	evt.template_id = (int16_t)w16;
	evt.spill_flag  = (int16_t)w19;

	if (!computeDatasetView(evt.buffer_chunks, evt)) {
	  TLOG(TLVL_ERROR) << "[PARSER] Failed to compute dataset sizes";
	  done_.store(true);
	  return;
	}

	evt.size_bytes = evt_total_bytes;

	// --- Push to consumer queue ---
	size_t push_attempts = 0;
	while (!parser_to_consumer_queue_->push(evt)) {
	  push_attempts++;
	  if (push_attempts % 1000 == 0) std::this_thread::sleep_for(std::chrono::microseconds(1));
	  if (done_.load()) break;
	}

	TLOG(TLVL_DEBUG) << "[PARSER] Event pushed to consumer queue, event_num=" << evt.event_num
			 << ", evt_total_bytes=" << evt_total_bytes
			 << ", push_attempts=" << push_attempts;

	event_count_.fetch_add(1);

	// --- Advance deque ---
	offset_in_first_chunk += evt_total_bytes;
	auto it = chunks.begin();
	while (it != chunks.end() && offset_in_first_chunk >= (*it)->size()) {
	  offset_in_first_chunk -= (*it)->size();
	  it = chunks.erase(it);
	}
      }
    }

    TLOG(TLVL_INFO) << "[PARSER] Parser thread exiting";
  }

  // =====================================================================================
  // getNext_: Convert parsed events to artdaq::Fragments
  // =====================================================================================
  bool STMTCPReceiver::getNext_(artdaq::FragmentPtrs& frags)
  {
    TLOG(TLVL_INFO) << "[getNext_] Entering getNext_()";

    EventView evt;
    bool made_any = false;
    const bool done = done_.load(std::memory_order_acquire);

    // --- Check ARTDAQ stop signal first ---
    if (should_stop()) {
      TLOG(TLVL_INFO) << "[getNext_] should_stop() triggered, exiting immediately";
      done_.store(true, std::memory_order_release);
      return false;
    }

    // --- Process all available events in the parser queue ---
    while (parser_to_consumer_queue_->pop(evt)) {

      uint64_t seq_id = static_cast<uint64_t>(evt.event_num);
      
      // Lambda to process any dataset type safely
      auto process_dataset = [&](DatasetView& ds, uint64_t stream_id, uint64_t seq) {

        if (ds.size > 0) {
	  auto p = try_get_single_chunk_ptr(evt, ds);
	  if (p.second) {
	    makeAndEmplaceFragment_(frags,
				    start_fragment_id_ + chan_ + stream_id,
				    seq,
				    p.first,
				    ds.size);
	  } else {
	    auto tmp = copy_dataset_to_temp(evt, ds);
	    makeAndEmplaceFragment_(frags,
				    start_fragment_id_ + chan_ + stream_id,
				    seq,
				    tmp.data(),
				    tmp.size());
	  }
        } else {
	  // Always produce an empty fragment if no data
	  makeAndEmplaceFragment_(frags,
				  start_fragment_id_ + chan_ + stream_id,
				  seq,
				  nullptr,
				  0);
        }
      };

      process_dataset(evt.raw, raw_stream_id_, seq_id);
      process_dataset(evt.zs,  zs_stream_id_, seq_id);
      process_dataset(evt.mwd, mwd_stream_id_, seq_id);
      
      raw_frag_count_.fetch_add(1);
      zs_frag_count_.fetch_add(1);
      mwd_frag_count_.fetch_add(1);

      TLOG(TLVL_DEBUG) << "[getNext_] Event processed, event_num=" << evt.event_num
		       << " | seq_id=" << seq_id;

      made_any = true;
    }

    // --- No events available right now ---
    if (!made_any) {
      if (!done) {
	TLOG(TLVL_DEBUG) << "[getNext_] No events available yet, yielding";
	std::this_thread::sleep_for(std::chrono::milliseconds(50)); // reduce CPU spinning
	return true; // keep getNext being called
      } else {
	TLOG(TLVL_INFO) << "[getNext_] Exiting getNext_() - done and no events";
	return false; // done and queue empty -> proper stop
      }
    }

    TLOG(TLVL_INFO) << "[getNext_] Exiting getNext_() - made_any=" << made_any;
    return made_any;
  }

  // =====================================================================================
  // Stop function
  // =====================================================================================
  void mu2e::STMTCPReceiver::stop()
  {
    TLOG(TLVL_INFO) << "[STOP] STMTCPReceiver stop() called";

    // Signal threads to exit
    done_.store(true);

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

    TLOG(TLVL_INFO)
      << "[STOP] Summary: events=" << event_count_.load()
      << " bytes_received=" << total_bytes_received_.load()
      << " | produced RAW/ZS/MWD frags= " << raw_frag_count_.load() << " / "
      << zs_frag_count_.load() << " / " << mwd_frag_count_.load();
  }

} // namespace mu2e

// =====================================================================================
DEFINE_ARTDAQ_COMMANDABLE_GENERATOR(mu2e::STMTCPReceiver)
