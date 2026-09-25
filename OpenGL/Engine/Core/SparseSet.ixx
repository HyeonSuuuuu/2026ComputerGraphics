module;
#include "Core/Check.h"

export module hs.sparse_set;

import std;
import hs.check;

export namespace hs
{
	// 키로 바로 찾기 + 빈틈없는 순회
	// 삭제는 끝 원소로 구멍을 메움 → 순서가 바뀌고, 받아 둔 참조가 다른 원소를 가리킬 수 있음
	template<class T>
	class SparseSet
	{
	public:
		bool Contains(std::uint32_t key) const
		{
			return key < _sparse.size() && _sparse[key] < _dense.size() && _dense[_sparse[key]] == key;
		}

		template<class... Args>
		T& Add(std::uint32_t key, Args&&... args)
		{
			HS_DCHECK(!Contains(key), "같은 키로 두 번 추가");

			if (key >= _sparse.size())
				_sparse.resize(key + 1);
			_sparse[key] = static_cast<std::uint32_t>(_dense.size());
			_dense.push_back(key);
			return _data.emplace_back(std::forward<Args>(args)...);
		}

		T& Get(std::uint32_t key)
		{
			HS_DCHECK(Contains(key), "없는 키");
			return _data[_sparse[key]];
		}

		void Remove(std::uint32_t key)
		{
			HS_DCHECK(Contains(key), "없는 키");

			std::uint32_t hole = _sparse[key];
			std::uint32_t last = static_cast<std::uint32_t>(_dense.size() - 1);
			if (hole != last)	// 자기 자신으로 move하면 vector 등은 비워질 수 있음
			{
				_dense[hole] = _dense[last];
				_data[hole] = std::move(_data[last]);
				_sparse[_dense[hole]] = hole;
			}
			_dense.pop_back();
			_data.pop_back();
		}

		void Clear()
		{
			_sparse.clear();
			_dense.clear();
			_data.clear();
		}

		std::size_t Size() const { return _dense.size(); }
		std::uint32_t KeyAt(std::size_t position) const { return _dense[position]; }
		T& At(std::size_t position) { return _data[position]; }

	private:
		std::vector<std::uint32_t> _sparse;	// 키 → 칸
		std::vector<std::uint32_t> _dense;	// 칸 → 키
		std::vector<T> _data;				// _dense와 같은 순서
	};
}
