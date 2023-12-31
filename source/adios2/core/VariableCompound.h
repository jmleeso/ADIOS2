/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * VariableCompound.h
 *
 *  Created on: Feb 20, 2017
 *      Author: William F Godoy godoywf@ornl.gov
 */

#ifndef ADIOS2_CORE_VARIABLECOMPOUND_H_
#define ADIOS2_CORE_VARIABLECOMPOUND_H_

#include "VariableBase.h"

#include "adios2/common/ADIOSMacros.h"
#include "adios2/common/ADIOSTypes.h"

namespace adios2
{
namespace core
{

/**
 * @param Base (parent) class for template derived (child) class CVariable.
 * Required to put CVariable objects in STL containers.
 */
class VariableCompound : public VariableBase
{

public:
    const void *m_Data = nullptr;

    /** Primitive type element */
    struct Element
    {
        const std::string Name;
        const adios2::DataType Type; ///< from GetDataType<T>
        const size_t Offset;         ///< element offset in struct
    };

    /** vector of primitve element types defining compound struct */
    std::vector<Element> m_Elements;

    VariableCompound(const std::string &name, const size_t structSize,
                     const Dims &shape, const Dims &start, const Dims &count,
                     const bool constantDims);

    ~VariableCompound() = default;

    /**
     * Inserts an Element into the compound struct definition
     * @param name
     * @param offset
     */
    template <class T>
    void InsertMember(const std::string &name, const size_t offset);
};

} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_CORE_VARIABLECOMPOUND_H_ */
