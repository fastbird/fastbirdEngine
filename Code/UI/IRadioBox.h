#pragma once
namespace fastbird
{
	class IRadioBox
	{
	public:
		virtual void SetCheck(bool check) = 0;
		virtual bool GetCheck() const = 0;
		virtual void SetGroupID(int id) = 0;
		virtual int GetGroupID() const = 0;
	};
}