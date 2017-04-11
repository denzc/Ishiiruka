// Copyright 2008 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <memory>
#include "Common/CommonTypes.h"

class PointerWrap;

namespace MMIO
{
class Mapping;
}

namespace ExpansionInterface
{
class IEXIDevice;
enum TEXIDevices : int;

class CEXIChannel
{
public:
	explicit CEXIChannel(u32 channel_id);
	~CEXIChannel();

	// get device
	IEXIDevice* GetDevice(u8 chip_select);
	IEXIDevice* FindDevice(TEXIDevices device_type, int custom_index = -1);

	void RegisterMMIO(MMIO::Mapping* mmio, u32 base);

	void SendTransferComplete();

	void AddDevice(TEXIDevices device_type, int device_num);
	void AddDevice(std::unique_ptr<IEXIDevice> device, int device_num,
		bool notify_presence_changed = true);

	// Remove all devices
	void RemoveDevices();

	bool IsCausingInterrupt();
	void DoState(PointerWrap& p);
	void PauseAndLock(bool do_lock, bool resume_on_unlock);

	// This should only be used to transition interrupts from SP1 to Channel 2
	void SetEXIINT(bool exiint);

private:
	enum
	{
		EXI_STATUS = 0x00,
		EXI_DMA_ADDRESS = 0x04,
		EXI_DMA_LENGTH = 0x08,
		EXI_DMA_CONTROL = 0x0C,
		EXI_IMM_DATA = 0x10
	};

	// EXI Status Register - "Channel Parameter Register"
	union UEXI_STATUS
	{
		u32 Hex = 0;
		// DO NOT obey the warning and give this struct a name. Things will fail.
		struct
		{
			// Indentation Meaning:
			// Channels 0, 1, 2
			//  Channels 0, 1 only
			//      Channel 0 only
			u32 EXIINTMASK : 1;
			u32 EXIINT : 1;
			u32 TCINTMASK : 1;
			u32 TCINT : 1;
			u32 CLK : 3;
			u32 CHIP_SELECT : 3;  // CS1 and CS2 are Channel 0 only
			u32 EXTINTMASK : 1;
			u32 EXTINT : 1;
			u32 EXT : 1;     // External Insertion Status (1: External EXI device present)
			u32 ROMDIS : 1;  // ROM Disable
			u32 : 18;
		};
		UEXI_STATUS() = default;
		explicit UEXI_STATUS(u32 hex) : Hex{ hex } {}
	};

	// EXI Control Register
	union UEXI_CONTROL
	{
		u32 Hex = 0;
		struct
		{
			u32 TSTART : 1;
			u32 DMA : 1;
			u32 RW : 2;
			u32 TLEN : 2;
			u32 : 26;
		};
	};

	// STATE_TO_SAVE
	UEXI_STATUS m_status;
	u32 m_dma_memory_address = 0;
	u32 m_dma_length = 0;
	UEXI_CONTROL m_control;
	u32 m_imm_data = 0;

	// Devices
	enum
	{
		NUM_DEVICES = 3
	};

	std::array<std::unique_ptr<IEXIDevice>, NUM_DEVICES> m_devices;

	// Since channels operate a bit differently from each other
	u32 m_channel_id;
};
}  // namespace ExpansionInterface
