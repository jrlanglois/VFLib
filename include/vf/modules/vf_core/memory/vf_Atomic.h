// Copyright (C) 2008 by Vinnie Falco, this file is part of VFLib.
// See the file LICENSE.txt for licensing information.

#ifndef __VF_ATOMIC_VFHEADER__
#define __VF_ATOMIC_VFHEADER__

namespace Atomic {

class State
{
public:
  explicit State (const int s = 0) : m_value (s) { }

  // Caller must synchronize
  inline operator int () const { return m_value.get(); }

  inline bool tryChangeState (const int from, const int to)
  {
    return m_value.compareAndSetBool (to, from);
  }

  inline void changeState (const int from, const int to)
  {
#if VF_DEBUG
    const bool success = tryChangeState (from, to);
    vfassert (success);
#else
    tryChangeState (from, to);
#endif
  }

private:
  VF_JUCE::Atomic <int> m_value;
};

}

#endif
