export module hs.type_id;

import std;

namespace hs
{
    using namespace std::meta;

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

    consteval std::string Name(info r);

    // "app04::" / 클래스 안이면 "Outer<int>::"
    consteval std::string Path(info r)
    {
        info parent = parent_of(r);
        return parent == ^^:: ? "" : Name(parent) + "::";
    }

    // display_string_of는 import한 쪽에서만 "@모듈"을 붙임 → 모듈마다 이름이 달라져 ID가 갈림
    // 그래서 타입을 조각으로 분해해 이름 있는 선언은 identifier_of로, 기본 타입·정수 값만 display_string_of로
    // 규칙 없는 경우(배열·함수 타입·volatile·익명 네임스페이스 등)는 identifier_of에서 컴파일 에러 → 조용히 틀리지 않음
    consteval std::string Name(info r)
    {
        if (is_type(r))
        {
            r = dealias(r);
            if (is_const_type(r))               return "const " + Name(remove_const(r));
            if (is_pointer_type(r))             return Name(remove_pointer(r)) + "*";
            if (is_lvalue_reference_type(r))    return Name(remove_reference(r)) + "&";
            if (is_fundamental_type(r))         return std::string{ display_string_of(r) };

            if (has_template_arguments(r))
            {
                std::string name = Name(template_of(r)) + "<";
                std::string_view separator;
                for (info argument : template_arguments_of(r))
                {
                    name += separator;
                    name += Name(argument);
                    separator = ", ";
                }
                return name + ">";
            }
        }
        else if (is_value(r) && is_integral_type(type_of(r)))
        {
            return std::string{ display_string_of(r) };
        }

        return Path(r) + std::string{ identifier_of(r) };
    }
}

export namespace hs
{
    using TypeId = std::uint64_t;

    // 모듈·빌드와 무관하게 같은 이름 → 같은 ID
    template<class T>
    constexpr std::string_view TypeName = std::define_static_string(Name(^^T));

    template<class T>
    constexpr TypeId TypeIdOf = Fnv1a(TypeName<T>);
}
