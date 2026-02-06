#ifndef STMTCPRECEIVER_HH
#define STMTCPRECEIVER_HH

// STMTCPReceiver.hh

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
// Trace includes
// =====================================================================================
#include "trace.h"
#define TRACE_NAME "STMTCPReceiver"

// =====================================================================================
// Helper headers
// =====================================================================================
#include "sys_utils.hh"   // Sys utils ( e.g. pin_thread_to_least_busy_core(std::thread&, const char*) )
#include "tcp_utils.hh"   // General utils ( e.g. variable definitions )
#include "tcp_event_header.hh"  // Header defintion

// =====================================================================================
// Generator
// =====================================================================================

namespace mu2e {

  class STMTCPReceiver : public artdaq::CommandableFragmentGenerator {
  public:
    explicit STMTCPReceiver(fhicl::ParameterSet const& ps);
    ~STMTCPReceiver() override;

    FragmentType GetFragmentType() { return fragment_type_; }

  private:
    // ----------------- artDAQ -----------------
    bool getNext_(artdaq::FragmentPtrs& output) override;
    void start() override;
    void stopNoMutex() override {}
    void stop() override;

    FragmentType fragment_type_{FragmentType::STM};  // Type of fragment (see FragmentType.hh)

    // ----------------- Config -----------------
    int chan_;                      // Data channel (0=HPGE, 1=LaBr3)
    std::string host_;              // I.P. host of TCP socket
    int port_;                      // Port of TCP socket
    int rcvbuf_bytes_;              // Socket receive buffer bytes
    int recv_buffer_size_;          // Size of the temporary TCP receive buffer
    uint16_t event_anchor_word_;    // Anchor word in Event header
    uint64_t start_fragment_id_;    // Base fragment id
    uint64_t raw_stream_id_;        // RAW fragment id
    uint64_t zs_stream_id_;         // ZS fragment id
    uint64_t mwd_stream_id_;        // MWD fragment id
    size_t   recv_queue_capacity_;  // Capacity of the recv queue (fixed at compile time)
    size_t   evt_queue_capacity_;   // Capacity of the event queue (fixed at compile time)
    int      debug_level_;          // Debug verbosity
    bool     pin_threads_;          // Pin thread to specific cores
    int      idle_timeout_ms_;      // Idle timeout after first packet (ms)
    
    // ----------------- Runtime -----------------
    std::atomic<bool> done_{false}; // Threads stop flag
    std::atomic<size_t> event_count_{0}; // Number of events processed
    std::atomic<size_t> raw_frag_count_{0}; // Number of raw frags produced
    std::atomic<size_t> zs_frag_count_{0}; // Number of ZS frags produced
    std::atomic<size_t> mwd_frag_count_{0}; // Number of MWD frags produced
    std::atomic<size_t> total_words_parsed_{0}; // Total number of words processed
    std::atomic<uint64_t> total_bytes_received_{0}; // Total bytes received over TCP
    int gNextCounter; // Counter for getNext function

    // TCP sockets
    int listen_fd_{-1}; // Listening TCP socket fd (created in start())
    int client_fd_{-1}; // Accepted client connection fd

    // TCP debug helper
    int tcpDebugLevel() const { return debug_level_ + 5; }
    
    // Queue capacity
    static constexpr size_t DEFAULT_RECV_QCAP = 2621;
    static constexpr size_t DEFAULT_EVT_QCAP  = 2621;

    // Receiver -> Parser (raw bulk byte buffers)
    using RecvToParserQ = boost::lockfree::spsc_queue<std::shared_ptr<std::vector<uint8_t>>, boost::lockfree::capacity<DEFAULT_RECV_QCAP>>;
    // Parser -> Consumer (parsed event objects)
    using ParserToConsQ = boost::lockfree::spsc_queue<EventView, boost::lockfree::capacity<DEFAULT_EVT_QCAP>>;

    std::unique_ptr<RecvToParserQ> recv_to_parser_queue_;
    std::unique_ptr<ParserToConsQ> parser_to_consumer_queue_;

    // ----------------- Threads -----------------
    std::thread receiver_thread_;
    std::thread parser_thread_;

    // Thread functions
    void receiverThread_();
    void parserThread_();

    // ----------------- Helpers -----------------

    // --- Header Fetch ---
    bool getEventHeaderPtr(const uint8_t*& header_ptr,uint8_t* header_tmp,
                           const std::deque<std::shared_ptr<std::vector<uint8_t>>>& chunks,size_t offset_in_first_chunk) const
    {
      if (chunks.empty()) return false;
  
      // How many bytes remain in the first chunk from current offset
      size_t first_avail = chunks.front()->size() - offset_in_first_chunk;

      // --- FAST PATH: header fully inside the first chunk ---
      if (first_avail >= EVENT_HEADER_BYTES) {
        header_ptr = chunks.front()->data() + offset_in_first_chunk;
        return true;
      }

      // --- SLOW PATH: header spans multiple chunks ---
      size_t copied = 0;
      size_t local_off = offset_in_first_chunk;
      for (auto it = chunks.begin(); it != chunks.end() && copied < EVENT_HEADER_BYTES; ++it) {
        size_t avail = (*it)->size() - local_off;
        size_t take  = std::min(avail, EVENT_HEADER_BYTES - copied);
        std::memcpy(header_tmp + copied, (*it)->data() + local_off, take);
        copied += take;
        local_off = 0;
      }

      if (copied < EVENT_HEADER_BYTES) return false; // incomplete header
      header_ptr = header_tmp;
      return true;
    }

    // Get single pointer in mem from multiple chunks
    std::pair<const int16_t*, bool> try_get_single_chunk_ptr(const EventView& evt, const DatasetView& ds) 
    {
      // Convert dataset word offset/size to byte offset/size and account for event header  
      const bool is_raw = (&ds == &evt.raw);

      size_t byte_offset =
	is_raw
	? ds.offset * sizeof(int16_t)
	: ds.offset * sizeof(int16_t) + EVENT_HEADER_BYTES;

      size_t byte_size   = ds.size   * sizeof(int16_t);

      // Iterate over the chunks to locate the dataset
      for (auto const& chunk_ref : evt.buffer_chunks) {
        if (byte_offset < chunk_ref.size) {
          // Dataset starts in this chunk
          if (byte_offset + byte_size <= chunk_ref.size) {
            // Entire dataset fits in this single chunk -> safe for zero-copy
            const uint8_t* base = chunk_ref.chunk->data() + chunk_ref.offset + byte_offset;
            return {reinterpret_cast<const int16_t*>(base), true};
          }
          // Dataset spans multiple chunks. Cannot provide single pointer
          return {nullptr, false};
        }
        // Dataset does not start in this chunk. Skip now
        byte_offset -= chunk_ref.size;
      }

      // Dataset offset exceeds total bytes. Cannot provide single pointer
      return {nullptr, false};
    }

    // Compute the DatasetView offsets and sizes from a fully read event
    bool computeDatasetView(EventView& evt)
    {
      const uint16_t* hdr_words = evt.header;

      if (!hdr_words) {
	TLOG(TLVL_ERROR) << "[PARSER] Null event header pointer";
	return false;
      }

      if (hdr_words[anchor_start] != event_anchor_word_ ||
	  hdr_words[anchor_end]   != event_anchor_word_) {

	TLOG(TLVL_ERROR)
	  << "[PARSER] Anchor mismatch: start=0x"
	  << std::hex << hdr_words[anchor_start]
	  << " end=0x" << hdr_words[anchor_end]
	  << " expected=0x" << event_anchor_word_;
	done_.store(true);
	return false;
      }

      const uint16_t raw_len = hdr_words[RAW_LEN];
      const uint16_t zs_len  = hdr_words[ZS_LEN];
      const uint16_t ph_num  = hdr_words[PH_NUM];

      if (raw_len > MAX_RAW_WORDS ||
	  zs_len  > MAX_ZS_WORDS  ||
	  ph_num  > MAX_MWD_WORDS) {

	TLOG(TLVL_ERROR)
	  << "[RECV] Invalid dataset sizes: RAW=" << raw_len
	  << " ZS=" << zs_len
	  << " PH=" << ph_num;
	return false;
      }

      size_t offset = 0;

      // RAW includes header
      evt.raw = { offset, raw_len + EVENT_HEADER_WORDS };
      offset += raw_len;

      evt.zs  = { offset, zs_len };
      offset += zs_len;

      evt.mwd = { offset, ph_num };

      return true;
    }


    // Make temporary buffer to store dataset (fallback if we cant get single pointer)
    std::vector<int16_t> copy_dataset_to_temp(const EventView& evt, const DatasetView& ds)
    {

      const bool is_raw = (&ds == &evt.raw);

      size_t byte_offset =
	is_raw
	? ds.offset * sizeof(int16_t)
	: ds.offset * sizeof(int16_t) + EVENT_HEADER_BYTES;

      size_t byte_size = ds.size * sizeof(int16_t);

      std::vector<int16_t> tmp(ds.size);
      uint8_t* dst = reinterpret_cast<uint8_t*>(tmp.data());
      size_t copied = 0;

      for (size_t idx = 0; idx < evt.buffer_chunks.size() && copied < byte_size; ++idx)
        {
          const auto& chunk_ref = evt.buffer_chunks[idx];

          if (!chunk_ref.chunk) {
            TLOG(TLVL_ERROR) << "[copy_dataset_to_temp] ERROR: null chunk pointer, idx=" << idx;
            return {};
          }

          if (chunk_ref.offset > chunk_ref.chunk->size()) {
            TLOG(TLVL_ERROR) << "[copy_dataset_to_temp] ERROR: offset=" << chunk_ref.offset
                             << " > chunk size=" << chunk_ref.chunk->size()
                             << " (idx=" << idx << ")";
            return {};
          }

          if (chunk_ref.offset + chunk_ref.size > chunk_ref.chunk->size()) {
            TLOG(TLVL_ERROR) << "[copy_dataset_to_temp] ERROR: offset+size="
                             << (chunk_ref.offset + chunk_ref.size)
                             << " > chunk size=" << chunk_ref.chunk->size()
                             << " (idx=" << idx << ")";
            return {};
          }

          if (byte_offset >= chunk_ref.size) {
            byte_offset -= chunk_ref.size;
            continue;
          }

          size_t avail = chunk_ref.size - byte_offset;
          size_t take  = std::min(avail, byte_size - copied);

          if (chunk_ref.offset + byte_offset + take > chunk_ref.chunk->size()) {
            TLOG(TLVL_ERROR) << "[copy_dataset_to_temp] ERROR: read past end of chunk, idx=" << idx
                             << " off=" << chunk_ref.offset
                             << " byte_off=" << byte_offset
                             << " take=" << take
                             << " chunk_size=" << chunk_ref.chunk->size();
            return {};
          }

          if (copied + take > byte_size) {
            TLOG(TLVL_ERROR) << "[copy_dataset_to_temp] ERROR: write past tmp, copied=" << copied
                             << " take=" << take << " total=" << byte_size;
            return {};
          }

          std::memcpy(dst + copied,
                      chunk_ref.chunk->data() + chunk_ref.offset + byte_offset,
                      take);

          copied += take;
          byte_offset = 0; // only first chunk has offset
        }

      if (copied != byte_size) {
        TLOG(TLVL_ERROR) << "[copy_dataset_to_temp] WARNING: copied=" << copied
                         << " expected=" << byte_size;
      }

      return tmp;
    }


    // Fragment creation helper
    void makeAndEmplaceFragment_(artdaq::FragmentPtrs& out,
                                 uint64_t fragment_id,
                                 uint64_t sequence_id,
                                 const int16_t* data,
                                 size_t n_words,
                                 FragmentType type = FragmentType::STM)
    {
      const size_t n_bytes = n_words * sizeof(int16_t);
      const auto tid = std::this_thread::get_id();

      if (!data && n_bytes > 0) {
        TLOG(TLVL_ERROR) << "[makeAndEmplaceFragment_] ERROR (tid=" << tid
                         << "): null data pointer for " << n_bytes << " bytes" << std::endl;
        done_.store(true);
        return;
      }

      if (n_bytes == 0) {
        TLOG(TLVL_ERROR) << "[makeAndEmplaceFragment_] WARNING (tid=" << tid
                         << "): zero-length fragment requested";
      }

      // Allocate fragment
      auto frag = artdaq::Fragment::FragmentBytes(n_bytes);
      if (!frag) {
        TLOG(TLVL_ERROR) << "[makeAndEmplaceFragment_] ERROR (tid=" << tid
                         << "): failed to allocate fragment of " << n_bytes << " bytes";
        done_.store(true);
        return;
      }

      // Set IDs
      frag->setUserType(type);
      frag->setSequenceID(sequence_id);
      frag->setFragmentID(fragment_id);
      
      // Additional safety check: artdaq Fragment buffer size
      auto frag_buf = frag->dataBeginBytes();
      if (!frag_buf) {
        TLOG(TLVL_ERROR) << "[makeAndEmplaceFragment_] ERROR (tid=" << tid
                         << "): null frag->dataBeginBytes() pointer";
        done_.store(true);
        return;
      }

      // Copy and verify that both ends are valid
      try {
        std::memcpy(frag_buf, reinterpret_cast<const uint8_t*>(data), n_bytes);
      } catch (...) {
        TLOG(TLVL_ERROR) << "[makeAndEmplaceFragment_] EXCEPTION (tid=" << tid
                         << "): memcpy failed for " << n_bytes << " bytes";
        done_.store(true);
        return;
      }

      // Add final debug guard
      TLOG(TLVL_DEBUG) << "[makeAndEmplaceFragment_] INFO (tid=" << tid
                       << "): fragment_id=" << fragment_id
                       << " seq_id=" << sequence_id
                       << " n_words=" << n_words
                       << " (" << n_bytes << " bytes)";

      // Print first 26 words using TLOG
      if (fragment_id == 100)
	{
	  const int16_t* data_words = reinterpret_cast<const int16_t*>(data);
	  size_t n_to_print = std::min<size_t>(26, n_words);
	  
	  std::ostringstream oss;
	  oss << "[makeAndEmplaceFragment_] First " << n_to_print << " words: ";
	  for (size_t i = 0; i < n_to_print; ++i) {
	    oss << data_words[i] << " ";
	  }
	  TLOG(TLVL_INFO) << oss.str();
	}
	 
      out.emplace_back(std::move(frag));
    }

    // -------------------------------------------
    
  };
} // namespace mu2e

#endif // STMTCPRECEIVER_HH
