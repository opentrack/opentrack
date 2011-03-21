////////////////////////////////////////////////////////////////////////////////
// Auto pointer template.

#if !defined(AUTOCLOSEPTR_H)
#define AUTOCLOSEPTR_H

//////////////////////////////////////////////////////////////////////////////
// T auto pointer (not suitable for containers).
// This auto pointer uses T's member function to perform clean up, rather
// than `operator Closeete'.
//
template<typename T, typename R, R (T::*Close)()>
class AutoClosePtr
{
public:
    explicit AutoClosePtr(T* p = NULL)
        : m_p(p)
    {  // construct from object pointer
    }

    AutoClosePtr(AutoClosePtr& Other)
        : m_p(Other.Release())
    {  // construct from Other AutoClosePtr
    }

    AutoClosePtr& operator=(AutoClosePtr& Other)
    {  // assign Other
        Reset(Other.Release());
        return (*this);
    }

    ~AutoClosePtr()
    {  // close and destroy
        if(m_p)
        {
            (m_p->*Close)();
            m_p = NULL;
        }
    }

    T& operator*() const
    {  // return T
        return (*m_p);
    }

    T** operator&()
    {  // return the address of the wrapped pointer
        Reset();
        return &m_p;
    }

    T* operator->() const
    {  // return wrapped pointer
        return Get();
    }

    operator bool() const
    {  // check wrapped pointer for NULL 
        return m_p != NULL;
    }

    T* Get() const
    {  // return wrapped pointer
        return m_p;
    }

    T* Release()
    {  // return wrapped pointer and give up ownership
        T* pTmp = m_p;
        m_p = NULL;
        return pTmp;
    }

    void Reset(T* p = NULL)
    {  // destroy the object and store new pointer
        if(p != m_p)
        {
            if(m_p)
                (m_p->*Close)();
        }

        m_p = p;
    }

private:
    T* m_p;    // the wrapped raw pointer
};

#endif // AUTOCLOSEPTR_H