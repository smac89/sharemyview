#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>

namespace smv {
  enum class EventType : uint32_t
  {
    None = 0x0,
    Move = 0x2,
    Resize = 0x4,
    MouseEnter = 0x8,
    MouseLeave = 0x10,
    MouseMove = 0x20,
    MouseDown = 0x40,
    MouseUp = 0x80,
    MouseWheel = 0x100,
  };

  // generate bitwise operator implementation for EventType
  inline constexpr EventType operator|(EventType a, EventType b)
  {
    return static_cast<EventType>(static_cast<uint32_t>(a) |
                                  static_cast<uint32_t>(b));
  }

  inline constexpr EventType operator&(EventType a, EventType b)
  {
    return static_cast<EventType>(static_cast<uint32_t>(a) &
                                  static_cast<uint32_t>(b));
  }

  inline constexpr EventType operator^(EventType a, EventType b)
  {
    return static_cast<EventType>(static_cast<uint32_t>(a) ^
                                  static_cast<uint32_t>(b));
  }

  inline constexpr EventType operator~(EventType a)
  {
    return static_cast<EventType>(~static_cast<uint32_t>(a));
  }

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

  class EventData
  {
    constexpr static EventType type = EventType::None;

  public:
    [[nodiscard]] constexpr EventType get_type() const { return type; }
    virtual ~EventData() = default;
  };

  class EventDataMouseEnter final : public EventData
  {
    constexpr static EventType type = EventType::MouseEnter;

  public:
    // the mouse position in window coordinate
    uint32_t x, y;
  };

  class EventDataMouseLeave final : public EventData
  {
    constexpr static EventType type = EventType::MouseLeave;

  public:
    // the last mouse position in window coordinate
    uint32_t x, y;
  };

  class EventDataMouseMove final : public EventData
  {
    constexpr static EventType type = EventType::MouseMove;

  public:
    // the mouse position in window coordinate
    uint32_t x, y;

    uint32_t screen_x, screen_y;

    int32_t delta_x, delta_y;
  };

  class EventDataMouseDown final : public EventData
  {
    constexpr static EventType type = EventType::MouseDown;

  public:
    // the mouse position in window coordinate
    uint32_t x, y;

    MouseButton button;
  };

  class EventDataMouseUp final : public EventData
  {
    constexpr static EventType type = EventType::MouseUp;

  public:
    // the mouse position in window coordinate
    uint32_t x, y;

    MouseButton button;
  };

  class EventDataMouseWheel final : public EventData
  {
    constexpr static EventType type = EventType::MouseWheel;

  public:
    int32_t delta_x, delta_y;

    ModifierState modifiers;
  };

  class EventDataResize final : public EventData
  {
    constexpr static EventType type = EventType::Resize;

  public:
    // The new width and height
    uint32_t w, h;
  };

  class EventDataMove final : public EventData
  {
    constexpr static EventType type = EventType::Move;

  public:
    // The new position
    uint32_t x, y;
    // The change in position
    int32_t delta_x, delta_y;
  };

} // namespace smv
