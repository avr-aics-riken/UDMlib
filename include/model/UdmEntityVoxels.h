/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMENTITYVOXELS_H_
#define _UDMENTITYVOXELS_H_

/**
 * @file UdmEntityVoxels.h
 * ノード・要素構成クラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"
#include "model/UdmEntity.h"

namespace udm
{
class UdmZone;

/**
 * ノード・要素構成基底クラス.
 * 物理量を持つノード・要素のリスト管理を行うクラスの基底クラス.
 * グリッド座標クラス(UdmGridCoordinates),要素管理クラス(UdmSections)の基底クラス
 */
class UdmEntityVoxels: public UdmGeneral, public UdmISerializable
{
public:
    UdmEntityVoxels();
    virtual ~UdmEntityVoxels();
    UdmError_t setEntitySolutionArray(const std::string &field_name, UdmSize_t nodesize, int vector_size, UdmDataType_t datatype, void* solution_array, UdmMemArrayType_t memtype);
    UdmError_t getEntitySolutionArray(const std::string &field_name, UdmSize_t start_id, UdmSize_t end_id, int vector_size, UdmDataType_t datatype, void* solution_array, UdmMemArrayType_t memtype);
    virtual UdmSize_t getNumEntities() const = 0;
    virtual UdmZone* getParentZone() const = 0;
    // MPI
    MPI_Comm getMpiComm() const;
    int getMpiRankno() const;
    int getMpiProcessSize() const;

protected:

private:
    virtual UdmEntity* getEntityById(UdmSize_t entity_id) = 0;
    virtual const UdmEntity* getEntityById(UdmSize_t entity_id) const = 0;
    virtual const UdmSolutionFieldConfig* getSolutionFieldConfig(const std::string &solution_name) const = 0;

};

} /* namespace udm */

#endif /* _UDMENTITYVOXELS_H_ */
