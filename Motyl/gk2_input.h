#ifndef __GK2_INPUT_H_
#define __GK2_INPUT_H_

#include <cassert>
#include <memory>
#include "gk2_utils.h"
#include "gk2_exceptions.h"

namespace gk2
{

	class ApplicationBase;

	struct KeyboardState
	{
		static const unsigned int STATE_TAB_LENGTH = 256;
		static const BYTE KEY_MASK = 0x80;

		BYTE m_keys[STATE_TAB_LENGTH];

		KeyboardState()
		{
			ZeroMemory(m_keys, STATE_TAB_LENGTH*sizeof(char));
		}

		KeyboardState(const KeyboardState& other)
		{
			memcpy(m_keys, other.m_keys, STATE_TAB_LENGTH*sizeof(char));
		}

		KeyboardState& operator=(const KeyboardState& other)
		{
			memcpy(m_keys, other.m_keys, STATE_TAB_LENGTH*sizeof(char));
			return *this;
		}

		bool isKeyDown(BYTE keyCode) const
		{
			return 0 != (m_keys[keyCode] & KEY_MASK);
		}

		bool isKeyUp(BYTE keyCode) const
		{
			return 0 == (m_keys[keyCode] & KEY_MASK);
		}

		bool operator[](BYTE keyCode) const
		{
			return 0 != (m_keys[keyCode] & KEY_MASK);
		}
	};

	struct MouseState
	{
		enum Buttons
		{
			Left = 0,
			Right = 1,
			Middle = 2
		};

		static const BYTE BUTTON_MASK = 0x80;

		DIMOUSESTATE m_state;

		MouseState()
		{
			ZeroMemory(&m_state, sizeof(DIMOUSESTATE));
		}

		MouseState(const MouseState& other)
		{
			memcpy(&m_state, &other.m_state, sizeof(DIMOUSESTATE));
		}

		MouseState& operator=(const MouseState& other)
		{
			memcpy(&m_state, &other.m_state, sizeof(DIMOUSESTATE));
			return *this;
		}

		POINT getMousePositionChange() const
		{
			POINT r;
			r.x = m_state.lX;
			r.y = m_state.lY;
			return r;
		}

		LONG getWheelPositionChange() const
		{
			return m_state.lZ;
		}

		bool isButtonDown(BYTE button) const
		{
			assert(button < 4);
			return 0 != (m_state.rgbButtons[button] & BUTTON_MASK);
		}

		bool isButtonUp(BYTE button) const
		{
			assert(button < 4);
			return 0 == (m_state.rgbButtons[button] & BUTTON_MASK);
		}

		bool operator[](BYTE button) const
		{
			assert(button < 4);
			return 0 != (m_state.rgbButtons[button] & BUTTON_MASK);
		}
	};

	template<typename TState>
	class DeviceBase
	{
	public:
		static const unsigned int GET_STATE_RETRIES = 2;
		static const unsigned int AQUIRE_RETRIES = 2;

	protected:
		explicit DeviceBase(unique_ptr_del<IDirectInputDevice8W>&& device)
			: m_device(move(device))
		{ }

		bool GetState(unsigned int size, void* ptr) const
		{
			if (!m_device)
				return false;
			for (auto i = 0; i < GET_STATE_RETRIES; ++i)
			{
				auto result = m_device->GetDeviceState(size, ptr);
				if (SUCCEEDED(result))
					return true;
				if (result != DIERR_INPUTLOST && result != DIERR_NOTACQUIRED)
					THROW_DX11(result);
				for (auto j = 0; j < AQUIRE_RETRIES; ++j)
				{
					result = m_device->Acquire();
					if (SUCCEEDED(result))
						break;
					if (result != DIERR_INPUTLOST && result != E_ACCESSDENIED)
						THROW_DX11(result);
				}
			}
			return false;
		}

		unique_ptr_del<IDirectInputDevice8W> m_device;
	};

	class Keyboard : public DeviceBase<KeyboardState>
	{
	public:
		bool GetState(KeyboardState& state) const;

		friend class ApplicationBase;
		
	private:
		explicit Keyboard(unique_ptr_del<IDirectInputDevice8W>&& device)
			: DeviceBase(move(device))
		{ }
	};

	class Mouse : public DeviceBase<MouseState>
	{
	public:
		bool GetState(MouseState& state) const;

		friend class ApplicationBase;

	private:
		explicit Mouse(unique_ptr_del<IDirectInputDevice8W> device)
			: DeviceBase(move(device))
		{ }
	};

	class InputHelper
	{
	public:
		unique_ptr_del<IDirectInput8W> m_inputObject;

		unique_ptr_del<IDirectInputDevice8W> CreateInputDevice(HWND hWnd, const GUID& deviceGuid,
																const DIDATAFORMAT& dataFormat) const;
	};
}

#endif __GK2_INPUT_H_
