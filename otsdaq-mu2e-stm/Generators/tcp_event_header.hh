#ifndef TCP_EVENT_HEADER_HH
#define TCP_EVENT_HEADER_HH

#include "artdaq-core-mu2e/Overlays/STMHeaderLayout.hh"

// ---- TCP framing (not part of the event header itself) ----
constexpr size_t LENGTH_PREFIX_WORDS = 3;
constexpr size_t LENGTH_PREFIX_BYTES = 3 * sizeof(uint16_t);

// ---- Convenience aliases derived from stm::RawHeader ----
constexpr size_t EVENT_HEADER_WORDS = stm::RawHeader::WORDS;
constexpr size_t EVENT_HEADER_BYTES = EVENT_HEADER_WORDS * sizeof(uint16_t);

#endif  //TCP_EVENT_HEADER_HH
