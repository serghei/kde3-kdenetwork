#ifndef __stringparserstate_h__
#define __stringparserstate_h__

// ### optimize me: it's stupid to do a linear search for each
// atEnd() invocation. This should be done once in the ctor and
// end() should be set appropriately
template <typename CharType, typename _SizeType = size_t> 
class StringParserState
{
public:
    typedef CharType ValueType;
    typedef const CharType * ConstIterator;
    typedef _SizeType SizeType;
    typedef QValueList<ValueType> ValueList;

    StringParserState( ConstIterator start, SizeType maxSteps,
                       const ValueList &optionalEndItems = ValueList() )
    {
	m_begin = start;
        m_current = start;
        m_end = start + maxSteps;
        m_endItems = optionalEndItems;
    }

    void operator++() { ++m_current; }

    bool atBegin() const
    {
	return m_current == m_begin;
    }

    bool atEnd() const
    { 
        if ( m_current >= m_end )
            return true;
        return m_endItems.findIndex( currentValue() ) != -1;
    }

    ConstIterator current() const { return m_current; }
    ValueType currentValue() const { return *current(); }

    ConstIterator begin() const { return m_begin; }
    ConstIterator end() const { return m_end; }

    SizeType stepsLeft() const { return m_end - m_current; }

    SizeType advanceTo( ValueType val )
    {
	ConstIterator start = m_current;
	while ( !atEnd() && currentValue() != val )
	    ++m_current;
	return m_current - start;
    }

    SizeType skip( ValueType valueToIgnore )
    {
	ConstIterator start = m_current;
	while ( !atEnd() && currentValue() == valueToIgnore )
	    ++m_current;
	return m_current - start;
    }

private:
    ConstIterator m_begin;
    ConstIterator m_current;
    ConstIterator m_end;
    ValueList m_endItems;
};

#endif

