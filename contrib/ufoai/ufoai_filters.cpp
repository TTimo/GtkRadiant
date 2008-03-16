
#include "plugin.h"
#include "ufoai_filters.h"
#include "ifilters.h"

#define FilterAdd g_FuncTable.m_pfnFilterAdd
#define FiltersActivate g_FuncTable.m_pfnFiltersActivate

static bfilter_t* filters[FILTER_MAX];

void UFOAIFilterInit (void)
{
	filters[FILTER_ACTORCLIP] = FilterAdd(1, 0, "actorclip", 0);
	filters[FILTER_WEAPONCLIP] = FilterAdd(1, 0, "weaponclip", 0);
	filters[FILTER_NODRAW] = FilterAdd(1, 0, "nodraw", 0);
	filters[FILTER_STEPON] = FilterAdd(1, 0, "stepon", 0);

	filters[FILTER_LEVEL1] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_1, "level1", 0);
	filters[FILTER_LEVEL2] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_2, "level2", 0);
	filters[FILTER_LEVEL3] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_3, "level3", 0);
	filters[FILTER_LEVEL4] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_4, "level4", 0);
	filters[FILTER_LEVEL5] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_5, "level5", 0);
	filters[FILTER_LEVEL6] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_6, "level6", 0);
	filters[FILTER_LEVEL7] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_7, "level7", 0);
	filters[FILTER_LEVEL8] = FilterAdd(2, UFOAI_CONTENTS_LEVEL_8, "level8", 0);

	Sys_Printf("UFO:AI Filters initialized\n");
}

void PerformFiltering (int type)
{
	if (!filters[type]) {
		Sys_Printf("filters are not yet initialized\n");
		return;
	}

	if (filters[type]->active) {
		filters[type]->active = false;
		Sys_Printf("filter %i deactivated\n", type);
	} else {
		filters[type]->active = true;
		Sys_Printf("filter %i activated\n", type);
	}

	FiltersActivate();
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
