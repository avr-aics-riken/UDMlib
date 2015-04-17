/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMSECTION_H_
#define _UDMSECTION_H_

/**
 * @file UdmElements.h
 * セクション（要素構成）構成情報クラスのヘッダーファイル
 */

#include "model/UdmGeneral.h"

namespace udm
{
class UdmZone;
class UdmGridCoordinates;
class UdmSections;
class UdmCell;

/**
 * セクション（要素構成）情報クラス.
 * 要素のリストを管理するクラスです.
 * CGNS:Elements/Sectionに相当するクラスです.
 */
class UdmElements: public UdmGeneral, public UdmISerializable
{
friend class UdmSections;

private:
    UdmSections *parent_sections;                ///< 親要素管理クラス
    std::vector<UdmCell*> cell_list;        ///< 要素（セル）リスト

    /**
     * 要素形状タイプ
     */
    UdmElementType_t element_type;

    /**
     * 要素管理上の管理ID:開始管理ID.
     * 要素（セル）IDとは異なる。
     */
    UdmSize_t start_elemid;

    /**
     * 要素管理上の管理ID:終了管理ID.
     * 要素（セル）IDとは異なる。
     */
    UdmSize_t end_elemid;

    /**
     * セクション（要素構成）内の最小座標
     */
    // UdmCoordsValue min_coords;

    /**
     * セクション（要素構成）内の最大座標
     */
    // UdmCoordsValue max_coords;

public:
    UdmElements();
    UdmElements(UdmSections* parent_elements);
    UdmElements(const std::string &section_name, UdmSize_t section_id);
    virtual ~UdmElements();

    // UdmElementType_t
    UdmElementType_t getElementType() const;
    void setElementType(UdmElementType_t elementType);

    // std::vector<UdmCell*>
    UdmSize_t getNumCells() const;
    UdmCell* getCell(UdmSize_t cell_id) const;
    UdmError_t clearCells();
    UdmError_t removeCell(const UdmCell* cell);
    UdmSize_t insertCellConnectivity(UdmElementType_t elem_type, UdmSize_t *node_ids);
    UdmSize_t setCellsConnectivity(UdmElementType_t elem_type, UdmSize_t cell_size, UdmSize_t *elements);
    UdmError_t removeCells(const std::vector<UdmCell*>& remove_cells);

    // UdmZone
    UdmSections* getParentSections();
    const UdmSections* getParentSections() const;
    UdmError_t setParentSections(UdmSections *elements);

    // CGNS
    UdmError_t readCgns(int index_file, int index_base, int index_zone, int index_section);
    UdmError_t writeCgns(int index_file, int index_base, int index_zone, int index_cell);
    UdmError_t writeCgnsLinkFile(int index_file, int index_base, int index_zone, const std::string &link_output_path, const std::vector<std::string> &linked_files);
    UdmError_t initializeWriteCgns();
    UdmError_t joinCgnsElements(UdmElements *dest_elements);

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);
    UdmError_t importCell(UdmCell* cell, const std::vector<UdmNode*>& import_nodes);

    // 要素（セル）生成
    template<class DATA_TYPE>
    UdmCell* factoryCell(UdmElementType_t elem_type, DATA_TYPE *elements);
    UdmSize_t insertCell(UdmCell* cell);

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;

private:
    void initialize();

    // std::vector<UdmCell*>
    bool existsCellByElemId(UdmSize_t elem_id) const;
    UdmCell* getCellByElemId(UdmSize_t elem_id);
    const UdmCell* getCellByElemId(UdmSize_t elem_id) const;
    UdmSize_t setCellByElemId(UdmSize_t elem_id, UdmCell* cell);

    // CGNS
    UdmElementType_t setElementType(ElementType_t cgns_elementtype);

    UdmZone* getZone() const;
    UdmGridCoordinates* getGridCoordinates() const;
    UdmError_t cloneElements(const UdmElements& src);
    bool equalsElements(const UdmElements &elements) const;
};

} /* namespace udm */

#endif /* _UDMSECTION_H_ */
