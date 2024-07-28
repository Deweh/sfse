#include "Hooks_Serialization.h"

#include "sfse_common/BranchTrampoline.h"
#include "sfse_common/Relocation.h"
#include "sfse_common/SafeWrite.h"

#include "sfse/PluginManager.h"

#include "xbyak/xbyak.h"

class BGSSaveLoadGame;

typedef void (*_SaveGame)(BGSSaveLoadGame* a_this, void* a_unk1, void* a_unk2, const char* a_name);
RelocAddr <_SaveGame> SaveGame_Call(0x024ACCB0 + 0x12B);
RelocAddr <_SaveGame> SaveGame_Original(0x024AFCC0);

typedef bool (*_LoadGame)(BGSSaveLoadGame* a_this, const char* a_name, void* a_unk1, void* a_unk2);
RelocAddr <_LoadGame> LoadGame_Call(0x024DFF80 + 0x572);
RelocAddr <_LoadGame> LoadGame_Original(0x024B55B0);

typedef bool (*_VM_SaveGame)(void* a_this, void* a_storage, void* a_handleReaderWriter, bool a_flag);
typedef bool (*_VM_LoadGame)(void* a_this, void* a_storage, void* a_handleReaderWriter, bool* a_flag, bool* b_flag);
typedef void* (*_VM_DropAllRunningData)(void* a_this);
_VM_SaveGame VM_SaveGame_Original = nullptr;
_VM_LoadGame VM_LoadGame_Original = nullptr;
_VM_DropAllRunningData VM_DropAllRunningData_Original = nullptr;
RelocAddr <uintptr_t> VirtualMachine_IVMSaveLoadInterface_VTable(0x0545A240);

void SaveGame_Hook(BGSSaveLoadGame* a_this, void* a_unk1, void* a_unk2, const char* a_name)
{
	SaveGame_Original(a_this, a_unk1, a_unk2, a_name);
}

bool LoadGame_Hook(BGSSaveLoadGame* a_this, const char* a_name, void* a_unk1, void* a_unk2)
{
	return LoadGame_Original(a_this, a_name, a_unk1, a_unk2);
}

void* VM_DropAllRunningData_Hook(void* a_this)
{
	return VM_DropAllRunningData_Original(a_this);
}

bool VM_SaveGame_Hook(void* a_this, void* a_storage, void* a_handleReaderWriter, bool a_flag)
{
	return VM_SaveGame_Original(a_this, a_storage, a_handleReaderWriter, a_flag);
}

bool VM_LoadGame_Hook(void* a_this, void* a_storage, void* a_handleReaderWriter, bool* a_flag, bool* b_flag)
{
	return VM_LoadGame_Original(a_this, a_storage, a_handleReaderWriter, a_flag, b_flag);
}

void Hooks_Serialization_Apply()
{
	g_branchTrampoline.write5Call(SaveGame_Call.getUIntPtr(), (uintptr_t)SaveGame_Hook);
	g_branchTrampoline.write5Call(LoadGame_Call.getUIntPtr(), (uintptr_t)LoadGame_Hook);

	uintptr_t VM_SaveGame_VFunc = VirtualMachine_IVMSaveLoadInterface_VTable.getUIntPtr() + (0x1 * 0x8);
	uintptr_t VM_LoadGame_VFunc = VirtualMachine_IVMSaveLoadInterface_VTable.getUIntPtr() + (0x2 * 0x8);
	uintptr_t VM_DropAllRunningData_VFunc = VirtualMachine_IVMSaveLoadInterface_VTable.getUIntPtr() + (0x7 * 0x8);

	VM_SaveGame_Original = *reinterpret_cast<_VM_SaveGame*>(VM_SaveGame_VFunc);
	VM_LoadGame_Original = *reinterpret_cast<_VM_LoadGame*>(VM_LoadGame_VFunc);
	VM_DropAllRunningData_Original = *reinterpret_cast<_VM_DropAllRunningData*>(VM_DropAllRunningData_VFunc);

	safeWrite64(VM_SaveGame_VFunc, (uintptr_t)VM_SaveGame_Hook);
	safeWrite64(VM_LoadGame_VFunc, (uintptr_t)VM_LoadGame_Hook);
	safeWrite64(VM_DropAllRunningData_VFunc, (uintptr_t)VM_DropAllRunningData_Hook);
}