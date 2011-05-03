// Copyright (C) 2008-2011 by Vincent Falco, All rights reserved worldwide.
// This file is released under the MIT License:
// http://www.opensource.org/licenses/mit-license.php

#ifndef __VF_SHAREDOBJECT_VFHEADER__
#define __VF_SHAREDOBJECT_VFHEADER__

#include "vf/vf_Atomic.h"
#include "vf/vf_LeakChecked.h"

// A copy of juce::ReferenceCountedObject,
// with the following features:
//
// - Shorter name
//
// - Return value of decReferenceCount() indicates the
//   object was deleted (This can be handy for assertions)
//
// - Derived class may override the behavior of destruction.
//
// - Default behavior performs the delete on a separate thread.
//

class SharedObject : LeakChecked <SharedObject>, NonCopyable
{
public:
  inline void incReferenceCount() noexcept
  {
    m_refs.addref ();
  }

  inline bool decReferenceCount() noexcept
  {
    vfassert (m_refs.is_signaled());

    const bool final = m_refs.release ();

    if (final)
      destroySharedObject ();
    
    return final;
  }

  // Caller must synchronize.
  inline bool isBeingReferenced () const
  {
    return m_refs.is_signaled();
  }

protected:
  SharedObject()
  {
  }

  virtual ~SharedObject()
  {
    vfassert (m_refs.is_reset ());
  }

  // default implementation performs the delete
  // on a separate, provided thread that cleans up
  // after itself on exit.
  //
  virtual void destroySharedObject ();

protected:
  // declaration in vf_SharedSingleton.h to break #include cycle.
  class Singleton;

private:
  Atomic::Counter m_refs;
};

//
// RAII wrapper for a SharedObject
//
template <class SharedObjectClass>
class SharedObjectPtr
{
public:
  inline SharedObjectPtr() noexcept
    : referencedObject (0)
  {
  }

  inline SharedObjectPtr (SharedObjectClass* const refCountedObject) noexcept
    : referencedObject (refCountedObject)
  {
    if (refCountedObject != nullptr)
      refCountedObject->incReferenceCount();
  }

  inline SharedObjectPtr (const SharedObjectPtr<SharedObjectClass>& other) noexcept
    : referencedObject (other.referencedObject)
  {
    if (referencedObject != nullptr)
      referencedObject->incReferenceCount();
  }

  SharedObjectPtr <SharedObjectClass>& operator= (const SharedObjectPtr<SharedObjectClass>& other)
  {
    SharedObjectClass* const newObject = other.referencedObject;

    if (newObject != referencedObject)
    {
      if (newObject != nullptr)
        newObject->incReferenceCount();

      SharedObjectClass* const oldObject = referencedObject;
      referencedObject = newObject;

      if (oldObject != nullptr)
        oldObject->decReferenceCount();
    }

    return *this;
  }

  SharedObjectPtr <SharedObjectClass>& operator= (SharedObjectClass* const newObject)
  {
    if (referencedObject != newObject)
    {
      if (newObject != nullptr)
        newObject->incReferenceCount();

      SharedObjectClass* const oldObject = referencedObject;
      referencedObject = newObject;

      if (oldObject != nullptr)
        oldObject->decReferenceCount();
    }

    return *this;
  }

  inline ~SharedObjectPtr ()
  {
    if (referencedObject != nullptr)
      referencedObject->decReferenceCount();
  }

  inline operator SharedObjectClass* () const noexcept
  {
    return referencedObject;
  }

  inline SharedObjectClass* operator-> () const noexcept
  {
    return referencedObject;
  }

  inline SharedObjectClass* getObject () const noexcept
  {
    return referencedObject;
  }

private:
  SharedObjectClass* referencedObject;
};

//
// Comparisons
//

template <class SharedObjectClass>
bool operator== (const SharedObjectPtr<SharedObjectClass>& object1, SharedObjectClass* const object2) noexcept
{
  return object1.getObject() == object2;
}

template <class SharedObjectClass>
bool operator== (const SharedObjectPtr<SharedObjectClass>& object1, const SharedObjectPtr<SharedObjectClass>& object2) noexcept
{
  return object1.getObject() == object2.getObject();
}

template <class SharedObjectClass>
bool operator== (SharedObjectClass* object1, SharedObjectPtr<SharedObjectClass>& object2) noexcept
{
  return object1 == object2.getObject();
}

template <class SharedObjectClass>
bool operator!= (const SharedObjectPtr<SharedObjectClass>& object1, const SharedObjectClass* object2) noexcept
{
  return object1.getObject() != object2;
}

template <class SharedObjectClass>
bool operator!= (const SharedObjectPtr<SharedObjectClass>& object1, SharedObjectPtr<SharedObjectClass>& object2) noexcept
{
  return object1.getObject() != object2.getObject();
}
template <class SharedObjectClass>
bool operator!= (SharedObjectClass* object1, SharedObjectPtr<SharedObjectClass>& object2) noexcept
{
  return object1 != object2.getObject();
}

#endif

