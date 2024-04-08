#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

namespace smv
{
  struct Window;

  enum class EventType : uint16_t
  {
    None        = 0x0,
    Move        = 0x2,
    Resize      = 0x4,
    MouseEnter  = 0x8,
    MouseLeave  = 0x10,
    MouseMove   = 0x20,
    MouseDown   = 0x40,
    MouseUp     = 0x80,
    MouseWheel  = 0x100,
    WindowClose = 0x200,
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
    constexpr static EventType type = EventType::None;

    const std::weak_ptr<Window> window;
  };

  struct EventDataMouseEnter final: public EventData
  {
    constexpr static EventType type = EventType::MouseEnter;

    // the mouse position in window coordinate
    uint32_t x, y;
  };

  struct EventDataMouseLeave final: public EventData
  {
    constexpr static EventType type = EventType::MouseLeave;

    // the last mouse position in window coordinate
    uint32_t x, y;
  };

  struct EventDataMouseMove final: public EventData
  {
    constexpr static EventType type = EventType::MouseMove;

    // the mouse position in window coordinate
    uint32_t x, y;

    uint32_t screen_x, screen_y;

    int32_t delta_x, delta_y;
  };

  struct EventDataMouseDown final: public EventData
  {
    constexpr static EventType type = EventType::MouseDown;

    // the mouse position in window coordinate
    uint32_t x, y;

    MouseButton button;
  };

  struct EventDataMouseUp final: public EventData
  {
    constexpr static EventType type = EventType::MouseUp;

    // the mouse position in window coordinate
    uint32_t x, y;

    MouseButton button;
  };

  struct EventDataMouseWheel final: public EventData
  {
    constexpr static EventType type = EventType::MouseWheel;

    int32_t delta_x, delta_y;

    ModifierState modifiers;
  };

  struct EventDataResize final: public EventData
  {
    constexpr static EventType type = EventType::Resize;

    // The new width and height
    uint32_t w, h;
  };

  struct EventDataMove final: public EventData
  {
    constexpr static EventType type = EventType::Move;

    // The new position
    uint32_t x, y;
    // The change in position
    int16_t delta_x, delta_y;
  };

  struct EventDataWindowClose final: public EventData
  {
    constexpr static EventType type = EventType::WindowClose;
  };

} // namespace smv
