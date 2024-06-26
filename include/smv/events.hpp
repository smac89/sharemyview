#pragma once
#include "window.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <spdlog/fmt/fmt.h>

namespace smv {
  struct EventData;

  using Cancel = std::function<void()>;
  template<typename D>
  using TEventCB = std::function<void(const D &)>;
  using EventCB  = TEventCB<EventData>;

  enum class EventType : uint16_t
  {
    None          = 0x0,
    WindowVisible = 0x1,
    WindowMove    = 0x2,
    WindowResize  = 0x4,
    MouseEnter    = 0x8,
    MouseLeave    = 0x10,
    MouseMove     = 0x20,
    MouseDown     = 0x40,
    MouseUp       = 0x80,
    MouseWheel    = 0x100,
    WindowCreated = 0x200,
    WindowClose   = 0x400,
    WindowRenamed = 0x800,
  };

  enum class MouseButton
  {
    Left,
    Right,
    Middle,
    Button4,
    Button5
  };

  /**
   * The state of modifier keys such as ctrl, alt, shift, and the
   * windows/command buttons. Pressed is true, released is false;
   */
  struct ModifierState
  {
    // Ctrl key
    bool ctrl = false;
    // Alt key
    bool alt = false;
    // Shift key
    bool shift = false;
    // Meta buttons such as the Windows button or Mac's Command button
    bool meta = false;
  };

  struct EventData
  {
    EventData(const std::weak_ptr<Window> &window)
      : window(window)
    {
    }
    virtual auto format() const -> std::string { return "<No Data>"; }
    virtual ~EventData() = default;

    std::weak_ptr<Window>      window;
    constexpr static EventType type = EventType::None;
  };

  struct EventDataMouseEnter final: EventData
  {

    EventDataMouseEnter(const std::weak_ptr<Window> &window,
                        uint32_t                     xPos,
                        uint32_t                     yPos)
      : EventData(window)
      , x(xPos)
      , y(yPos)
    {
    }

    auto format() const -> std::string override
    {
      return fmt::format("{}, {}", x, y);
    }

    // the mouse position in window coordinate
    uint32_t                   x, y;
    constexpr static EventType type = EventType::MouseEnter;
  };

  struct EventDataMouseLeave final: EventData
  {
    EventDataMouseLeave(const std::weak_ptr<Window> &window,
                        uint32_t                     xPos,
                        uint32_t                     yPos)
      : EventData(window)
      , x(xPos)
      , y(yPos)
    {
    }
    auto format() const -> std::string override
    {
      return fmt::format("{}, {}", x, y);
    }

    // the last mouse position in window coordinate
    uint32_t                   x, y;
    constexpr static EventType type = EventType::MouseLeave;
  };

  struct EventDataMouseMove final: EventData
  {
    EventDataMouseMove(const std::weak_ptr<Window> &window,
                       uint32_t                     xPos,
                       uint32_t                     yPos,
                       int32_t                      dx,
                       int32_t                      dy)
      : EventData(window)
      , x(xPos)
      , y(yPos)
      , delta_x(dx)
      , delta_y(dy)
    {
    }

    // the mouse position in window coordinate
    uint32_t                   x, y;
    int32_t                    delta_x, delta_y;
    uint32_t                   screen_x {}, screen_y {};
    constexpr static EventType type = EventType::MouseMove;
  };

  struct EventDataMouseDown final: EventData
  {
    EventDataMouseDown(const std::weak_ptr<Window> &window,
                       uint32_t                     xPos,
                       uint32_t                     yPos,
                       MouseButton                  btn)
      : EventData(window)
      , x(xPos)
      , y(yPos)
      , button(btn)
    {
    }

    // the mouse position in window coordinate
    uint32_t                   x, y;
    MouseButton                button;
    constexpr static EventType type = EventType::MouseDown;
  };

  struct EventDataMouseUp final: EventData
  {
    EventDataMouseUp(const std::weak_ptr<Window> &window,
                     uint32_t                     xPos,
                     uint32_t                     yPos,
                     MouseButton                  btn)
      : EventData(window)
      , x(xPos)
      , y(yPos)
      , button(btn)
    {
    }

    // the mouse position in window coordinate
    uint32_t                   x, y;
    MouseButton                button;
    constexpr static EventType type = EventType::MouseUp;
  };

  struct EventDataMouseWheel final: EventData
  {
    EventDataMouseWheel(const std::weak_ptr<Window> &window,
                        int32_t                      dx,
                        int32_t                      dy)
      : EventData(window)
      , delta_x(dx)
      , delta_y(dy)
    {
    }

    const int32_t              delta_x, delta_y;
    ModifierState              modifiers;
    constexpr static EventType type = EventType::MouseWheel;
  };

  struct EventDataWindowResize final: EventData
  {
    EventDataWindowResize(const std::weak_ptr<Window> &window,
                          uint32_t                     width,
                          uint32_t                     height)
      : EventData(window)
      , w(width)
      , h(height)
    {
    }

    // The new width and height
    const uint32_t             w, h;
    constexpr static EventType type = EventType::WindowResize;
  };

  struct EventDataWindowMove final: EventData
  {
    EventDataWindowMove(const std::weak_ptr<Window> &window,
                        int32_t                      xPos,
                        int32_t                      yPos,
                        int32_t                      dx,
                        int32_t                      dy)
      : EventData(window)
      , x(xPos)
      , y(yPos)
      , delta_x(dx)
      , delta_y(dy)
    {
    }

    // The new position
    int32_t x, y;
    // The change in position
    int32_t                    delta_x, delta_y;
    constexpr static EventType type = EventType::WindowMove;
  };

  struct EventDataWindowCreated final: EventData
  {
    EventDataWindowCreated(const std::weak_ptr<Window> &window)
      : EventData(window)
    {
    }
    constexpr static EventType type = EventType::WindowCreated;
  };

  struct EventDataWindowClose final: EventData
  {
    EventDataWindowClose(const std::weak_ptr<Window> &window)
      : EventData(window)
    {
    }
    constexpr static EventType type = EventType::WindowClose;
  };

  struct EventDataWindowRenamed final: public EventData
  {
    EventDataWindowRenamed(const std::weak_ptr<Window> &window,
                           std::string                  name)
      : EventData(window)
      , name(std::move(name))
    {
    }
    // check the name of the window
    std::string                name;
    constexpr static EventType type = EventType::WindowRenamed;
  };

  struct EventDataWindowVisible final: EventData
  {
    EventDataWindowVisible(const std::weak_ptr<Window> &window, bool visible)
      : EventData(window)
      , visible(visible)
    {
    }
    bool                       visible;
    constexpr static EventType type = EventType::WindowVisible;
  };
} // namespace smv
