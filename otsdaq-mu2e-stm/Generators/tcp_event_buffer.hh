#ifndef TCP_EVENT_BUFFER_HH
#define TCP_EVENT_BUFFER_HH

#include <cstddef>
#include <cstdint>

class EventRingBuffer
{
public:

  explicit EventRingBuffer(size_t size)
      : buffer_(size), capacity_(size) {}

  uint8_t* writePtr(){
    return buffer_.data() + write_pos_;
  }

  const uint8_t* readPtr() const{
    return buffer_.data() + read_pos_;
  }

  uint8_t* begin(){
    return buffer_.data();
  }

  size_t readable() const{
    size_t w = write_pos_.load(std::memory_order_acquire);
    size_t r = read_pos_.load(std::memory_order_acquire);

    if (w >= r) return w - r;
    return capacity_ - r + w;
  }

  size_t writable() const{
    return capacity_ - readable() - 1;
  }

  size_t readIndex() const{
    return read_pos_.load(std::memory_order_acquire);
  }
  
  size_t writeIndex() const{
    return write_pos_.load(std::memory_order_acquire);
  }
  
  size_t contiguousReadable() const{
    size_t w = write_pos_.load(std::memory_order_acquire);
    size_t r = read_pos_.load(std::memory_order_acquire);

    if (w >= r)
      return w - r;
    else
      return capacity_ - r;
  }

  size_t contiguousWritable() const{
    size_t w = write_pos_.load(std::memory_order_relaxed);
    size_t r = read_pos_.load(std::memory_order_acquire);

    if (w >= r)
      return capacity_ - w;
    else
      return r - w - 1;
  }

  void advanceWrite(size_t n){
    write_pos_.store((write_pos_ + n) % capacity_,
                     std::memory_order_release);
  }

  void advanceRead(size_t n){
    read_pos_.store((read_pos_ + n) % capacity_,
                    std::memory_order_release);
  }

  void releaseUntil(size_t pos){
    read_pos_.store(pos, std::memory_order_release);
  }
  
private:

  std::vector<uint8_t> buffer_;
  const size_t capacity_;

  std::atomic<size_t> write_pos_{0};
  std::atomic<size_t> read_pos_{0};
};

#endif //TCP_EVENT_BUFFER_HH
