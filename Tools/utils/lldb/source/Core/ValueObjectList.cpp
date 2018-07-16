//===-- ValueObjectList.cpp -------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lldb/Core/ValueObjectList.h"

// C Includes
// C++ Includes
// Other libraries and framework includes
// Project includes
#include "lldb/Core/ValueObjectChild.h"
#include "lldb/Core/ValueObjectRegister.h"
#include "lldb/Core/ValueObjectVariable.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/Process.h"

using namespace lldb;
using namespace lldb_private;

ValueObjectList::ValueObjectList () :
    m_value_objects()
{
}

ValueObjectList::ValueObjectList (const ValueObjectList &rhs) :
    m_value_objects(rhs.m_value_objects)
{
}


ValueObjectList::~ValueObjectList ()
{
}

const ValueObjectList &
ValueObjectList::operator = (const ValueObjectList &rhs)
{
    if (this != &rhs)
        m_value_objects = rhs.m_value_objects;
    return *this;
}

void
ValueObjectList::Append (const ValueObjectSP &val_obj_sp)
{
    m_value_objects.push_back(val_obj_sp);
}

void
ValueObjectList::Append (const ValueObjectList &valobj_list)
{
    std::copy(valobj_list.m_value_objects.begin(),  // source begin
              valobj_list.m_value_objects.end(),    // source end
              back_inserter(m_value_objects));      // destination
    
}


uint32_t
ValueObjectList::GetSize() const
{
    return m_value_objects.size();
}

void
ValueObjectList::Resize (uint32_t size)
{
    m_value_objects.resize (size);
}

lldb::ValueObjectSP
ValueObjectList::GetValueObjectAtIndex (uint32_t idx)
{
    lldb::ValueObjectSP valobj_sp;
    if (idx < m_value_objects.size())
        valobj_sp = m_value_objects[idx];
    return valobj_sp;
}

void
ValueObjectList::SetValueObjectAtIndex (uint32_t idx, const ValueObjectSP &valobj_sp)
{
    if (idx >= m_value_objects.size())
        m_value_objects.resize (idx + 1);
    m_value_objects[idx] = valobj_sp;
}

ValueObjectSP
ValueObjectList::FindValueObjectByValueName (const char *name)
{
    ConstString name_const_str(name);
    ValueObjectSP val_obj_sp;
    collection::iterator pos, end = m_value_objects.end();
    for (pos = m_value_objects.begin(); pos != end; ++pos)
    {
        ValueObject *valobj = (*pos).get();
        if (valobj && valobj->GetName() == name_const_str)
        {
            val_obj_sp = *pos;
            break;
        }
    }
    return val_obj_sp;
}

ValueObjectSP
ValueObjectList::FindValueObjectByUID (lldb::user_id_t uid)
{
    ValueObjectSP valobj_sp;
    collection::iterator pos, end = m_value_objects.end();

    for (pos = m_value_objects.begin(); pos != end; ++pos)
    {
        // Watch out for NULL objects in our list as the list
        // might get resized to a specific size and lazily filled in
        ValueObject *valobj = (*pos).get();
        if (valobj && valobj->GetID() == uid)
        {
            valobj_sp = *pos;
            break;
        }
    }
    return valobj_sp;
}


ValueObjectSP
ValueObjectList::FindValueObjectByPointer (ValueObject *find_valobj)
{
    ValueObjectSP valobj_sp;
    collection::iterator pos, end = m_value_objects.end();

    for (pos = m_value_objects.begin(); pos != end; ++pos)
    {
        ValueObject *valobj = (*pos).get();
        if (valobj && valobj == find_valobj)
        {
            valobj_sp = *pos;
            break;
        }
    }
    return valobj_sp;
}

void
ValueObjectList::Swap (ValueObjectList &value_object_list)
{
    m_value_objects.swap (value_object_list.m_value_objects);
}
