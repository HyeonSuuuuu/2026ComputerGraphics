export module hs.component;

export namespace hs
{
	// 오브젝트에 붙는 기능 한 조각. 데이터만 들고 실행은 시스템이 한다
	class IComponent
	{
	public:
		virtual ~IComponent() = default;
	};
}
