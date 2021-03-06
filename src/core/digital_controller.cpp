#include "digital_controller.h"
#include "common/assert.h"
#include "common/state_wrapper.h"
#include "host_interface.h"

DigitalController::DigitalController() = default;

DigitalController::~DigitalController() = default;

ControllerType DigitalController::GetType() const
{
  return ControllerType::DigitalController;
}

std::optional<s32> DigitalController::GetAxisCodeByName(std::string_view axis_name) const
{
  return StaticGetAxisCodeByName(axis_name);
}

std::optional<s32> DigitalController::GetButtonCodeByName(std::string_view button_name) const
{
  return StaticGetButtonCodeByName(button_name);
}

void DigitalController::Reset()
{
  m_transfer_state = TransferState::Idle;
}

bool DigitalController::DoState(StateWrapper& sw)
{
  if (!Controller::DoState(sw))
    return false;

  sw.Do(&m_button_state);
  sw.Do(&m_transfer_state);
  return true;
}

void DigitalController::SetAxisState(s32 axis_code, float value) {}

void DigitalController::SetButtonState(Button button, bool pressed)
{
  if (pressed)
    m_button_state &= ~(u16(1) << static_cast<u8>(button));
  else
    m_button_state |= u16(1) << static_cast<u8>(button);
}

void DigitalController::SetButtonState(s32 button_code, bool pressed)
{
  if (button_code < 0 || button_code >= static_cast<s32>(Button::Count))
    return;

  SetButtonState(static_cast<Button>(button_code), pressed);
}

u32 DigitalController::GetButtonStateBits() const
{
  return m_button_state ^ 0xFFFF;
}

void DigitalController::ResetTransferState()
{
  m_transfer_state = TransferState::Idle;
}

bool DigitalController::Transfer(const u8 data_in, u8* data_out)
{
  static constexpr u16 ID = 0x5A41;

  switch (m_transfer_state)
  {
    case TransferState::Idle:
    {
      // ack when sent 0x01, send ID for 0x42
      if (data_in == 0x42)
      {
        *data_out = Truncate8(ID);
        m_transfer_state = TransferState::IDMSB;
        return true;
      }
      else
      {
        *data_out = 0xFF;
        return (data_in == 0x01);
      }
    }

    case TransferState::IDMSB:
    {
      *data_out = Truncate8(ID >> 8);
      m_transfer_state = TransferState::ButtonsLSB;
      return true;
    }

    case TransferState::ButtonsLSB:
    {
      *data_out = Truncate8(m_button_state);
      m_transfer_state = TransferState::ButtonsMSB;
      return true;
    }

    case TransferState::ButtonsMSB:
      *data_out = Truncate8(m_button_state >> 8);
      m_transfer_state = TransferState::Idle;
      return false;

    default:
    {
      UnreachableCode();
      return false;
    }
  }
}

std::unique_ptr<DigitalController> DigitalController::Create()
{
  return std::make_unique<DigitalController>();
}

std::optional<s32> DigitalController::StaticGetAxisCodeByName(std::string_view button_name)
{
  return std::nullopt;
}

std::optional<s32> DigitalController::StaticGetButtonCodeByName(std::string_view button_name)
{
#define BUTTON(name)                                                                                                   \
  if (button_name == #name)                                                                                            \
  {                                                                                                                    \
    return static_cast<s32>(ZeroExtend32(static_cast<u8>(Button::name)));                                              \
  }

  BUTTON(Select);
  BUTTON(L3);
  BUTTON(R3);
  BUTTON(Start);
  BUTTON(Up);
  BUTTON(Right);
  BUTTON(Down);
  BUTTON(Left);
  BUTTON(L2);
  BUTTON(R2);
  BUTTON(L1);
  BUTTON(R1);
  BUTTON(Triangle);
  BUTTON(Circle);
  BUTTON(Cross);
  BUTTON(Square);

  return std::nullopt;

#undef BUTTON
}

Controller::AxisList DigitalController::StaticGetAxisNames()
{
  return {};
}

Controller::ButtonList DigitalController::StaticGetButtonNames()
{
  return {{TRANSLATABLE("DigitalController", "Up"), static_cast<s32>(Button::Up)},
          {TRANSLATABLE("DigitalController", "Down"), static_cast<s32>(Button::Down)},
          {TRANSLATABLE("DigitalController", "Left"), static_cast<s32>(Button::Left)},
          {TRANSLATABLE("DigitalController", "Right"), static_cast<s32>(Button::Right)},
          {TRANSLATABLE("DigitalController", "Select"), static_cast<s32>(Button::Select)},
          {TRANSLATABLE("DigitalController", "Start"), static_cast<s32>(Button::Start)},
          {TRANSLATABLE("DigitalController", "Triangle"), static_cast<s32>(Button::Triangle)},
          {TRANSLATABLE("DigitalController", "Cross"), static_cast<s32>(Button::Cross)},
          {TRANSLATABLE("DigitalController", "Circle"), static_cast<s32>(Button::Circle)},
          {TRANSLATABLE("DigitalController", "Square"), static_cast<s32>(Button::Square)},
          {TRANSLATABLE("DigitalController", "L1"), static_cast<s32>(Button::L1)},
          {TRANSLATABLE("DigitalController", "L2"), static_cast<s32>(Button::L2)},
          {TRANSLATABLE("DigitalController", "R1"), static_cast<s32>(Button::R1)},
          {TRANSLATABLE("DigitalController", "R2"), static_cast<s32>(Button::R2)}};
}

u32 DigitalController::StaticGetVibrationMotorCount()
{
  return 0;
}
