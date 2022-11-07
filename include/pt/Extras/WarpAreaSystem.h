#pragma once

#include "syati.h"

namespace pt {
	class ErrorLayout : public LayoutActor {
	public:
		ErrorLayout();

		virtual void init (const JMapInfoIter& rIter);

		void printf(bool canPrint, const char* format, ...);
	};

	class WarpAreaStageTable {
	public:

		WarpAreaStageTable(bool init);
		void readTable(s32 selectedindex, bool useErrors);

		void selectWipeClose(s32 type, s32 fadeTime);
		void selectWipeOpen(s32 type, s32 fadeTime);

		const char* mDestStageName;
		s32 mDestScenarioNo;
		s32 mDestGreenScenarioNo;
		s32 mBCSVWipeType;
		s32 mBCSVWipeTime;
		s32 mIndex;
		bool mCanWarp;
		ErrorLayout* mErrorLayout;
	};
};