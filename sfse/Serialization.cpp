#include "Serialization.h"
#include "GameEvents.h"

#include "sfse_common/Log.h"
#include "sfse_common/Errors.h"
#include "sfse_common/sfse_version.h"
#include "sfse/GameSettings.h"

#include <ShlObj.h>
#include <unordered_map>

namespace Serialization
{
	const char* kSavegamePath = "\\My Games\\" SAVE_FOLDER_NAME "\\";

	std::unordered_map<u32, u32> changedIDs;
	std::string s_savePath;

	struct IDRemapListener : public BSTEventSink<TESFormIDRemapEvent>
	{
		IDRemapListener()
		{
			GetEventSource<TESFormIDRemapEvent>()->RegisterSink(this);
		}

		virtual	EventResult	ProcessEvent(const TESFormIDRemapEvent& arEvent, BSTEventSource<TESFormIDRemapEvent>* eventSource)
		{
			changedIDs[arEvent.oldID] = arEvent.newID;
			return EventResult::kContinue;
		};
	};

	std::string MakeSavePath(std::string name, const char* extension, bool hasExtension)
	{
		if (hasExtension)
		{
			size_t lastDot = name.find_last_of('.');
			if (lastDot != std::string::npos) {
				name.erase(lastDot);
			}
		}

		char path[MAX_PATH];
		ASSERT(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, path)));

		std::string	result = path;
		result += kSavegamePath;
		Setting* localSavePath = (*SettingT<INISettingCollection>::pCollection)->GetSetting("sLocalSavePath:General");
		if (localSavePath && (localSavePath->GetType() == Setting::kType_String))
			result += localSavePath->data.s;
		else
			result += "Saves\\";

		result += "\\";
		result += name;
		if (extension)
			result += extension;
		return result;
	}

	void SetSaveName(const char* name, bool hasExtension)
	{
		if (name)
		{
			_MESSAGE("save name is %s", name);
			s_savePath = MakeSavePath(name, ".sfse", hasExtension);
			_MESSAGE("full save path: %s", s_savePath.c_str());
		}
		else
		{
			_MESSAGE("cleared save path");
			s_savePath.clear();
		}
	}

	void HandleBeginLoad()
	{
		//if the remap listener isn't already registered, register it now.
		static IDRemapListener listener{};
	}

	void HandleEndLoad()
	{
		changedIDs.clear();
	}

	bool ResolveFormId(u32 formId, u32* formIdOut)
	{
		if (auto iter = changedIDs.find(formId); iter != changedIDs.end()) {
			(*formIdOut) = iter->second;
			return true;
		}
		else
		{
			(*formIdOut) = formId;
			return false;
		}
	}

	bool ResolveHandle(u64 handle, u64* handleOut)
	{
		u32 formId = static_cast<u32>(handle & 0x00000000FFFFFFFF);
		if (auto iter = changedIDs.find(formId); iter != changedIDs.end()) {
			(*handleOut) = (handle & 0xFFFFFFFF00000000) | static_cast<u64>(iter->second);
			return true;
		}
		else
		{
			(*handleOut) = handle;
			return false;
		}
	}

	void HandleRevertGlobalData()
	{
		_MESSAGE("RevertGlobalData");

		//TODO: add implementation for revert callbacks.
	}

	void HandleSaveGlobalData()
	{
		_MESSAGE("SaveGlobalData");

		//TODO: add implementation for serialization & save callbacks.
	}

	void HandleLoadGlobalData()
	{
		_MESSAGE("LoadGlobalData");

		//TODO: add implementation for deserialization & load callbacks.
	}

	void HandleDeleteSave(std::string saveName)
	{
	}



}