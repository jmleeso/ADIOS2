/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * SscWriterNaive.cpp
 *
 *  Created on: Mar 7, 2022
 *      Author: Jason Wang
 */

#include "SscWriterNaive.tcc"

namespace adios2
{
namespace core
{
namespace engine
{
namespace ssc
{

SscWriterNaive::SscWriterNaive(IO &io, const std::string &name, const Mode mode,
                               MPI_Comm comm)
: SscWriterBase(io, name, mode, comm)
{
}

StepStatus SscWriterNaive::BeginStep(const StepMode mode,
                                     const float timeoutSeconds,
                                     const bool writerLocked)
{
    ++m_CurrentStep;

    m_Buffer.clear();
    m_Buffer.resize(16);
    m_Metadata.clear();

    return StepStatus::OK;
}

size_t SscWriterNaive::CurrentStep() { return m_CurrentStep; }

void SscWriterNaive::PerformPuts() {}

void SscWriterNaive::EndStep(const bool writerLocked)
{
    m_Buffer.value<uint64_t>() = m_Buffer.size();
    m_Buffer.value<uint64_t>(8) = m_Buffer.size();

    ssc::SerializeVariables(m_Metadata, m_Buffer, m_WriterRank);

    if (m_WriterRank == 0)
    {
        ssc::SerializeAttributes(m_IO, m_Buffer);
    }

    int localSize = static_cast<int>(m_Buffer.value<uint64_t>());
    std::vector<int> localSizes(m_WriterSize);
    MPI_Gather(&localSize, 1, MPI_INT, localSizes.data(), 1, MPI_INT, 0,
               m_WriterComm);
    size_t globalSize =
        std::accumulate(localSizes.begin(), localSizes.end(), 0);
    ssc::Buffer globalBuffer(globalSize);

    std::vector<int> displs(m_WriterSize);
    for (size_t i = 1; i < static_cast<size_t>(m_WriterSize); ++i)
    {
        displs[i] = displs[i - 1] + localSizes[i - 1];
    }

    MPI_Gatherv(m_Buffer.data(), localSize, MPI_CHAR, globalBuffer.data(),
                localSizes.data(), displs.data(), MPI_CHAR, 0, m_WriterComm);

    if (m_WriterRank == 0)
    {
        MPI_Send(&globalSize, 1, MPI_UNSIGNED_LONG_LONG,
                 m_ReaderMasterStreamRank, 0, m_StreamComm);
        MPI_Send(globalBuffer.data(), globalSize, MPI_CHAR,
                 m_ReaderMasterStreamRank, 0, m_StreamComm);
    }
}

void SscWriterNaive::Close(const int transportIndex)
{

    uint64_t globalSize = 1;
    ssc::Buffer globalBuffer(globalSize);
    if (m_WriterRank == 0)
    {
        MPI_Send(&globalSize, 1, MPI_UNSIGNED_LONG_LONG,
                 m_ReaderMasterStreamRank, 0, m_StreamComm);
        MPI_Send(globalBuffer.data(), globalSize, MPI_CHAR,
                 m_ReaderMasterStreamRank, 0, m_StreamComm);
    }
}

#define declare_type(T)                                                        \
    void SscWriterNaive::PutDeferred(Variable<T> &variable, const T *data)     \
    {                                                                          \
        PutDeferredCommon(variable, data);                                     \
    }
ADIOS2_FOREACH_STDTYPE_1ARG(declare_type)
#undef declare_type

}
}
}
}
