export module hs.rect_layer;

import std;
import hs.shape;
import hs.entity;

export namespace hs
{
	using RectId = EntityId;

	class RectLayer
	{
	public:
		struct Entry
		{
			RectId id;
			Rectangle rect;
		};

		// 저장소가 직접 ID를 발급한다. World 없이 단독으로 쓸 때의 경로
		RectId Add(const Rectangle& rect)
		{
			RectId id = _nextId++;
			_entries.push_back({ id, rect });
			return id;
		}

		// World 등 바깥에서 발급한 ID로 추가한다
		void Add(RectId id, const Rectangle& rect)
		{
			_entries.push_back({ id, rect });
			if (id >= _nextId)			// 내부 발급과 번호가 겹치지 않게 밀어 둔다
				_nextId = id + 1;
		}

		bool Remove(RectId id)
		{
			auto it = FindEntry(id);
			if (it == _entries.end())
				return false;
			_entries.erase(it);
			return true;
		}
		
		void Clear() { _entries.clear(); }
		
		Rectangle* Find(RectId id)
		{
			auto it = FindEntry(id);
			return it != _entries.end() ? &it->rect : nullptr;
		}

		const Rectangle* Find(RectId id) const
		{
			auto it = FindEntry(id);
			return it != _entries.end() ? &it->rect : nullptr;
		}
		
		std::optional<RectId> HitTest(Vec2 p) const
		{
			for (const auto& entry : _entries | std::views::reverse)
				if (entry.rect.Contains(p))
					return entry.id;
			return std::nullopt;
		}
		
		std::optional<RectId> FindOverlap(RectId id) const
		{
			const Rectangle* target = Find(id);
			if (!target)
				return std::nullopt;

			for (const auto& entry : _entries | std::views::reverse)
				if (entry.id != id && entry.rect.Intersects(*target))
					return entry.id;
			return std::nullopt;
		}
		
		std::span<const Entry> Entries() const { return _entries; }
		std::size_t Size() const { return _entries.size(); }
		bool IsEmpty() const { return _entries.empty(); }

	private:
		auto FindEntry(RectId id) { return std::ranges::find(_entries, id, &Entry::id); }
		auto FindEntry(RectId id) const { return std::ranges::find(_entries, id, &Entry::id); }

		std::vector<Entry> _entries;
		RectId _nextId{};
	};
}
