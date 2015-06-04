#include <CommonLib/StdAfx.h>
#include <CommonLib/Math/BoundingVolume.h>
namespace fastbird{
	void BoundingVolume::FinishSmartPtr(){
		assert(NumRefs() == 0);
		FB_DELETE(this);
	}
}