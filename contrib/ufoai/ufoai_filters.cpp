
#include "plugin.h"
#include "ifilters.h"

#define FilterAdd g_FuncTable.m_pfnFilterAdd

enum FILTERS {
	FILTER_ACTORCLIP,
	FILTER_WEAPONCLIP,
	FILTER_NODRAW,
	FILTER_STEPON,
	FILTER_LEVEL1,
	FILTER_LEVEL2,
	FILTER_LEVEL3,
	FILTER_LEVEL4,
	FILTER_LEVEL5,
	FILTER_LEVEL6,
	FILTER_LEVEL7,
	FILTER_LEVEL8,

	FILTER_MAX
};

static bfilter_t* filters[FILTER_MAX];

void UFOAIFilterInit (void)
{
	filters[FILTER_LEVEL1] = FilterAdd(1, 0, "level1", (0x10 << 0));
	filters[FILTER_LEVEL2] = FilterAdd(1, 0, "level2", (0x10 << 1));
	filters[FILTER_LEVEL3] = FilterAdd(1, 0, "level3", (0x10 << 2));
	filters[FILTER_LEVEL4] = FilterAdd(1, 0, "level4", (0x10 << 3));
	filters[FILTER_LEVEL5] = FilterAdd(1, 0, "level5", (0x10 << 4));
	filters[FILTER_LEVEL6] = FilterAdd(1, 0, "level6", (0x10 << 5));
	filters[FILTER_LEVEL7] = FilterAdd(1, 0, "level7", (0x10 << 6));
	filters[FILTER_LEVEL8] = FilterAdd(1, 0, "level8", (0x10 << 7));
}

void PerformFiltering (int type)
{
	switch (type) {
	default:
		Sys_Printf("TODO: Implement filters");
		break;
	}
}

void DoSteponFiltering(void)
{
	PerformFiltering(FILTER_STEPON);
}

void DoWeaponClipFiltering(void)
{
	PerformFiltering(FILTER_WEAPONCLIP);
}

void DoActorClipFiltering(void)
{
	PerformFiltering(FILTER_ACTORCLIP);
}

void DoNoDrawFiltering(void)
{
	PerformFiltering(FILTER_NODRAW);
}

void DoLevel1Filtering(void)
{
	PerformFiltering(FILTER_LEVEL1);
}

void DoLevel2Filtering(void)
{
	PerformFiltering(FILTER_LEVEL2);
}

void DoLevel3Filtering(void)
{
	PerformFiltering(FILTER_LEVEL3);
}

void DoLevel4Filtering(void)
{
	PerformFiltering(FILTER_LEVEL4);
}

void DoLevel5Filtering(void)
{
	PerformFiltering(FILTER_LEVEL5);
}

void DoLevel6Filtering(void)
{
	PerformFiltering(FILTER_LEVEL6);
}

void DoLevel7Filtering(void)
{
	PerformFiltering(FILTER_LEVEL7);
}

void DoLevel8Filtering(void)
{
	PerformFiltering(FILTER_LEVEL8);
}
