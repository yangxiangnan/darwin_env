//===-- ModuleChild.h -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_ModuleChild_h_
#define liblldb_ModuleChild_h_

#include "lldb/lldb-private.h"

namespace lldb_private {

//----------------------------------------------------------------------
/// @class ModuleChild ModuleChild.h "lldb/Core/ModuleChild.h"
/// @brief A mix in class that contains a pointer back to the module
///        that owns the object which inherits from it.
//----------------------------------------------------------------------
class ModuleChild
{
public:
    //------------------------------------------------------------------
    /// Construct with owning module.
    ///
    /// @param[in] module
    ///     The module that owns the object that inherits from this
    ///     class.
    //------------------------------------------------------------------
    ModuleChild (Module* module);

    //------------------------------------------------------------------
    /// Copy constructor.
    ///
    /// @param[in] rhs
    ///     A const ModuleChild class reference to copy.
    //------------------------------------------------------------------
    ModuleChild (const ModuleChild& rhs);

    //------------------------------------------------------------------
    /// Destructor.
    ///
    /// The destructor is virtual since this class is designed to be
    /// inherited from.
    //------------------------------------------------------------------
    virtual
    ~ModuleChild();

    //------------------------------------------------------------------
    /// Assignment operator.
    ///
    /// @param[in] rhs
    ///     A const ModuleChild class reference to copy.
    ///
    /// @return
    ///     A const reference to this object.
    //------------------------------------------------------------------
    const ModuleChild&
    operator= (const ModuleChild& rhs);

    //------------------------------------------------------------------
    /// Get accessor for the module pointer.
    ///
    /// @return
    ///     A pointer to the module that owns this object.
    //------------------------------------------------------------------
    Module *
    GetModule ();

    //------------------------------------------------------------------
    /// Get const accessor for the module pointer.
    ///
    /// @return
    ///     A const pointer to the module that owns the object that
    ///     inherits from this class.
    //------------------------------------------------------------------
    Module *
    GetModule () const;

    //------------------------------------------------------------------
    /// Set accessor for the module pointer.
    ///
    /// @param[in] module
    ///     A new module that owns the object that inherits from this
    ///      class.
    //------------------------------------------------------------------
    void
    SetModule (Module *module);

protected:
    //------------------------------------------------------------------
    // Member variables
    //------------------------------------------------------------------
    Module *m_module;   ///< The Module that owns the object that inherits
                        ///< from this class.
};

} // namespace lldb_private


#endif  // liblldb_ModuleChild_h_
