/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMGENERAL_H_
#define _UDMGENERAL_H_

#include <cgnslib.h>
#include "UdmBase.h"
#include "utils/UdmSerialization.h"

namespace udm {

class UdmEntity;
class UdmNode;
class UdmCell;
class UdmComponent;

/**
 * モデル構成の基底クラス
 */
class UdmGeneral : public UdmBase {
private:

    /**
     * ID : CGNSノードID、要素ローカルID, ノードローカルID
     */
    UdmSize_t id;

    /**
     * CGNSノードの名前です。
     */
    std::string name;

    /**
     * CGNSノードのラベルです。
     */
    std::string label;

    /**
     * CGNSノードのデータ型を定義します.
     */
    UdmDataType_t datatype;

public:
    UdmGeneral();
    UdmGeneral(UdmDataType_t dataType);
    UdmGeneral(const std::string &name, UdmSize_t id);
    UdmGeneral(const std::string &name, UdmSize_t id, UdmDataType_t dataType);
    virtual ~UdmGeneral();
    UdmDataType_t getDataType() const;
    void setDataType(UdmDataType_t dataType);
    UdmSize_t getId() const;
    void setId(UdmSize_t id);
    const std::string& getLabel() const;
    void setLabel(const std::string& label);
    const std::string& getName() const;
    void setName(const std::string& name);
    bool equalsName(const std::string &dest_name);

    // CGNS
    static UdmDataType_t toUdmDataType(DataType_t cgns_datatype);
    static DataType_t toCgnsDataType(UdmDataType_t datatype);
    static UdmElementType_t toElementType(ElementType_t cgns_elementtype);
    static ElementType_t toCgnsElementType(UdmElementType_t elementtype);
    static UdmGridLocation_t toGridLocation(GridLocation_t cgns_location);
    static GridLocation_t toCgnsGridLocation(UdmGridLocation_t grid_location);
    static int getNumVertexOfElementType(UdmElementType_t elementtype);
    static int getNumComponentOfElementType(UdmElementType_t elementtype);
    static UdmError_t toStringElementType(UdmElementType_t value, std::string &buf);
    static UdmSimulationType_t toSimulationType(SimulationType_t cgns_simulationtype);
    static bool isSupportElementType(UdmElementType_t elementtype);

    // シリアライズ
    static UdmSerializeArchive& serializeGeneralBase(
                    UdmSerializeArchive &archive,
                    UdmSize_t id,
                    UdmDataType_t datatype,
                    const std::string &name );
    static UdmSerializeArchive& deserializeGeneralBase(
                    UdmSerializeArchive &archive,
                    UdmSize_t &id,
                    UdmDataType_t &datatype,
                    std::string &name );
    static UdmSerializeArchive& serializeGeneralBase(
                            UdmSerializeArchive& archive,
                            UdmDataType_t datatype,
                            const std::string &name);
    static UdmSerializeArchive& deserializeGeneralBase(
                    UdmSerializeArchive &archive,
                    UdmDataType_t &datatype,
                    std::string &name );

    // memory size
    size_t getMemSize() const;

protected:
    template<class DATA_TYPE>
    UdmDataType_t getDataType();
    static void* createDataArray(UdmDataType_t datatype, UdmSize_t size);
    static void deleteDataArray(void* array, UdmDataType_t datatype);
    static unsigned int sizeofDataType(UdmDataType_t datatype);
    static void* copyDataArray(void *src, UdmDataType_t src_type, const void *dest, UdmDataType_t dest_type, unsigned int size);

    // CGNS
    int getCgnsBaseIterativeDataIds(std::vector<UdmSize_t> &iterative_ids, int index_file, int index_base, int timeslice_step = -1);
    UdmError_t getCgnsIterativeFlowSolutionNames(std::vector<std::string> &solution_names, int index_file, int index_base, int index_zone, int timeslice_step);
    int getCgnsFlowSolutionId(int index_file, int index_base, int index_zone, const std::string &solution_name);
    bool existsCgnsBaseIterativeData(int index_file, int index_base) const;
    bool existsCgnsZoneIterativeData(int index_file, int index_base, int index_zone) const;
    bool existsCgnsIterativeData(int index_file, int index_base, int index_zone) const;

    // std::vector２分探索
    template<class DATA_TYPE>
    typename std::vector<DATA_TYPE*>::const_iterator searchEntityId(const std::vector<DATA_TYPE*>& list, UdmSize_t entity_id) const;
    template<class DATA_TYPE>
    typename std::vector<DATA_TYPE*>::const_iterator searchGlobalId(const std::vector<DATA_TYPE*>& list, const UdmEntity *src) const;
    template<class DATA_TYPE>
    typename std::vector<DATA_TYPE*>::const_iterator searchCurrentGlobalId(const std::vector<DATA_TYPE*>& list, int rankno, UdmSize_t entity_id) const;
    template<class DATA_TYPE>
    typename std::vector<DATA_TYPE*>::const_iterator searchPreviousGlobalId(const std::vector<DATA_TYPE*>& list, int rankno, UdmSize_t entity_id) const;
    template<class DATA_TYPE>
    typename std::vector<DATA_TYPE*>::const_iterator searchUpperGlobalId(const std::vector<DATA_TYPE*>& list, const UdmEntity *src) const;
    std::vector<UdmCell*>::const_iterator searchElemId(const std::vector<UdmCell*>& list, UdmSize_t elem_id) const;
    std::vector<UdmCell*>::const_iterator searchUpperElemId(const std::vector<UdmCell*>& list, UdmSize_t elem_id) const;
    std::vector<UdmComponent*>::const_iterator
    searchEntityId(const std::vector<UdmComponent*>& list, UdmSize_t entity_id) const;

    // clone
    UdmError_t cloneGeneral(const UdmGeneral &src);

    // path
    void getLinkedRelativePath(const std::string &link_output_path,
                            const std::string &linked_file,
                            std::string &linked_relativepath) const;

private:
    /**
     * 初期化を行う.
     */
    virtual void initialize(void) = 0;
};

template<> UdmDataType_t UdmGeneral::getDataType<int>();
template<> UdmDataType_t UdmGeneral::getDataType<long long>();
template<> UdmDataType_t UdmGeneral::getDataType<float>();
template<> UdmDataType_t UdmGeneral::getDataType<double>();


} /* namespace udm */
#endif /* _UDMGENERAL_H_ */
