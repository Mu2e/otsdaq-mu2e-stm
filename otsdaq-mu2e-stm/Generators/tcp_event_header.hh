#ifndef TCP_EVENT_HEADER_HH
#define TCP_EVENT_HEADER_HH

#include <cstddef>
#include <cstdint>

// ---- Layout ----
constexpr size_t LENGTH_PREFIX_WORDS = 3;
constexpr size_t LENGTH_PREFIX_BYTES = 3 * sizeof(uint16_t);

constexpr size_t EVENT_HEADER_WORDS = 21;
constexpr size_t EVENT_HEADER_BYTES = EVENT_HEADER_WORDS * sizeof(uint16_t);

// ---- Anchor ----
constexpr uint16_t ANCHOR_WORD = 0xCAFE;

// ---- Index positions ----
constexpr uint16_t anchor_start = 0;

constexpr uint16_t EWT_0 = 1;
constexpr uint16_t EWT_1 = 2;
constexpr uint16_t EWT_2 = 3;

constexpr uint16_t ADCclk_0 = 4;
constexpr uint16_t ADCclk_1 = 5;
constexpr uint16_t ADCclk_2 = 6;
constexpr uint16_t ADCclk_3 = 7;

constexpr uint16_t Ch_DTCclk_0 = 8;
constexpr uint16_t DTCclk_1 = 9;
constexpr uint16_t DTCclk_2 = 10;
constexpr uint16_t DTCclk_3 = 11;

constexpr uint16_t EM_0 = 12;
constexpr uint16_t EM_1 = 13;
constexpr uint16_t EM_2_DRTDC = 14;

constexpr uint16_t PRESCALE = 15;

constexpr uint16_t RAW_LEN = 16;
constexpr uint16_t ZS_REGIONS = 17;
constexpr uint16_t ZS_LEN = 18;
constexpr uint16_t PH_NUM = 19;

constexpr uint16_t anchor_end = 20;

static_assert(anchor_end + 1 == EVENT_HEADER_WORDS,
              "Event header size mismatch");

#endif //TCP_EVENT_HEADER_HH
