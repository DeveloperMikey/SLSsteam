#include "IClientCompat.hpp"

#include "../memhlp.hpp"
#include "../vftableinfo.hpp"


void IClientCompat::specifyCompatTool(const AppId_t appId, const char* name, const char* config, int32_t priority)
{
	MemHlp::callVFunc<void(*)(void*, AppId_t, const char*, const char*, int32_t)>
	(
		VFTIndexes::IClientCompat::SpecifyCompatTool.index,
		this,
		appId,
		name,
		config,
		priority
	);
}

IClientCompat* g_pClientCompat = nullptr;
