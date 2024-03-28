#include "smv/windowobject.hpp"
#include "smv/events.hpp"
#include <list>
#include <unordered_map>

namespace smv {
class WindowLinux final : public WindowObject {
  friend class WindowsClient;

  void on_event(const EventType evt, const smv::EventData &data) {
    auto it = m_callbacks.find(evt);
    if (it != m_callbacks.end()) {
      for (auto &cb : it->second) {
        cb(data);
      }
    }
  }

public:
  explicit WindowLinux(uint32_t id, uint32_t x, uint32_t y, uint32_t w,
                       uint32_t h)
      : m_id(id), m_pos_x(x), m_pos_y(y), m_width(w), m_height(h) {}

  uint32_t id() override { return m_id; }
  std::string name() override { return ""; }
  std::tuple<uint32_t, uint32_t> get_position() override {
    return std::make_tuple(m_pos_x, m_pos_y);
  }
  std::tuple<uint32_t, uint32_t> get_size() override {
    return std::make_tuple(m_width, m_height);
  }

  void move(uint32_t x, uint32_t y) {
    m_pos_x = x;
    m_pos_y = y;
  }

  void resize(uint32_t w, uint32_t h) {
    m_width = w;
    m_height = h;
  }

private:
  uint32_t m_id;
  uint32_t m_pos_x, m_pos_y;
  uint32_t m_width, m_height;
};

} // namespace smv
