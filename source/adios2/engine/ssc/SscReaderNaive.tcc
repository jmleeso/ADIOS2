/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * SscReaderNaive.tcc
 *
 *  Created on: Mar 7, 2022
 *      Author: Jason Wang
 */

#ifndef ADIOS2_ENGINE_SSCREADERNAIVE_TCC_
#define ADIOS2_ENGINE_SSCREADERNAIVE_TCC_

#include "SscReaderNaive.h"
#include "adios2/helper/adiosMemory.h"

namespace adios2
{
namespace core
{
namespace engine
{
namespace ssc
{

template <>
void SscReaderNaive::GetDeferredCommon(Variable<std::string> &variable,
                                       std::string *data)
{
    variable.SetData(data);

    for (const auto &b : m_BlockMap[variable.m_Name])
    {
        if (b.name == variable.m_Name)
        {
            *data =
                std::string(m_Buffer.data() + b.bufferStart,
                            m_Buffer.data() + b.bufferStart + b.bufferCount);
            variable.m_Value = *data;
            variable.m_Min = *data;
            variable.m_Max = *data;
        }
    }
}

template <class T>
void SscReaderNaive::GetDeferredCommon(Variable<T> &variable, T *data)
{
    variable.SetData(data);

    Dims vStart = variable.m_Start;
    Dims vCount = variable.m_Count;
    Dims vShape = variable.m_Shape;

    if (m_IO.m_ArrayOrder != ArrayOrdering::RowMajor)
    {
        std::reverse(vStart.begin(), vStart.end());
        std::reverse(vCount.begin(), vCount.end());
        std::reverse(vShape.begin(), vShape.end());
    }

    for (const auto &b : m_BlockMap[variable.m_Name])
    {
        if (b.shapeId == ShapeID::GlobalArray ||
            b.shapeId == ShapeID::LocalArray)
        {
            helper::NdCopy(m_Buffer.data<char>() + b.bufferStart, b.start,
                           b.count, true, true, reinterpret_cast<char *>(data),
                           vStart, vCount, true, true, sizeof(T));
        }
        else if (b.shapeId == ShapeID::GlobalValue ||
                 b.shapeId == ShapeID::LocalValue)
        {
            std::memcpy(data, m_Buffer.data() + b.bufferStart, b.bufferCount);
        }
    }
}

template <typename T>
std::vector<typename Variable<T>::BPInfo>
SscReaderNaive::BlocksInfoCommon(const Variable<T> &variable,
                                 const size_t step) const
{

    std::vector<typename Variable<T>::BPInfo> ret;

    return ret;
}

}
}
}
}

#endif // ADIOS2_ENGINE_SSCREADERNAIVE_TCC_
