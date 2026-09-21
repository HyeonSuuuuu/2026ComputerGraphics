export module hs.component;

import std;
import hs.check;

export namespace hs
{
	// 오브젝트에 기능 추가 / 제거 
	template<class T>
	class Component
	{
	public:
		// 이미 붙어있으면 잡
		template<class... Args>
		T& Add(Args&&... args)
		{
			Check(!_value.has_value(), "이미 컴포넌트가 붙어있음. 값을 바꾸려면 Get이나 ->를 사용");
			return _value.emplace(std::forward<Args>(args)...);
		}

		void Remove() { _value.reset(); }

		explicit operator bool() const { return _value.has_value(); }

		T* Get() { return _value ? &*_value : nullptr; }
		const T* Get() const { return _value ? &*_value : nullptr; }

		T* operator->() { Check(_value.has_value(), "붙어 있지 않은 컴포넌트 사용"); return &*_value; }
		const T* operator->() const { Check(_value.has_value(), "붙어 있지 않은 컴포넌트 사용"); return &*_value; }
		T& operator*() { return *operator->(); }
		const T& operator*() const { return *operator->(); }

	private:
		std::optional<T> _value;
	};
}
