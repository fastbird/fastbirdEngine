#pragma once
namespace fastbird
{
	class ICheckbox
	{
	public:
		virtual void SetCheck(bool check) = 0;
		virtual bool GetCheck() const = 0;
	};
}