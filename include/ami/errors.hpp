#pragma once
#include <fmt/core.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
namespace ami {
namespace exceptions {
struct ExceptionInterface {
  std::string name, err, file, src;
  std::size_t linepos;
};
class BaseException {
  std::string name, err, file, src;
  std::size_t linepos;
  std::string m_Fmt;

 public:
  explicit BaseException(const ExceptionInterface& ei)
      : name(ei.name),
        err(ei.err),
        file(ei.file),
        src(ei.src),
        linepos(ei.linepos) {
    std::string _fmt_str = "at \"<{file}>\" col '{pos}', {name}:  {error}";
    _fmt_str = fmt::format(
        "{}\n{}\n",
        fmt::format(_fmt_str, fmt::arg("file", file), fmt::arg("pos", linepos),
                    fmt::arg("name", name), fmt::arg("error", err)),
        src);
    _fmt_str += std::string(linepos, ' ') + '^';
    m_Fmt = _fmt_str;
  }
  std::string what() const throw() { return m_Fmt; }
};
}  // namespace exceptions
}  // namespace ami
