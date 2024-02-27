#pragma once

#include <memory_resource>
#include <cassert>

using BumpAllocator = std::pmr::polymorphic_allocator<std::byte>;

namespace utils {

class CustomBufferResource : public std::pmr::memory_resource {
public:
   CustomBufferResource() : CustomBufferResource{128 * sizeof(void*)} {}
   CustomBufferResource(size_t size) {
      buffers_.emplace_back(size, std::malloc(size));
      cur_buf_ = buffers_.begin();
      alloc_top_ = cur_buf_->buf;
      avail_ = size;
   }

   void* do_allocate(std::size_t bytes, std::size_t alignment) override {
      assert((!invalid) && "attempt to allocate when CustomBufferResource is invalid");

      // Do not return the same pointer twice, so set min size to 1
      if(bytes == 0) bytes = 1;

      // Align the next allocation
      void* p = std::align(alignment, bytes, alloc_top_, avail_);

      // Do we have enough space in the current buffer?
      if(!p) {
         // Is there no next buffer? Allocate a new one
         if(std::next(cur_buf_) == buffers_.end()) {
            size_t new_size = cur_buf_->size * growth_factor;
            buffers_.emplace_back(new_size, std::malloc(new_size));
            cur_buf_ = std::prev(buffers_.end());
         } else {
            ++cur_buf_;
         }
         alloc_top_ = cur_buf_->buf;
         avail_ = cur_buf_->size;
         p = cur_buf_->buf;
      }

      // Update the allocation pointer and the available space
      alloc_top_ = static_cast<char*>(alloc_top_) + bytes;
      avail_ -= bytes;

      // Return the aligned pointer
      return p;
   }

   void reset() {
      alloc_top_ = 0;
      cur_buf_ = buffers_.begin();
   }

   void destroy() {
      invalid = true;
   }

   ~CustomBufferResource() {
      for(auto& buf : buffers_) {
         std::free(buf.buf);
      }
   }

   void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
      // No-op
      (void)p;
      (void)bytes;
      (void)alignment;
   }

   bool do_is_equal(
         const std::pmr::memory_resource& other) const noexcept override {
      // No-op
      (void)other;
      return false;
   }

private:
   static constexpr float growth_factor = 1.5;
   struct Buffer {
      size_t size;
      void* buf;
   };
   void* alloc_top_;
   size_t avail_;
   std::vector<Buffer>::iterator cur_buf_;
   std::vector<Buffer> buffers_;
   bool invalid = false;
};

} // namespace utils
