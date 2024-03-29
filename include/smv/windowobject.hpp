#pragma once
#include "events.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <tuple>

namespace smv {
  using EventCallback = std::function<void(const smv::EventData &)>;

  struct WindowObject
  {
    virtual uint32_t id() = 0;
    virtual std::string name() = 0;
    virtual ~WindowObject() = default;
    virtual std::tuple<uint32_t, uint32_t> get_position() = 0;
    virtual std::tuple<uint32_t, uint32_t> get_size() = 0;
    virtual void listen(const EventType evt, const EventCallback &cb)
    {
      m_callbacks[evt].push_back(cb);
    }

  protected:
    std::unordered_map<EventType, std::vector<EventCallback>> m_callbacks{};
  };

} // namespace smv
