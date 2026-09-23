export module hs.type_id;

import std;

namespace hs
{
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    consteval std::uint64_t Fnv1a(std::string_view text)
    {
        std::uint64_t hash = 14695981039346656037ull;
        for (char c : text)
        {
            hash ^= static_cast<unsigned char>(c);
            hash *= 1099511628211ull;
        }
        return hash;
    }
}

export namespace hs
{
    using TypeId = std::uint64_t;
    
    template<class T>
    // display_string_of -> 컴파일마다 동작 다름 (GCC 기준으로 작성함)
    constexpr std::string_view TypeName = std::define_static_string(std::meta::display_string_of(^^T));
    
    template<class T>
    constexpr TypeId TypeIdOf = Fnv1a(TypeName<T>);
}