/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

#ifndef _UDMELEMENTS_H_
#define _UDMELEMENTS_H_

/**
 * @file UdmSections.h
 * 要素管理クラスのヘッダーファイル
 */

#include "model/UdmEntityVoxels.h"

namespace udm
{

class UdmZone;
class UdmElements;
class UdmCell;
class UdmFlowSolutions;
class UdmComponent;

/**
 * 要素管理クラス.
 * セクション（要素構成）情報の管理、要素全体の管理を行う.
 */
class UdmSections: public UdmEntityVoxels
{
friend class UdmElements;

private:

    /**
     * セクション（要素構成）情報リスト
     */
    std::vector<UdmElements*> section_list;

    /**
     * すべての実体要素リスト.
     * キーはCGNS要素ID（CGNSファイル内定義のCGNS:Sctionの要素ID範囲）
     * 連続したCGNS要素IDで設定されていれば、キー=格納IDとなる。
     */
    std::vector<UdmCell*> entity_cells;

    /**
     * 仮想要素リスト
     */
    std::vector<UdmCell*>  virtual_cells;

    /**
     * 要素の部品要素リスト
     */
    std::vector<UdmComponent*> component_cells;

    /**
     * ゾーン内の最小座標
     */
    //UdmCoordsValue min_coords;

    /**
     * ゾーン内の最大座標
     */
    //UdmCoordsValue max_coords;

    UdmZone *parent_zone;            ///< 親ゾーン

public:
    UdmSections();
    UdmSections(UdmZone* zone);
    virtual ~UdmSections();

    // UdmElements
    UdmElements * createSection(const std::string &section_name, UdmElementType_t element_type);
    UdmElements * createSection(UdmElementType_t element_type);
    int getNumSections() const;
    bool existsSection(const std::string &section_name) const;
    UdmElements * getSection(int section_id);
    const UdmElements * getSection(int section_id) const;
    UdmElements * getSection(const std::string &section_name);
    const UdmElements * getSection(const std::string &section_name) const;
    int setSection(UdmElements *section);
    UdmError_t removeSection(int section_id);
    UdmError_t removeSection(const std::string &section_name);
    UdmError_t clearSections();

    // UdmCell entity_array
    UdmSize_t getNumEntityCells() const;
    UdmCell* getEntityCell(UdmSize_t cell_id) const;
    UdmCell* getEntityCellByElemId(UdmSize_t elem_id);
    const UdmCell* getEntityCellByElemId(UdmSize_t elem_id) const;
    UdmError_t removeEntityCell(UdmSize_t cell_id);
    UdmError_t clearEntityCells();
    UdmSize_t getCellId(UdmSize_t elem_id);
    UdmSize_t setEntityCellByElemId(const std::string &section_name, UdmSize_t elem_id, UdmCell* cell);
    UdmSize_t setEntityCellByElemId(UdmSize_t section_id, UdmSize_t elem_id, UdmCell* cell);
    UdmSize_t getNumEntities() const;
    template <class VALUE_TYPE> UdmError_t initializeValueEntities(const std::string &solution_name, VALUE_TYPE value);
    UdmCell* getCellByLocalId(UdmSize_t cell_id) const;

    // UdmCell virtual_cells
    UdmSize_t getNumVirtualCells() const;
    UdmCell* getVirtualCell(UdmSize_t virtual_cell_id) const;
    UdmSize_t insertVirtualCell(UdmCell* cell);
    UdmError_t removeVirtualCell(UdmSize_t virtual_cell_id);
    UdmError_t clearVirtualCells();

    // 部品要素
    UdmError_t createComponentCells();
    UdmSize_t getNumComponentCells() const;
    UdmComponent* getComponentCell(UdmSize_t component_cell_id) const;
    UdmSize_t insertComponentCell(UdmComponent* component_cell);
    UdmError_t removeComponentCell(UdmSize_t component_cell_id);
    UdmError_t clearComponentCells();
    UdmComponent* findComponentCell(const UdmComponent* component_cell);

    // 要素共通
    UdmError_t removeCell(const UdmCell* cell);
    UdmError_t removeCells(const std::vector<UdmCell*>& export_cells);
    UdmError_t removeComponentCell(const UdmComponent* component);

    // UdmZone
    UdmZone* getParentZone() const;
    UdmError_t setParentZone(UdmZone *zone);

    // CGNS
    UdmError_t readCgns(int index_file, int index_base, int index_zone);
    UdmError_t writeCgns(int index_file, int index_base, int index_zone, int timeslice_step);
    UdmError_t writeCgnsLinkFile(int index_file, int index_base, int index_zone, const std::string &link_output_path, const std::vector<std::string> &linked_files, int timeslice_step);
    UdmError_t initializeWriteCgns();
    UdmError_t writeVirtualCells(int index_file, int index_base, int index_zone);
    UdmError_t joinCgnsSections(UdmSections *dest_sections);

    // シリアライズ
    UdmSerializeArchive& serialize(UdmSerializeArchive &archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive &archive);
    UdmError_t brodcastSections();
    UdmError_t importCells(const std::vector<UdmCell*>& import_cells, const std::vector<UdmNode*>& import_nodes);
    UdmSize_t getMaxEntityElemId() const;
    UdmError_t rebuildCellId();
    UdmError_t rebuildVirtualCells();
    UdmError_t importVirtualCells(const std::vector<UdmCell*>& import_virtuals, const std::vector<UdmNode*>& virtual_nodes);

    // FlowSoution
    // 要素（セル）
    template <class VALUE_TYPE>
    UdmError_t getSolutionScalar(UdmSize_t cell_id, const std::string &solution_name, VALUE_TYPE &value) const;
    template <class VALUE_TYPE>
    unsigned int getSolutionVector(UdmSize_t cell_id, const std::string &solution_name, VALUE_TYPE *value)  const;
    template <class VALUE_TYPE>
    UdmError_t setSolutionScalar(UdmSize_t cell_id, const std::string &solution_name, VALUE_TYPE value);
    template <class VALUE_TYPE>
    UdmError_t setSolutionVector(UdmSize_t cell_id, const std::string &solution_name, const VALUE_TYPE* values, unsigned int size = 3);

    // 分割重み
    void clearPartitionWeight();

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;

private:
    void initialize();
    void finalize();
    UdmSize_t setEntityCell(UdmSize_t elem_id, UdmCell* cell);
    UdmSize_t insertEntityCell(UdmCell* cell);
    UdmError_t rebuildSections();
    UdmFlowSolutions* getFlowSolutions();
    UdmEntity* getEntityById(UdmSize_t entity_id);
    const UdmEntity* getEntityById(UdmSize_t entity_id) const;
    const UdmSolutionFieldConfig* getSolutionFieldConfig(const std::string &solution_name) const;
    UdmError_t cloneSections(const UdmSections& src);
    bool equalsSections(const UdmSections &sections) const;

};


} /* namespace udm */

#endif /* _UDMELEMENTS_H_ */
