#include "anydsl2/type.h"

#include "anydsl2/lambda.h"
#include "anydsl2/literal.h"
#include "anydsl2/world.h"
#include "anydsl2/util/for_all.h"

namespace anydsl2 {

//------------------------------------------------------------------------------

const Ptr* Type::to_ptr() const { return world().ptr(this); }

const Type* Type::elem_via_lit(const Def* def) const {
    return elem(def->primlit_value<size_t>());
}

//------------------------------------------------------------------------------

PrimType::PrimType(World& world, PrimTypeKind kind)
    : Type(world, kind, 0)
{
    debug = kind2str(primtype_kind());
}

//------------------------------------------------------------------------------

CompoundType::CompoundType(World& world, int kind, size_t size)
    : Type(world, kind, size)
{}

CompoundType::CompoundType(World& world, int kind, Elems elems)
    : Type(world, kind, elems.size())
{
    size_t x = 0;
    for_all (elem, elems)
        set(x++, elem);
}

//------------------------------------------------------------------------------

size_t Sigma::hash() const {
    return named_ ? boost::hash_value(this) : CompoundType::hash();
}

bool Sigma::equal(const Node* other) const {
    return named_ ? this == other : CompoundType::equal(other);
}

//------------------------------------------------------------------------------

template<bool first_order>
bool Pi::classify_order() const {
    for_all (elem, elems())
        if (first_order ^ (elem->isa<Pi>() == 0))
            return false;

    return true;
}

bool Pi::is_fo() const { return classify_order<true>(); }
bool Pi::is_ho() const { return classify_order<false>(); }

//------------------------------------------------------------------------------

size_t Generic::hash() const { 
    size_t seed = 0;
    boost::hash_combine(seed, Node_Generic);
    boost::hash_combine(seed, index_);
    return seed;
}

bool Generic::equal(const Node* other) const { 
    return other->kind() == Node_Generic && index_ == other->as<Generic>()->index();
}

//------------------------------------------------------------------------------

} // namespace anydsl2
