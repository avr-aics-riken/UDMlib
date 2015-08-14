// ##################################################################################
//
// UDMlib - Unstructured Data Management Library
//
// Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
// All rights reserved.
//
// Copyright (c) 2015 Advanced Institute for Computational Science, RIKEN.
// All rights reserved.
//
// ###################################################################################

#ifndef _UDMSOLID_H_
#define _UDMSOLID_H_
/**
 * @file UdmSolid.h
 * Solid(3D)要素クラスのヘッダーファイル
 */

#include "model/UdmCell.h"

namespace udm
{

/**
 * Solid(3D)要素クラス
 */
class UdmSolid: public UdmCell
{
public:
    UdmSolid();
    UdmSolid(UdmElementType_t element_type);
    virtual ~UdmSolid();
    UdmError_t createComponentCells();
    int getDimensions() const;

private:
    void initialize();
    UdmSize_t createComponentCell(UdmElementType_t element_type, unsigned int num_nodes, UdmSize_t face_nodes[]);
};

} /* namespace udm */

#endif /* _UDMSOLID_H_ */
