
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

#define UFOAI_CONTENTS_LEVEL_1 0x0100
#define UFOAI_CONTENTS_LEVEL_2 0x0200
#define UFOAI_CONTENTS_LEVEL_3 0x0400
#define UFOAI_CONTENTS_LEVEL_4 0x0800
#define UFOAI_CONTENTS_LEVEL_5 0x1000
#define UFOAI_CONTENTS_LEVEL_6 0x2000
#define UFOAI_CONTENTS_LEVEL_7 0x4000
#define UFOAI_CONTENTS_LEVEL_8 0x8000

#define UFOAI_CONTENTS_ACTORCLIP   0x00010000
#define UFOAI_CONTENTS_WEAPONCLIP  0x02000000
#define UFOAI_CONTENTS_STEPON      0x40000000

#define UFOAI_SURF_NODRAW    0x00000080


void DoSteponFiltering(void);
void DoWeaponClipFiltering(void);
void DoActorClipFiltering(void);
void DoNoDrawFiltering(void);
void DoLevel1Filtering(void);
void DoLevel2Filtering(void);
void DoLevel3Filtering(void);
void DoLevel4Filtering(void);
void DoLevel5Filtering(void);
void DoLevel6Filtering(void);
void DoLevel7Filtering(void);
void DoLevel8Filtering(void);

// add the ufoai filters
void UFOAIFilterInit(void);
