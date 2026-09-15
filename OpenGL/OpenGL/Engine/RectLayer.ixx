export module hs.rect_layer;

import std;
import hs.shape;

export namespace hs
{
	using RectId = std::uint32_t;
	
	class RectLayer
	{
	public:
		struct Entry
		{
			RectId id;
			Rectangle rect;
		};
		
		RectId Add(const Rectangle& rect)
		{
			RectId id = _nextId++;
			_entries.push_back({ id, rect });
			return id;
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
