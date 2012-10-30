#include "anydsl2/type.h"

#include "anydsl2/lambda.h"
#include "anydsl2/literal.h"
#include "anydsl2/printer.h"
#include "anydsl2/world.h"
#include "anydsl2/util/for_all.h"

namespace anydsl2 {

//------------------------------------------------------------------------------

const Type*& GenericMap::operator [] (const Generic* generic) {
    size_t i = generic->index();
    if (i >= size())
        resize(i+1, 0);
    return get(i);
}

bool GenericMap::is_empty() const {
    bool result = true;
    for (size_t i = 0, e = size(); i != e && result; ++i)
        result &= !get(i);

    return result;
}

const char* GenericMap::to_string() const {
    std::ostringstream o;
    bool first = true;
    for (size_t i = 0, e = size(); i != e; ++i)
        if (const Type* type = get(i)) {
            if (first)
                first = false;
            else
                o << ", ";
            o << '_' << i << " = " << type;
        }

    return o.str().c_str();
}

//------------------------------------------------------------------------------

const Ptr* Type::to_ptr() const { return world().ptr(this); }

const Type* Type::elem_via_lit(const Def* def) const {
    return elem(def->primlit_value<size_t>());
}

bool Type::check_with(const Type* other) const {
    if (this == other || this->isa<Generic>())
        return true;

    if (this->kind() != other->kind() || this->size() != other->size())
        return false;

    bool result = true;
    for (size_t i = 0, e = size(); i != e && result; ++i)
        result &= this->elem(i)->check_with(other->elem(i));

    return result;
}

bool Type::infer_with(GenericMap& map, const Type* other) const {
    size_t num_elems = this->size();
    assert(num_elems == other->size());
    assert(this->isa<Generic>() || this->kind() == other->kind());

    if (this == other)
        return true;

    if (const Generic* generic = this->isa<Generic>()) {
        const Type*& mapped = map[generic];
        if (!mapped) {
            mapped = other;
            return true;
        } else
            return mapped == other;
    }

    bool result = true;
    for (size_t i = 0; i < num_elems && result; ++i)
        result &= this->elem(i)->infer_with(map, other->elem(i));

    return result;
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

size_t GenericBuilder::new_def() {
    size_t handle = index2generic_.size();
    index2generic_.push_back(0);
    return handle;
}

const Generic* GenericBuilder::use(size_t handle) {
    assert(handle < index2generic_.size());
    const Generic*& ref = index2generic_[handle];
    if (const Generic* generic = ref)
        return generic;

    return ref = world_.generic(index_++);
}

void GenericBuilder::pop() { 
    if (const Generic* generic = index2generic_.back()) {
        --index_;
        assert(generic->index() == index_);
    }
    index2generic_.pop_back(); 
}

//------------------------------------------------------------------------------

std::ostream& operator << (std::ostream& o, const Type* type) {
    Printer p(o, false);
    type->vdump(p);
    return p.o;
}

//------------------------------------------------------------------------------

} // namespace anydsl2
