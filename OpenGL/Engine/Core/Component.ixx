export module hs.component;

export namespace hs
{
	// 데이터만. 실행은 시스템 담당
	class IComponent
	{
	public:
		virtual ~IComponent() = default;
	};
}
