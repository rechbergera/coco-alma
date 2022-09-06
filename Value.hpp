#ifdef VALUE_H

template <verif_mode_t mode>
constexpr void Value<mode>::sanity() const
{
    assert(implies(has_glitches(mode) && is_glitch_const(), is_stable_const()));
    assert(implies(!has_glitches(mode), m_const_stable));
}

// Constructor with constant for all cases
template<verif_mode_t mode>
constexpr Value<mode>::Value(bool val)
        : m_const_value(val), m_const_stable(!has_glitches(mode)), m_stable_pvs(), m_glitch_pvs()
{}

template<verif_mode_t M>
Value<M> operator~(const Value<M>& a)
{
    // Check that this implication always holds
    a.sanity();

    Value<M> result(false);
    result.m_const_stable = a.m_const_stable;
    result.m_const_value = !a.m_const_value;
    result.m_stable_pvs = a.m_stable_pvs;
    result.m_glitch_pvs = a.m_glitch_pvs;

    result.sanity();
    return result;
}

template<verif_mode_t mode>
Value<mode> operator&(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();

    const bool order = a.is_stable_const() || (!b.is_stable_const() && a.is_glitch_const());
    const Value<mode>& left =  order ? a : b;
    const Value<mode>& right = order ? b : a;

    Value<mode> result(false);

    // Perform the case distinction for the stable computation
    if (left.is_stable_const() && right.is_stable_const())
    {
        // It is a constant in both cases, result is a constant
        result.m_const_value = left.m_const_value & right.m_const_value;
        assert(result.is_stable_const());
    }
    else if (!left.is_stable_const() && !right.is_stable_const())
    {
        // It is symbolic in both cases, result is symbolic
        result.m_stable_pvs = left.m_stable_pvs & right.m_stable_pvs;
        assert(!result.is_stable_const());
    }
    else
    {
        // It is constant in left and symbolic in right
        assert(left.is_stable_const() && !right.is_stable_const());
        if (left.m_const_value == false)
        {
            // If the constant value is false, then result is constant
            assert(result.is_stable_const());
            assert(result.m_const_value == false);
        }
        else
        {
            // If the constant value is true, then result is symbolic
            result.m_stable_pvs = right.m_stable_pvs;
            assert(!result.is_stable_const());
        }
    }

    // Perform the case distinction for the glitch computation
    if (left.is_glitch_const() && right.is_glitch_const())
    {
        // It is a constant in both cases, should be covered already
        assert(result.m_const_value == (left.m_const_value & right.m_const_value));
        assert(result.is_glitch_const());
        // What is not covered is the stability computation
        result.m_const_stable = (left.m_const_stable && right.m_const_stable) ||
                                (left.m_const_stable && left.m_const_value == false) ||
                                (right.m_const_stable && right.m_const_value == false);
    }
    else if (!left.is_glitch_const() && !right.is_glitch_const())
    {
        // It is symbolic in both cases, result is symbolic
        result.m_glitch_pvs = left.m_glitch_pvs & right.m_glitch_pvs;
        assert(!result.is_glitch_const());
    }
    else
    {
        // It is constant in left and symbolic in right
        assert(left.is_glitch_const() && !right.is_glitch_const());
        if (left.m_const_stable)
        {
            // The constant is stable, so we can simplify
            if(left.m_const_value == false)
            {
                // If the constant value is false, then result is constant
                assert(result.is_glitch_const());
                assert(result.m_const_value == false);
            }
            else
            {
                // If the constant value is true, then result is symbolic
                result.m_glitch_pvs = right.m_glitch_pvs;
                assert(!result.is_glitch_const());
            }
        }
        else
        {
            // The constant is not stable, and could make the result false
            // In other words, the result is the biased symbolic argument
            result.m_glitch_pvs = +right.m_glitch_pvs;
            assert(!result.is_glitch_const());
        }
    }

    result.sanity();
    return result;
}

template<verif_mode_t mode>
Value<mode> operator^(const Value<mode>& a, const Value<mode>& b)
{
    a.sanity();
    b.sanity();

    const bool order = a.is_stable_const() || (!b.is_stable_const() && a.is_glitch_const());
    const Value<mode>& left =  order ? a : b;
    const Value<mode>& right = order ? b : a;

    Value<mode> result(false);

    // Perform the case distinction for the stable computation
    if (left.is_stable_const() && right.is_stable_const())
    {
        // It is a constant in both cases, result is a constant
        result.m_const_value = left.m_const_value ^ right.m_const_value;
        assert(result.is_stable_const());
    }
    else if (!left.is_stable_const() && !right.is_stable_const())
    {
        // It is symbolic in both cases, result is symbolic
        result.m_stable_pvs = left.m_stable_pvs ^ right.m_stable_pvs;
        assert(!result.is_stable_const());
    }
    else
    {
        // It is constant in left and symbolic in right
        assert(left.is_stable_const() && !right.is_stable_const());
        // Inherit from right because xor with constants does not change the set
        result.m_stable_pvs = right.m_stable_pvs;
        assert(!result.is_stable_const());
    }

    // Perform the case distinction for the glitch computation
    if (left.is_glitch_const() && right.is_glitch_const())
    {
        // It is a constant in both cases, should be covered already
        assert(result.m_const_value == (left.m_const_value ^ right.m_const_value));
        assert(result.is_glitch_const());
        // It is only stable if both parents are stable
        result.m_const_stable = (left.m_const_stable && right.m_const_stable);
    }
    else if (!left.is_glitch_const() && !right.is_glitch_const())
    {
        // It is symbolic in both cases, result is symbolic
        result.m_glitch_pvs = left.m_glitch_pvs ^ right.m_glitch_pvs;
        assert(!result.is_glitch_const());
    }
    else
    {
        // It is constant in left and symbolic in right
        assert(left.is_glitch_const() && !right.is_glitch_const());
        // Whether the constant is stable or not does not matter
        // In all cases, the symbolic argument is forwarded
        result.m_glitch_pvs = right.m_glitch_pvs;
        assert(!result.is_glitch_const());
    }

    result.sanity();
    return result;
}

template<verif_mode_t mode>
Value<mode> operator|(const Value<mode>& a, const Value<mode>& b)
{
    return ~(~a & ~b);
}

template<verif_mode_t mode>
ValueView<mode>& ValueView<mode>::operator=(Value<mode>& other)
{
    other.sanity();
    m_value.sanity();

    Value<mode> result(false);

    // Perform the update for the stable computation
    result.m_const_value = other.m_const_value;
    result.m_stable_pvs = other.m_stable_pvs;

    // Perform the case distinction for the glitch computation
    if (m_value.is_glitch_const() && other.is_glitch_const())
    {
        // It is a constant in both cases, should be covered already
        assert(result.m_const_value == other.m_const_value);
        assert(result.is_glitch_const());
        // It is only stable if both parents are stable and equivalent
        result.m_const_stable = (m_value.m_const_stable && other.m_const_stable &&
                                 (m_value.m_const_value == other.m_const_value));
    }
    else if (!m_value.is_glitch_const() && !other.is_glitch_const())
    {
        // It is symbolic in both cases, result is symbolic
        result.m_glitch_pvs = m_value.m_glitch_pvs | other.m_glitch_pvs;
        assert(!result.is_glitch_const());
    }
    else if (m_value.is_stable_const() && !other.is_glitch_const())
    {
        // The new glitchy value is the biased overwrite value
        result.m_glitch_pvs = +other.m_glitch_pvs;
        assert(!result.is_glitch_const());
    }
    else
    {
        assert(!m_value.is_glitch_const() && other.is_glitch_const());
        // The new glitchy value is the biased previous value
        result.m_glitch_pvs = +m_value.m_glitch_pvs;
        assert(!result.is_glitch_const());
    }

    result.sanity();
    return result;
}

#endif // VALUE_H