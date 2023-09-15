/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * CompressMGARDPlus.h :
 *
 *  Created on: Feb 23, 2022
 *      Author: Jason Wang jason.ruonan.wang@gmail.com
 */

#ifndef ADIOS2_OPERATOR_COMPRESS_COMPRESSMGARDPLUS_H_
#define ADIOS2_OPERATOR_COMPRESS_COMPRESSMGARDPLUS_H_

#include "adios2/core/Operator.h"
#include <torch/torch.h>

namespace adios2
{
namespace core
{
namespace compress
{

class CompressMGARDPlus : public Operator
{

public:
    CompressMGARDPlus(const Params &parameters);

    ~CompressMGARDPlus() = default;

    /**
     * @param dataIn
     * @param blockStart
     * @param blockCount
     * @param type
     * @param bufferOut
     * @return size of compressed buffer
     */
    size_t Operate(const char *dataIn, const Dims &blockStart,
                   const Dims &blockCount, const DataType type,
                   char *bufferOut) final;

    /**
     * @param bufferIn
     * @param sizeIn
     * @param dataOut
     * @return size of decompressed buffer
     */
    size_t InverseOperate(const char *bufferIn, const size_t sizeIn,
                          char *dataOut) final;

    bool IsDataTypeValid(const DataType type) const final;

private:
    /**
     * Decompress function for V1 buffer. Do NOT remove even if the buffer
     * version is updated. Data might be still in lagacy formats. This function
     * must be kept for backward compatibility
     * @param bufferIn : compressed data buffer (V1 only)
     * @param sizeIn : number of bytes in bufferIn
     * @param dataOut : decompressed data buffer
     * @return : number of bytes in dataOut
     */
    size_t DecompressV1(const char *bufferIn, size_t bufferInOffset,
        const size_t sizeIn, char *dataOut);

    Dims GetBlockDims(const char *bufferIn, size_t bufferInOffset);
    double getPDError(double eb, at::Tensor &orig, at::Tensor &decode, at::Tensor &perm_diff, Dims blockStart, Dims blockCount, const DataType type, char *bufferOut, double vx, double vy, double pd_omax_b, double pd_omin_b);
    double binarySearchEB(double lowereb, double uppereb, at::Tensor &orig, at::Tensor &decode, at::Tensor &perm_diff, Dims blockStart, Dims blockCount, const DataType type, char *bufferOut, double vx, double vy, double pd_omax_b, double pd_omin_b, double targetE);
    std::string m_VersionInfo;
};

} // end namespace compress
} // end namespace core
} // end namespace adios2

#endif /* ADIOS2_OPERATOR_COMPRESS_COMPRESSMGARDPLUS_H_ */
