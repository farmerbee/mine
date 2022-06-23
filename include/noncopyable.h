#ifndef __NONCOPYABLE_H
#define __NONCOPYABLE_H

namespace mine {
class NonCopyable {
public:
  NonCopyable() = default;
  ~NonCopyable() = default;
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
};
} // namespace mine

#endif