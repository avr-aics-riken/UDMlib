/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmModel.cpp
 */

#include "model/UdmModel.h"

#ifdef isnan
  #define ISNAN(_X) isnan(_X)
#else
  #define ISNAN(_X) std::isnan(_X)
#endif

namespace udm
{

/**
 * コンストラクタ
 */
UdmModel::UdmModel() : UdmGeneral(), iterator(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ : ベース名称, ベースID
 * @param base_name        ベース名称
 * @param base_id        ベースID
 */
UdmModel::UdmModel(const std::string& base_name, UdmSize_t base_id): UdmGeneral(base_name, base_id), iterator(NULL)
{
    this->initialize();
}

/**
 * コンストラクタ : DFIファイル
 * @param dfi_filename        index.dfiファイル名
 */
UdmModel::UdmModel(const char* dfi_filename) : UdmGeneral(), iterator(NULL)
{
    this->initialize();
    // index.dfiファイルを読込
    this->readDfi(dfi_filename);
}

/**
 * コンストラクタ
 * @param comm            MPIコミュニケータ
 */
UdmModel::UdmModel(const MPI_Comm& comm) : UdmGeneral(), iterator(NULL)
{
    // 初期化を行う。
    this->initialize();
    this->setMpiComm(comm);
}


/**
 * デストラクタ
 */
UdmModel::~UdmModel()
{
    // ゾーンの削除
    this->clearZone();
    // 時系列データの削除
    this->clearIterativeData();
    // ロードバランスクラスの削除
    if (this->partition != NULL) delete this->partition;
    if (this->config != NULL) delete this->config;
}

/**
 * 初期化を行う.
 */
void UdmModel::initialize()
{
    this->setId(1);
    this->setName(UDM_CGNS_NAME_BASE);
    this->simulation_type = Udm_TimeAccurate;
    cell_dimension = 3;
    physical_dimension = 3;
    // ゾーンの削除
    this->clearZone();
    // 時系列データの削除
    this->clearIterativeData();
    // DFI設定情報
    this->config = new UdmDfiConfig();

    // MPI
    this->setMpiComm(MPI_COMM_WORLD);

    // UdmInfo
    this->udminfo_version.clear();
    this->udminfo_process_size = 0;
    this->udminfo_rankno = -1;

    // ロードバランスクラス
    this->partition = new UdmLoadBalance();
}

/**
 * ゾーン数を取得する.
 * @return        ゾーン数
 */
int UdmModel::getNumZones() const
{
    return this->zones.size();
}

/**
 * ゾーンを取得する.
 * @param zone_name        ゾーン名称
 * @return        ゾーン
 */
UdmZone* UdmModel::getZone(const std::string& zone_name) const
{
    std::vector<UdmZone*>::const_iterator itr;
    for (itr = this->zones.begin(); itr != this->zones.end(); itr++) {
        if ((*itr)->equalsName(zone_name)) {
            return (*itr);
        }
    }

    return NULL;
}


/**
 * ゾーンを取得する.
 * @param zone_id        ゾーンID（１～）
 * @return        ゾーン
 */
UdmZone* UdmModel::getZone(int zone_id) const
{
    if (zone_id <= 0) return NULL;
    if (zone_id > this->zones.size()) return NULL;
    return this->zones[zone_id-1];
}


/**
 * ゾーンを生成して、ゾーンをモデルに追加する.
 * 既定の命名規約のゾーン名称にてゾーンを作成する.
 * @return        生成ゾーン、ゾーンの生成に失敗した場合はNULLを返す.
 */
UdmZone* UdmModel::createZone()
{
    char zone_name[33] = {0x00};
    sprintf(zone_name, UDM_CGNS_NAME_ZONE, this->getNumZones()+1);
    if (this->existsZone(zone_name)) return NULL;
    /// ゾーン名称、ゾーンIDでゾーンの生成を行う.
    UdmZone* zone = new UdmZone(zone_name, this->getNumZones()+1);

    this->insertZone(zone);
    return zone;
}


/**
 * ゾーンを生成して、ゾーンをモデルに追加する.
 * 既存のゾーン名称は作成できない。
 * @param zone_name        ゾーン名称
 * @return        生成ゾーン、ゾーンの生成に失敗した場合はNULLを返す.
 */
UdmZone* UdmModel::createZone(const std::string &zone_name)
{
    if (this->existsZone(zone_name)) return NULL;
    /// ゾーン名称、ゾーンIDでゾーンの生成を行う.
    UdmZone* zone = new UdmZone(zone_name, this->zones.size()+1);

    this->insertZone(zone);
    return zone;
}


/**
 * ゾーンを追加する.
 * ゾーンオブジェクトは上位が作成したオブジェクトポインタを使用する.（上位でゾーンオブジェクトを削除しないこと）
 * @param zone        ゾーン
 * @return        設定ゾーンID （0の場合は挿入エラー）
 */
int UdmModel::insertZone(UdmZone* zone)
{
    if (zone == NULL) return 0;
    std::string zone_name = zone->getName();
    if (this->getZone(zone_name) != NULL) {
        return 0;
    }

    this->zones.push_back(zone);
    zone->setId(this->zones.size());
    zone->setParentModel(this);

    return this->zones.size();
}

/**
 * ゾーンを削除する.
 * @param zone_name        削除ゾーン名称
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::removeZone(const std::string& zone_name)
{
    UdmZone *zone = this->getZone(zone_name);
    if (zone == NULL) return UDM_ERROR;

    return this->removeZone(zone->getId());
}

/**
 * ゾーンを削除する.
 * @param zone_id        削除ゾーンID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::removeZone(int zone_id)
{
    if (zone_id <= 0) return UDM_ERROR;
    if (zone_id > this->zones.size()) return UDM_ERROR;
    if (this->zones[zone_id-1] != NULL) delete this->zones[zone_id-1];
    std::vector<UdmZone*>::iterator itr = this->zones.begin();
    std::advance( itr, zone_id-1 );            // イテレータの移動
    this->zones.erase(itr);

    // ゾーンIDの再振分
    UdmSize_t new_id = 0;
    for (itr = this->zones.begin(); itr != this->zones.end(); itr++) {
        if ((*itr) != NULL) (*itr)->setId(++new_id);
    }
    return UDM_OK;
}
/**
 * ゾーンをすべて削除する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::clearZone()
{
/*******************
    std::vector<UdmZone*>::reverse_iterator ritr = this->zones.rbegin();
    while (ritr != this->zones.rend()) {
        UdmZone* zone = *(--(ritr.base()));
        if (zone != NULL) {
            delete zone;
        }
        this->zones.erase((++ritr).base());
        ritr = this->zones.rbegin();
    }
*******************/

    std::vector<UdmZone*>::iterator itr = this->zones.begin();
    for (itr=this->zones.begin();itr!=this->zones.end(); itr++) {
        UdmZone* zone = (*itr);
        if (zone != NULL) {
            delete zone;
        }
    }
    this->zones.clear();
    return UDM_OK;
}

/**
 * ゾーン名称が存在するかチェックする.
 * @param zone_name        ゾーン名称
 * @return        true=ゾーンは存在する
 */
bool UdmModel::existsZone(const std::string &zone_name) const
{
    return (this->getZone(zone_name) != NULL);
}

/**
 * ゾーン名称からゾーンIDを取得する.
 * @param zone_name        ゾーン名称
 * @return        ゾーンID : ゾーン名称が存在しない場合は0を返す。
 */
int UdmModel::getZoneId(const std::string &zone_name) const
{
    std::vector<UdmZone*>::const_iterator itr;
    for (itr = this->zones.begin(); itr != this->zones.end(); itr++) {
        if ((*itr)->equalsName(zone_name)) {
            return std::distance(this->zones.begin(), itr) + 1;
        }
    }

    return 0;
}

/**
 * CGNSファイルを読みこむ.
 * @param cgns_filename        CGNSファイル名
 * @param timeslice_step       CGNS読込ステップ回数 (default=-1)
 * @param element_path         CGNS読込パス (default=NULL)
 * @return        エラー番号 : UDM_OK | UDM_ERROR, etc.
 */
UdmError_t UdmModel::readCgns(const char* cgns_filename, int timeslice_step, const char* element_path)
{
#ifdef _UDM_PROFILER
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;
    FILE*fp = NULL;
    if( !(fp=fopen(cgns_filename, "rb")) ) {
        return UDM_ERROR_HANDLER(UDM_ERROR_READ_CGNSFILE_OPENERROR, "Can't open file. (%s)\n", cgns_filename);
    }
    fclose(fp);

    int index_file,index_base,index_zone;
    int num_bases = 0, num_zones = 0;
    char basename[33] = {0x00}, zonename[33] = {0x00};
    int cell_dim, phys_dim;
    std::vector<std::string> element_paths;
    int num_paths = 0;
    char msg[128] = {0x00};
    int n;

    // CGNSファイル:open
    if (cg_open(cgns_filename, CG_MODE_READ, &index_file) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", cgns_filename, cg_get_error());
    }

    // CGNS読込パスを分解する.
    if (element_path != NULL) {
        num_paths = this->split(element_path, '/', element_paths);
    }

    // CGNS:Base
    if (cg_nbases(index_file, &num_bases) != CG_OK) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "index_file=%d", index_file);
    }
    if (num_bases == 0) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "num_bases is zero");
    }
    if (error != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(error);
    }
    // CGNS:Base一覧の取得
    error = UDM_OK;
    this->setId(0);
    for (index_base = 1; index_base<=num_bases; index_base++) {
        if (cg_base_read(index_file, index_base, basename, &cell_dim, &phys_dim)!= CG_OK) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "index_base=%d, cgns_error=%s", index_base, cg_get_error());
            break;
        }
        // シミュレーションタイプ
        SimulationType_t simulation_type;
        cg_simulation_type_read(index_file, index_base, &simulation_type);

        bool found_base = false;
        if (num_paths == 0) {
            found_base = true;
        }
        // CGNS読込ベース名
        if (num_paths > 0 && strcasecmp(element_paths[0].c_str(), basename) == 0) {
            found_base = true;
        }
        if (found_base) {
            // モデルの基本情報を設定する
            if (this->createModel(index_base,
                            std::string(basename),
                            cell_dim, phys_dim,
                            this->toSimulationType(simulation_type)) != UDM_OK) {
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "can not create model[index_base=%d,name=%s]", index_base, basename);
            }
            break;
        }
    }
    if (error == UDM_OK && this->getId() <= 0) {
        if (element_path != NULL) sprintf(msg, "not found element path[%s].", element_path);
        else sprintf(msg, "not found CGNS:Base.");
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, msg);
    }
    if (error != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(error);
    }

    index_base = this->getId();

    // UdmInfo
    this->readCgnsUdmInfo(index_file, index_base);
    int cgns_rankno = this->getCgnsRankno();
    int mpi_rankno = this->getMpiRankno();

    // CGNS:Zone
    if (cg_nzones(index_file, index_base, &num_zones) != CG_OK) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "index_base=%d, cgns_error=%s", index_base, cg_get_error());
    }
    if (num_zones == 0) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "num_zones is zero");
    }
    if (error != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(error);
    }

    // CGNS:Zone一覧の取得
    for (index_zone = 1; index_zone<=num_zones; index_zone++) {
        // ゾーンタイプ
        ZoneType_t zonetype;
        if (cg_zone_type(index_file, index_base, index_zone, &zonetype) != CG_OK) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "index_zone=%d", index_zone);
            break;
        }
        // 非構造格子のみサポートする.
        if (zonetype != Unstructured) {
            continue;
        }

        cgsize_t sizes[9] = {0x00};
        if (cg_zone_read(index_file, index_base, index_zone, zonename,  sizes) != CG_OK) {
            error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "index_zone=%d, cgns_error=%s", index_zone, cg_get_error());
            break;
        }

        bool found_zone = false;
        if (num_paths == 0) {
            found_zone = true;
        }
        // CGNS読込ゾーン名
        if (num_paths > 1 && strcasecmp(element_paths[1].c_str(), zonename) == 0) {
            found_zone = true;
        }

        if (found_zone) {
            UdmZone *zone = new UdmZone(this);
            if (zone->readCgns(index_file, index_base, index_zone, timeslice_step) != UDM_OK) {
                delete zone;
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "index_zone=%d", index_zone);
                break;
            }

            UdmZone *read_zone = this->getZone(zonename);
            if (read_zone == NULL) {
                // ゾーンの追加
                this->insertZone(zone);
                read_zone = zone;
            }
            else {
                // ゾーンの結合
                error = read_zone->joinCgnsZone(zone);
                // 結合ゾーンの破棄
                delete zone;
                if (error != UDM_OK) {
                    this->closeWriteCgnsFile(index_file);
                    return UDM_ERRORNO_HANDLER(error);
                }
            }
            break;
        }
    }

    // 部品要素（セル）を作成する
    if (this->createComponentCells() != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR, "failure:createComponentCells.");
    }

    if (error == UDM_OK && this->getNumZones() <= 0) {
        if (element_path != NULL) sprintf(msg, "not found element path[%s].", element_path);
        else sprintf(msg, "not found CGNS:Zone.");
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, msg);
    }
    if (error != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(error);
    }

    /* close CGNS file */
    this->closeWriteCgnsFile(index_file);

#ifdef _UDM_PROFILER
    UDM_STOPWATCH_STOP(__FUNCTION__);
#endif
    return UDM_OK;
}

/**
 * モデルの再構築を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::rebuildModel()
{
    UdmError_t error = UDM_OK;
    int n;

    // モデル基本情報をブロードキャストする.
    error = this->broadcastModel();
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

#ifdef _DEBUG_TRACE
    if (UDM_IS_DEBUG_LEVEL()) {
        UDM_DEBUG_PRINTF("%s:%d [%s]",  __FILE__, __LINE__, __FUNCTION__);
        std::string buf;
        this->toString(buf);
        UDM_DEBUG_PRINTF(buf.c_str());
    }
#endif

    // CGNS読込後処理
    int num_zones = this->getNumZones();
    for (n=1; n<=num_zones; n++) {
        UdmZone *zone = this->getZone(n);
        if (zone != NULL) {
            std::vector<UdmNode*> import_nodes;        // インポートノードはなし、すべて再構成対象
            zone->rebuildZone(import_nodes);
        }
    }

    // 入出力ランクテーブルをクリアする.
    this->config->clearIoRankTable();

    // 仮想要素（セル）を転送する.
    std::vector<UdmNode*> virtual_nodes;
    error = this->transferVirtualCells(virtual_nodes);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}

/**
 * 時系列データが設定されているかチェックする.
 * @return        true=時系列データが設定sされている.
 */
bool UdmModel::isSetIterativeData() const
{
    return (this->iterator != NULL);
}

/**
 * ステップ数,ステップ時間を取得する.
 * @param [out] step        ステップ数
 * @param [out] time        ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::getIterativeData(UdmSize_t& step, double& time) const
{
    if (this->iterator == NULL) {
        return UDM_ERROR;
    }
    step = this->iterator->getStep();
    time = this->iterator->getTime();

    return UDM_OK;
}

/**
 * ステップ数,ステップ時間,平均ステップ数,平均ステップ時間を取得する.
 * @param [out] step        ステップ数
 * @param [out] time        ステップ時間
 * @param [out] average_step    平均ステップ数
 * @param [out] average_time    平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::getIterativeData(
                        UdmSize_t& step, double& time,
                        UdmSize_t& average_step, double& average_time)
{
    if (this->iterator == NULL) {
        return UDM_ERROR;
    }
    step = this->iterator->getStep();
    time = this->iterator->getTime();
    average_step = this->iterator->getAverageStep();
    average_time = this->iterator->getAverageTime();

    return UDM_OK;
}


/**
 * ステップ数,ステップ時間を設定する.
 * @param step                ステップ数
 * @param time                ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::setIterativeData(UdmSize_t step, double time)
{
    if (this->iterator == NULL) {
        this->iterator = new UdmIterativeData();
    }
    this->iterator->setIterativeData(step, time);

    return UDM_OK;
}

/**
 * ステップ数,ステップ時間,平均ステップ数,平均ステップ時間を設定する.
 * @param step                ステップ数
 * @param time                ステップ時間
 * @param average_step        平均ステップ数
 * @param average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::setIterativeData(
                        UdmSize_t step, double time,
                        UdmSize_t average_step, double average_time)
{
    if (this->iterator == NULL) {
        this->iterator = new UdmIterativeData();
    }
    this->iterator->setIterativeData(step, time, average_step, average_time);

    return UDM_OK;
}

/**
 * 時系列データをクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::clearIterativeData()
{
    if (this->iterator != NULL) delete this->iterator;
    this->iterator = NULL;
    return UDM_OK;
}

/**
 * 要素（セル）次元数を取得する.
 * @return        要素（セル）次元数
 */
int UdmModel::getCellDimension() const
{
    return this->cell_dimension;
}

/**
 * 要素（セル）次元数を設定する.
 * @param cellDimension        要素（セル）次元数
 */
void UdmModel::setCellDimension(int cell_dimension)
{
    this->cell_dimension = cell_dimension;
}

/**
 * ノード次元数を取得する.
 * @return        ノード次元数
 */
int UdmModel::getPhysicalDimension() const
{
    return this->physical_dimension;
}

/**
 * ノード次元数を設定する.
 * @param physical_dimension        ノード次元数
 */
void UdmModel::setPhysicalDimension(int physical_dimension)
{
    this->physical_dimension = physical_dimension;
}


/**
 * クラス情報を文字列出力する:デバッグ用.
 * @param buf        出力文字列
 */
void UdmModel::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;

    // ベース名
    stream << "Base name = " << this->getName()  << std::endl;
    stream << "Base id = " << this->getId()  << std::endl;
    stream << "要素（セル）次元数 = " << this->getCellDimension()  << std::endl;
    stream << "ノード次元数 = " << this->getPhysicalDimension()  << std::endl;
    if (this->getSimulationType() == Udm_TimeAccurate) {
        stream << "シミュレーションタイプ = TimeAccurate" << std::endl;
    }
    else if (this->getSimulationType() == Udm_NonTimeAccurate) {
        stream << "シミュレーションタイプ = NonTimeAccurate" << std::endl;
    }
    else {
        stream << "シミュレーションタイプ = 不明" << std::endl;
    }
    stream << "UdmInfo:UDMLibraryVersion = " << this->getUdminfoVersion()  << std::endl;
    stream << "UdmInfo:ProcessSize = " << this->getUdminfoProcessSize()  << std::endl;
    stream << "UdmInfo:Rankno = " << this->getUdminfoRankno()  << std::endl;

    buf += stream.str();

    int num_zone = this->getNumZones();
    for (int n=1; n<=num_zone; n++) {
        this->getZone(n)->toString(buf);
    }
#endif

    return;
}

/**
 * シミュレーションタイプを取得する.
 * @return    シミュレーションタイプ
 */
UdmSimulationType_t UdmModel::getSimulationType() const
{
    return this->simulation_type;
}

/**
 * シミュレーションタイプを設定する.
 * @param simulation_type        シミュレーションタイプ
 */
void UdmModel::setSimulationType(UdmSimulationType_t simulation_type)
{
    this->simulation_type = simulation_type;
}

/**
 * シミュレーションタイプを設定する.
 * @param simulation_type        CGNS:シミュレーションタイプ
 */
UdmSimulationType_t UdmModel::setSimulationType(SimulationType_t simulation_type)
{
    if (simulation_type == TimeAccurate) this->simulation_type = Udm_TimeAccurate;
    else if (simulation_type == NonTimeAccurate) this->simulation_type = Udm_NonTimeAccurate;
    else this->simulation_type = Udm_SimulationTypeUnknown;
    return this->simulation_type;
}

/**
 * DFIファイルを読み込む
 * @param dfi_filename        index.dfiファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::readDfi(const char* dfi_filename)
{
    if (this->config != NULL) delete this->config;
    this->config = new UdmDfiConfig();

    /// index.dfiファイルを読み込む
    UdmError_t error = this->config->readDfi(dfi_filename);
    if (error != UDM_OK) {
        return UDM_ERROR_HANDLER(error, "can not read dfi file[%s]", dfi_filename);
    }

    return error;
}

/**
 * DFIファイルの設定情報に従って、CGNSファイルを読み込む.
 * @param dfi_filename        index.dfiファイル名
 * @param timeslice_step    読込ステップ番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::loadModel(const char* dfi_filename, int timeslice_step)
{
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;
    int n;

    // DFIファイルの読込
    error = this->readDfi(dfi_filename);
    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 読込CGNSファイル名
    bool mpiexec = false;
    int myrank = 0;
    int comm_size = 0;
    int flag;
    udm_mpi_initialized(&flag);
    if (!flag) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "not initialize MPI.");
    }
    if (this->mpi_communicator == MPI_COMM_NULL) {
        this->setMpiComm(MPI_COMM_WORLD);
    }
    mpiexec = true;
    myrank = this->getMpiRankno();
    std::vector<int> input_rank_ids;
    comm_size = this->getMpiProcessSize();        // 実行プロセス数

    // 読込プロセス番号の取得
    int num_ranks = this->config->makeIoRankTable(comm_size, myrank, input_rank_ids);
    if (num_ranks > 0) {
        // 読込CGNSパス
        std::string element_path;
        this->getConfigFileinfo()->getElementPath(element_path);

        // if (timeslice_step <= 0) timeslice_step = 0;
        std::vector<int>::const_iterator itr;
        for (itr=input_rank_ids.begin(); itr!=input_rank_ids.end(); itr++) {
            int rank_no = (*itr);
            std::string input_path;
            this->config->getCgnsInputFilePath(input_path, rank_no);

            // CGNSファイルの読込
            error = this->readCgns(input_path.c_str(), timeslice_step, element_path.c_str());
            if (error != UDM_OK) {
                UDM_ERRORNO_HANDLER(error);
                break;
            }
        }
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // モデルの再構築を行う。
    error = this->rebuildModel();
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
#endif
    return error;
}

/**
 * DFI設定を取得する.
 * @return        DFI設定を取得する.
 */
UdmDfiConfig* UdmModel::getDfiConfig()
{
    return this->config;
}


/**
 * DFI設定を取得する : const.
 * @return        DFI設定を取得する.
 */
const UdmDfiConfig* UdmModel::getDfiConfig() const
{
    return this->config;
}

/**
 * DFI:FileInfo設定を取得する.
 * @return        DFI:FileInfo設定を取得する.
 */
const UdmFileInfoConfig* UdmModel::getConfigFileinfo() const
{
    if (this->config == NULL) return NULL;
    return this->config->getFileinfoConfig();
}


/**
 * 時系列CGNSファイルの出力を行う:DFI設定情報.
 * DFI設定情報に従ってCGNSファイルを出力する.
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeTimeSlice(
                    int timeslice_step,
                    float timeslice_time)
{
    if (this->getDfiConfig() == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "dfi config is null.");
    }

    return UDM_OK;
}


/**
 * CGNS:GridCoordinatesとCGNS:FlowSolutionを時系列CGNSファイルに出力する.
 * 出力CGNSファイルが存在していれば、時系列データを追加する.
 * 存在していなければ、新規に作成する.
 * @param cgns_filename        出力CGNSファイル
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @param grid_timeslice        CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する.
 * @param write_constsolutions    固定物理量データの出力フラグ : true=固定物理量データを出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsTimeSliceFile(
                        const char* cgns_filename,
                        int timeslice_step,
                        float timeslice_time,
                        bool grid_timeslice,
                        bool write_constsolutions)
{
    int index_file,index_base;
    int n;

    // ゾーン出力前の初期化を行う。
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->initializeWriteCgns() != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : UdmZone::initializeWriteCgns(zoneid=%d)", n);
        }
    }

    // CGNS:Baseを出力する.
    if (this->writeCgnsBase(cgns_filename, index_file, index_base) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNS:Zoneを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeCgnsTimeSliceFile(index_file, index_base, timeslice_step, grid_timeslice, write_constsolutions) != UDM_OK) {
            this->closeWriteCgnsFile(index_file);
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // CGNS:BaseIterativeDataを出力する.
    if (this->writeCgnsIterativeDatas(index_file, index_base, timeslice_step, timeslice_time, grid_timeslice) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNSファイルをクローズする。
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}

/**
 * CGNS:GridCoordinatesを時系列CGNSファイルに出力する.
 * 出力CGNS:GridCoordinatesファイルが存在していれば、時系列データを追加する.
 * 存在していなければ、新規に作成する.
 * @param cgns_filename        出力CGNS:GridCoordinatesファイル
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @param grid_timeslice        CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsGridCoordinatesFile(
                            const char* cgns_filename,
                            int timeslice_step,
                            float timeslice_time,
                            bool grid_timeslice)
{
    int index_file,index_base;
    int n;

    // CGNS:Baseを出力する.
    if (this->writeCgnsBase(cgns_filename, index_file, index_base) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNS:Zoneを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeCgnsGridCoordinatesFile(index_file, index_base, timeslice_step, grid_timeslice) != UDM_OK) {
            this->closeWriteCgnsFile(index_file);
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // CGNS:BaseIterativeDataを出力する.
    if (this->writeCgnsIterativeDatas(index_file, index_base, timeslice_step, timeslice_time, grid_timeslice) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNSファイルをクローズする。
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}

/**
 * CGNS:FlowSolutionを時系列CGNSファイルに出力する.
 * 出力CGNS:FlowSolutionファイルが存在していれば、時系列データを追加する.
 * 存在していなければ、新規に作成する.
 * @param cgns_filename        出力CGNS:FlowSolutionファイル
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @param write_constsolutions    固定物理量データの出力フラグ : true=固定物理量データを出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsFlowSolutionFile(
                const char* cgns_filename,
                int timeslice_step,
                float timeslice_time,
                bool write_constsolutions)
{

    int index_file,index_base;
    int n;

    // CGNS:Baseを出力する.
    if (this->writeCgnsBase(cgns_filename, index_file, index_base) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNS:Zoneを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeCgnsFlowSolutionFile(index_file, index_base, timeslice_step, write_constsolutions) != UDM_OK) {
            this->closeWriteCgnsFile(index_file);
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // CGNS:BaseIterativeDataを出力する.
    if (this->writeCgnsIterativeDatas(index_file, index_base, timeslice_step, timeslice_time, false) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNSファイルをクローズする。
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}


/**
 * CGNSリンクファイルの出力を行う:出力CGNSリンクファイル.
 * @param cgns_filename        出力CGNSリンクファイル
 * @param linked_files            リンクファイル
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @param grid_timeslice        CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsLinkFile(
                        const char* cgns_filename,
                        const std::vector<std::string> &linked_files,
                        int timeslice_step,
                        float timeslice_time,
                        bool grid_timeslice)
{
    int index_file,index_base;
    int n;

    // CGNS:Baseを出力する.
    if (this->writeCgnsBase(cgns_filename, index_file, index_base) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNS:Zone配下にリンクを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeCgnsLinkFile(index_file, index_base, cgns_filename, linked_files, timeslice_step) != UDM_OK) {
            this->closeWriteCgnsFile(index_file);
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // 再度、一度閉じて、開き直さないと、書込ノードが取得できない。
    this->closeWriteCgnsFile(index_file);
    // CGNSファイル:open
    if (cg_open(cgns_filename, CG_MODE_MODIFY, &index_file) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", cgns_filename, cg_get_error());
    }

    // CGNS:BaseIterativeDataを出力する.
    if (this->writeCgnsIterativeDatas(index_file, index_base, timeslice_step, timeslice_time, grid_timeslice) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNSファイルをクローズする。
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}

/**
 * CGNS:BASEを出力する.
 * @param cgns_filename        CGNS出力ファイル名
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsBase(const char* cgns_filename, int &index_file, int &index_base)
{
    int open_mode = CG_MODE_WRITE;        // modes for cgns file
    FILE*fp = NULL;
    char basename[33] = {0x00};
    int cell_dim, phys_dim;
    int num_bases = 0;
    char dirname[256] = {0x00};

    if (cgns_filename == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "filename is null.");
    }
    // 出力ディレクトリを作成する.
    udm_get_dirname(cgns_filename, dirname);
    if (strlen(dirname) > 0) {
        if (udm_make_directories(dirname) != 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", dirname);
        }
    }

    if( !(fp=fopen(cgns_filename, "rb")) ) {
        // CG_MODE_WRITEモードでcg_open, cg_closeを行い、ファイル初期化する.
        open_mode = CG_MODE_WRITE;        // 新規作成
        // CGNSファイル:open
        if (cg_open(cgns_filename, open_mode, &index_file) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", cgns_filename, cg_get_error());
        }
        this->closeWriteCgnsFile(index_file);
        open_mode = CG_MODE_MODIFY;        // 追加
    }
    else {
        open_mode = CG_MODE_MODIFY;        // 追加
        fclose(fp);
    }

    // CGNSファイル:open
    if (cg_open(cgns_filename, open_mode, &index_file) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", cgns_filename, cg_get_error());
    }

    index_base = 0;
    cg_nbases(index_file, &num_bases);
    for (index_base=1; index_base<=num_bases; index_base++) {
        if (cg_base_read(index_file, index_base, basename, &cell_dim, &phys_dim) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : cg_base_read.");
        }
        if (strcmp(basename, UDM_CGNS_NAME_BASE) == 0) {
            break;
        }
    }

    if (index_base == 0 || index_base>num_bases) {
        // CGNS:BASEノードの作成
        strcpy(basename, UDM_CGNS_NAME_BASE);
        if (cg_base_write(index_file, basename, this->cell_dimension,this->physical_dimension, &index_base) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "filename=%s", cgns_filename);
        }
    }

    // UdmInfoの出力
    if (this->writeCgnsUdmInfo(index_file, index_base) != UDM_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "index_file=%d,index_base=%d", index_file, index_base);
    }

    return UDM_OK;
}


/**
 * CGNS:BaseIterativeDataを出力する.
 * BaseIterativeDataが存在していなければ作成する。
 * 存在していれば、既存時系列に追加を行う.
 * @param index_file            CGNSファイルインデックス
 * @param index_base            CGNSベースインデックス
 * @param timeslice_step        時系列ステップ番号
 * @param timeslice_time        時系列ステップ時間
 * @param write_gridflag       CGNS:GridCoordinatesの時系列出力の有無:true=GridCoordinates時系列出力する
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsIterativeDatas(
                int index_file,
                int index_base,
                int timeslice_step,
                float timeslice_time,
                bool write_gridflag)
{
    int steps;
    std::vector<int> timeslice_steps;
    std::vector<float> timeslice_times;
    cgsize_t nuse;
    float *timeValues = NULL;
    int *iterationValues = NULL;
    int n;
    UdmError_t error = UDM_OK;;
    int count = 0;

    // CGNS:ZoneIterativeDatasを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeCgnsIterativeDatas(index_file, index_base, timeslice_step) != UDM_OK) {
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // シミュレーションタイプの書込 : cg_simulation_type_write
    cg_simulation_type_write(index_file, index_base, TimeAccurate);

    // CGNS:BaseIterativeDataのデータの取得
    steps = this->readCgnsBaseIterativeDatas(index_file, index_base, timeslice_steps, timeslice_times);
    if (steps == 0) {
        timeslice_steps.clear();
        timeslice_times.clear();
    }

    // 時系列ステップ時間のチェック
    if (ISNAN(timeslice_time) && timeslice_times.size() > 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "invalid timeslice_time.");
    }

    // 時系列ステップ番号を削除する.
    timeslice_steps.erase(std::remove(
            timeslice_steps.begin(), timeslice_steps.end(), timeslice_step), timeslice_steps.end());

    // 時系列ステップ時間を削除する.
    if (!ISNAN(timeslice_time)) {
        timeslice_times.erase(std::remove(
                timeslice_times.begin(), timeslice_times.end(), timeslice_time), timeslice_times.end());
    }

    // 出力CGNS:FlowSolution分のステップ数を追加する.
    count = this->getNumCgnsIterativeDatas();
    if (count <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "invalid iterativedata count.");
    }

    // 時系列ステップ番号、時間を追加する.
    for (n=0; n<count; n++) {
        timeslice_steps.push_back(timeslice_step);
        if (!ISNAN(timeslice_time)) {
            timeslice_times.push_back(timeslice_time);
        }
    }
    // ソートを行う
    std::sort(timeslice_steps.begin(),timeslice_steps.end());
    std::sort(timeslice_times.begin(),timeslice_times.end());

    if ( timeslice_times.size() > 0 && (timeslice_steps.size() != timeslice_times.size()) ) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "not equal size timeslice_steps and timeslice_times.");
    }
    steps = timeslice_steps.size();

    if (timeslice_steps.size() > 0) {
        iterationValues = new int[steps];
        for (n=0; n<steps; n++) iterationValues[n] = timeslice_steps[n];
    }
    if (timeslice_times.size() > 0) {
        timeValues = new float[steps];
        for (n=0; n<steps; n++) timeValues[n] = timeslice_times[n];
    }

    // CGNS:ベース時系列データの書込
    error = UDM_OK;
    if (cg_biter_write(index_file, index_base, "BaseIterativeData", steps) != CG_OK) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : cg_biter_write.");
    }
    if (cg_goto(index_file,index_base,"BaseIterativeData_t",1,"end") != CG_OK) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : cg_goto(BaseIterativeData_t).");
    }
    if (error == UDM_OK) {
        nuse = steps;
        if (timeValues != NULL) {
            if (cg_array_write("TimeValues", RealSingle, 1, &nuse, timeValues) != CG_OK) {
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : cg_array_write(TimeValues).");
            }
        }
        if (iterationValues != NULL) {
            if (cg_array_write("IterationValues", Integer, 1, &nuse, iterationValues) != CG_OK) {
                error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : cg_array_write(IterationValues).");
            }
        }
    }

    if (timeValues != NULL) delete []timeValues;
    if (iterationValues != NULL) delete []iterationValues;

    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}


/**
 * CGNS:BaseIterativeDataから時系列ステップ数と時間を取得する.
 * @param [in] index_file        CGNSファイルインデックス
 * @param [in] index_base        CGNSベースインデックス
 * @param [out] timeslice_steps    CGNS:BaseIterativeData/IterationValues:時系列ステップ数.
 * @param [out] timeslice_steps    CGNS:BaseIterativeData/TimeValues:時系列ステップ時間
 * @return      時系列ステップ数
 */
int UdmModel::readCgnsBaseIterativeDatas(
                int index_file,
                int index_base,
                std::vector<int> &timeslice_steps,
                std::vector<float> &timeslice_times)
{
    char bitername[33], arrayname[33];
    int nsteps, narrays;
    int dataDimension;
    cgsize_t n, i, dimensionVector[3];
    DataType_t idatatype;

    // CGNS:BaseIterativeDataの読込 : cg_biter_read
    if (cg_biter_read(index_file,index_base,bitername,&nsteps) != CG_OK) {
        return 0;
    }
    if (nsteps <= 0) {
        return 0;
    }
    if (cg_goto(index_file,index_base,"BaseIterativeData_t",1,"end") != CG_OK) {
        return 0;
    }
    // CGNS:BaseIterativeData/DataArray_tの数の取得 : cg_narrays
    narrays = 0;
    cg_narrays(&narrays);
    for (n=1; n<=narrays; n++) {
        // CGNS:BaseIterativeData/DataArray_tの名前,データ数の取得 : cg_array_info
        cg_array_info(n, arrayname, &idatatype,&dataDimension, dimensionVector);
        if (strcmp(arrayname, "IterationValues") == 0) {
            if (dataDimension != 1 && dimensionVector[0] != nsteps) {
                UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid DataDimension or DimensionVector.");
                return 0;
            }
            // int iterationValues[dimensionVector[0]];
            int *iterationValues = new int[dimensionVector[0]];
            // IterationValuesの繰返回数の取得 : cg_array_read_as
            cg_array_read_as(n, Integer, iterationValues);
            for (i=0; i<dimensionVector[0]; i++) {
                timeslice_steps.push_back(iterationValues[i]);
            }
            delete []iterationValues;
        }
        if (strcmp(arrayname, "TimeValues") == 0) {
            if (dataDimension != 1 && dimensionVector[0] != nsteps) {
                UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ITERATIVEDATA, "invalid DataDimension or DimensionVector.");
                return 0;
            }
            // float timeValues[dimensionVector[0]];
            float *timeValues = new float[dimensionVector[0]];
            // IterationValuesの繰返回数の取得 : cg_array_read_as
            cg_array_read_as(n, RealSingle, timeValues);
            for (i=0; i<dimensionVector[0]; i++) {
                timeslice_times.push_back(timeValues[i]);
            }
            delete []timeValues;
        }

    }
    return nsteps;
}

/**
 * 出力ステップ回数を取得する.
 * @return        出力ステップ回数
 */
int UdmModel::getNumCgnsIterativeDatas() const
{
    int n;
    int max_count = 0;
    // CGNS:Zoneを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        const UdmZone* zone = this->getZone(n);
        int count = zone->getNumCgnsIterativeDatas();
        if (max_count < count) {
            max_count = count;
        }
    }

    return max_count;
}

/**
 * MPIコミュニケータを取得する.
 * @return        MPIコミュニケータ
 */
MPI_Comm UdmModel::getMpiComm() const
{
    return this->mpi_communicator;
}

/**
 * MPIコミュニケータを設定する.
 * @param comm        MPIコミュニケータ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::setMpiComm(const MPI_Comm& comm)
{
    int rankno = -1, num_procs = 0;
    int mpi_flag;

    this->mpi_communicator = comm;
    this->mpi_rankno = rankno;
    this->mpi_num_process = num_procs;
    if (this->mpi_communicator == MPI_COMM_NULL) {
        return UDM_ERROR;
    }

    // MPI初期化済みであるかチェックする.
    udm_mpi_initialized(&mpi_flag);
    if (!mpi_flag) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Please execute MPI_Init beforehand.");
    }
    udm_mpi_comm_rank(this->mpi_communicator, &rankno);
    udm_mpi_comm_size(this->mpi_communicator, &num_procs);
    if (num_procs >= 0 && rankno >= 0 && rankno < num_procs) {
        this->mpi_rankno = rankno;
        this->mpi_num_process = num_procs;
        return UDM_OK;
    }

    // 分割クラスにMPIコミュニケータを設定する.
    this->getLoadBalance()->setMpiComm(comm);

    return UDM_ERROR;
}

/**
 * MPIランク番号を取得する.
 * @return        MPIランク番号
 */
int UdmModel::getMpiRankno() const
{
    return this->mpi_rankno;
}

/**
 * MPIランク番号を設定する.
 * @param rankno        MPIランク番号
 */
void UdmModel::setMpiRankno(int rankno)
{
    this->mpi_rankno = rankno;
}

/**
 * MPIプロセス数を取得する.
 * @return        MPIプロセス数
 */
int UdmModel::getMpiProcessSize() const
{
    return this->mpi_num_process;
}

/**
 * MPIプロセス数を設定する.
 * @param proc_size        MPIプロセス数
 */
void UdmModel::setMpiProcessSize(int proc_size)
{
    this->mpi_num_process = proc_size;
}

/**
 * モデルの基本情報を全プロセスに送信する.
 * CGNSを読み込んでいない場合は、モデル、ゾーンを作成する.
 * すべてのプロセスにて同一モデル、ゾーンが存在するかチェックする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::broadcastModel()
{
    int n;
    int mpi_flag;
    UdmError_t error = UDM_OK;

    // MPI初期化済みであるかチェックする.
    udm_mpi_initialized(&mpi_flag);
    if (!mpi_flag) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "Please execute MPI_Init beforehand.");
    }
    if (!(this->mpi_rankno >= 0 && this->mpi_num_process >= 1)) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_MPI, "invalid mpi process [mpi_rankno=%d,mpi_num_process=%d].", this->mpi_rankno, this->mpi_num_process);
    }

    UdmSerializeBuffer streamBuffer;
    UdmSerializeArchive archive(&streamBuffer);

    int  buf_size = 0;
    char* buf = NULL;
    if (this->mpi_rankno == 0) {
        // シリアライズを行う:バッファーサイズ取得
        archive << *this;
        buf_size = archive.getOverflowSize();
        if (buf_size > 0) {
            // バッファー作成
            buf = new char[buf_size];
            streamBuffer.initialize(buf, buf_size);
            // シリアライズを行う
            archive << *this;
        }
    }

    // バッファーサイズ送信
    udm_mpi_bcast(&buf_size, 1, MPI_INT, 0, this->getMpiComm());
    if (buf_size <= 0) {
        return UDM_ERROR_HANDLER(UDM_ERROR_SERIALIZE, "buffer size is zero.");
    }

    if (buf == NULL) {
        // バッファー作成
        buf = new char[buf_size];
    }

    // UdmModelシリアライズバッファー送信
    udm_mpi_bcast(buf, buf_size, MPI_CHAR, 0, this->getMpiComm());

    // デシリアライズ
    UdmModel mpi_model;
    streamBuffer.initialize(buf, buf_size);
    archive >> mpi_model;
    if (buf != NULL) {
        delete[] buf;
        buf = NULL;
    }

    // モデルの作成済みであるかチェックする.
    if (this->getId() == 0) {
        // モデル未作成であるので、モデルを作成する.
        error = this->createModel(mpi_model.getId(),
                        mpi_model.getName(),
                        mpi_model.getCellDimension(),
                        mpi_model.getPhysicalDimension(),
                        mpi_model.getSimulationType());
        if (error != UDM_OK) {
            UDM_ERRORNO_HANDLER(error);
        }
    }
    // モデルが存在するので、モデルが同一であるかチェックする.
    else if (!this->equalsModel(mpi_model)) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "not equals model(CGNS:BASE).");
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // ゾーンを作成する.
    int num_zone = 0;
    if (this->mpi_rankno == 0) {
        // ゾーン数
        num_zone = this->getNumZones();
    }

    // ゾーン数送信
    udm_mpi_bcast(&num_zone, 1, MPI_INT, 0, this->mpi_communicator);
    if (num_zone <= 0) {
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "zone size is zero.");
    }
    else if (this->getNumZones() > 0 && this->getNumZones() != (UdmSize_t)num_zone) {
        // ゾーン数が等しくない
        error = UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE, "not equals zone size.[root_rank=%d,my_rank=%d]", num_zone, this->getNumZones());
    }
    else if (this->getNumZones() == 0) {
        // ゾーンを作成する.
        for (n=1; n<=num_zone; n++) {
            // ID,ゾーン名はbroadcastZoneでセットする.
            UdmZone* zone = new UdmZone();
            this->insertZone(zone);
            zone->setId(0);            // ゾーンIDは０にするにより未作成ゾーンとする.
        }
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    // ゾーンの転送、作成、チェックを行う.
    for (n=1; n<=num_zone; n++) {
        error = this->getZone(n)->broadcastZone();
        // ACK
        if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
            UDM_ERROR_HANDLER(error, "failure : broadcastZone[zone_id=%d]", n);
            break;
        }
    }

    // ACK
    if (udm_mpi_ack(&error, this->getMpiComm()) != UDM_OK) {
        return UDM_ERRORNO_HANDLER(UDM_ERROR_MPI_ACK);
    }

    return UDM_OK;
}

/**
 * モデルの基本情報のシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmModel::serialize(UdmSerializeArchive& archive) const
{
    // UdmGeneral
    // ID : CGNSベースID
    // CGNSノードのデータ型
    // CGNSベース名
    this->serializeGeneralBase(archive, this->getId(), this->getDataType(), this->getName());

    // 要素（セル）次元数
    archive << this->cell_dimension;
    // ノードの次元数
    archive << this->physical_dimension;
    // シミュレーションタイプ
    archive << this->simulation_type;
    // ゾーン数
    int num_zone = this->getNumZones();
    archive << num_zone;

    return archive;
}


/**
 * モデルの基本情報のデシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmModel::deserialize(UdmSerializeArchive& archive)
{
    UdmSize_t id;
    UdmDataType_t data_type;
    std::string name;

    // UdmGeneral
    // ID : CGNSベースID
    // CGNSノードのデータ型
    // CGNSベース名
    this->deserializeGeneralBase(archive, id, data_type, name);
    // ID : CGNSベースID
    this->setId(id);
    // CGNSノードのデータ型
    this->setDataType(data_type);
    // CGNSベース名
    this->setName(name);

    // 要素（セル）次元数
    archive >> this->cell_dimension;
    // ノードの次元数
    archive >> this->physical_dimension;
    // シミュレーションタイプ
    archive.read(this->simulation_type, sizeof(UdmSimulationType_t));

    // ゾーン数
    int num_zone = 0;
    archive >> num_zone;

    return archive;
}

/**
 * 同一モデルであるかチェックする.
 * ID,name,次元数,シミュレーションタイプ等の基本情報が同じであるかチェックする.
 * @param model        チェック対象モデル
 * @return            true=同一
 */
bool UdmModel::equalsModel(const UdmModel &model) const
{
    if (this->getId() != model.getId()) return false;
    if (this->getName() != model.getName()) return false;
    if (this->getCellDimension() != model.getCellDimension()) return false;
    if (this->getPhysicalDimension() != model.getPhysicalDimension()) return false;
    if (this->getSimulationType() != model.getSimulationType()) return false;

    return true;
}


/**
 * モデルの基本情報を設定する.
 * @param index_base            ベースID
 * @param name_base                ベース名
 * @param cell_dim                要素（セル）次元数
 * @param phys_dim                ノードの次元数
 * @param simulation_type        シミュレーションタイプ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::createModel(
                        UdmSize_t index_base,
                        std::string name_base,
                        int cell_dim,
                        int phys_dim,
                        UdmSimulationType_t simulation_type)
{
    if (index_base <= 0) return UDM_ERROR;
    if (name_base.empty()) return UDM_ERROR;
    if (cell_dim <= 0) return UDM_ERROR;
    if (phys_dim <= 0) return UDM_ERROR;
    if (simulation_type == Udm_SimulationTypeUnknown) {
        UDM_WARNINGNO_HANDLER(UDM_WARNING_CGNS_SIMULATIONTYPEUNKNOWN);
        simulation_type = Udm_TimeAccurate;
    }

    this->setId(index_base);
    this->setName(name_base);
    this->setCellDimension(cell_dim);
    this->setPhysicalDimension(phys_dim);
    this->setSimulationType(simulation_type);

    return UDM_OK;
}


/**
 * CGNS:UdmInfoを読込を行う.
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::readCgnsUdmInfo(int index_file, int index_base)
{
    cgsize_t user_dims[2] = {0, 0};
    char version[33] = {0x00};
    int rank_info[2] = {0, 0};
    int n;
    char array_name[33] = {0x00};
    DataType_t cgns_datatype;
    int dimension;
    cgsize_t dimension_vector[3] = {1};

    // CGNS:Base
    if (cg_goto(index_file, index_base, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d, cgns_error=%s", index_file, cg_get_error());
    }

    // "UdmInfo" : UDMlib情報
    if (cg_goto(index_file, index_base, UDM_CGNS_NAME_UDMINFO, 0, "end") != CG_OK) {
        // UDMlib情報が存在しない。
        UDM_WARNING_HANDLER(UDM_WARNING_NOTFOUND_CGNSNODE, "not found CGNS:UdmInfo.");
        return UDM_OK;
    }

    int num_arrays, len = 0, index_array = 0;
    cg_narrays(&num_arrays);
    for (n=1; n<=num_arrays; n++) {
        if (cg_array_info(n, array_name, &cgns_datatype, &dimension, dimension_vector) != CG_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_info(%d)", n);
        }

        // UDMlib:バージョン : CGNS:DataArray[@name="UDMlibVersion"]
        if (strncmp(array_name, UDM_CGNS_NAME_UDMINFO_VERSION, strlen(UDM_CGNS_NAME_UDMINFO_VERSION)) == 0) {
            if (cg_array_read_as(n, Character, &version) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_read_as(%d=%s)", n, UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
            }
            this->setUdminfoVersion(version);
        }

        // 出力ランク番号 : CGNS:DataArray[@name="OutputRankInfo"]
        if (strncmp(array_name, UDM_CGNS_NAME_UDMINFO_RANKINFO, strlen(UDM_CGNS_NAME_UDMINFO_RANKINFO)) == 0) {
            if (cg_array_read_as(n, Integer, &rank_info) != CG_OK) {
                return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_read_as(%d=%s)", n, UDM_CGNS_NAME_RANKCONNECTIVITY_ARRAY);
            }
            this->setUdminfoProcessSize(rank_info[0]);
            this->setUdminfoRankno(rank_info[1]);
        }
    }

    return UDM_OK;
}

/**
 * CGNS:UdmInfo(UDMlib情報)を出力する
 * @param index_file        CGNS:ファイルインデックス
 * @param index_base        CGNS:ベースインデックス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsUdmInfo(int index_file, int index_base)
{
    cgsize_t user_dims[2] = {0, 0};
    char version[33] = {0x00};
    udm_get_version(version);
    char description[] = UDM_CGNS_TEXT_UDMINFO_DESCRIPTION;
    int rank_info[2] = { this->getMpiProcessSize(), this->getMpiRankno()};

    // "UdmInfo" : UDMlib情報
    if (cg_goto(index_file, index_base, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d, cgns_error=%s", index_file, cg_get_error());
    }

    // CGNS:UserDefinedData[@name="UdmInfo"]の作成 : cg_user_data_write
    if (cg_user_data_write(UDM_CGNS_NAME_UDMINFO) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_user_data_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO, cg_get_error());
    }
    if (cg_goto(index_file, index_base, UDM_CGNS_NAME_UDMINFO, 0, "end") != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_GOTO, "index_file=%d", index_file);
    }

    // UDMlib:バージョン
    // CGNS:DataArray[@name="UDMlibVersion"]の作成 : cg_array_write
    user_dims[0] = 32;
    user_dims[1] = 1;
    if (cg_array_write( UDM_CGNS_NAME_UDMINFO_VERSION,
                        Character,
                        2,
                        user_dims,
                        version) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO_VERSION, cg_get_error());
    }

    // UDMlib:説明
    // CGNS:DataArray[@name="UDMlibDescription"]の作成 : cg_array_write
    user_dims[0] = strlen(description);
    user_dims[1] = 1;
    if (cg_array_write( UDM_CGNS_NAME_UDMINFO_DESCRIPTION,
                        Character,
                        2,
                        user_dims,
                        description) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO_DESCRIPTION, cg_get_error());
    }

    // 出力ランク番号
    // CGNS:DataArray[@name="OutputRankInfo"]の作成 : cg_array_write
    user_dims[0] = 2;
    user_dims[1] = 0;
    if (cg_array_write( UDM_CGNS_NAME_UDMINFO_RANKINFO,
                        Integer,
                        1,
                        user_dims,
                        rank_info) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_USERDEFINEDDATA, "failure : cg_array_write(%s), cgns_error=%s", UDM_CGNS_NAME_UDMINFO_RANKINFO, cg_get_error());
    }

    return UDM_OK;
}

/**
 * CGNS:UdmInfo:CGNS出力時のプロセスの総数を取得する.
 * @return        CGNS:UdmInfo:CGNS出力時のプロセスの総数
 */
int UdmModel::getUdminfoProcessSize() const
{
    return this->udminfo_process_size;
}

/**
 * CGNS:UdmInfo:CGNS出力時のプロセスの総数を設定する.
 * @param process_size        CGNS:UdmInfo:CGNS出力時のプロセスの総数
 */
void UdmModel::setUdminfoProcessSize(int process_size)
{
    this->udminfo_process_size = process_size;
}

/**
 * CGNS:UdmInfo:CGNS出力時のランク番号を取得する
 * @return        CGNS:UdmInfo:CGNS出力時のランク番号
 */
int UdmModel::getUdminfoRankno() const
{
    return this->udminfo_rankno;
}

/**
 * CGNS:UdmInfo:CGNS出力時のランク番号を設定する.
 * @param rankno        CGNS:UdmInfo:CGNS出力時のランク番号
 */
void UdmModel::setUdminfoRankno(int rankno)
{
    this->udminfo_rankno = rankno;
}

/**
 * CGNS:UdmInfo:CGNS出力時のUDMlibのバージョンを取得する.
 * @return        CGNS:UdmInfo:CGNS出力時のUDMlibのバージョン
 */
const std::string& UdmModel::getUdminfoVersion() const
{
    return this->udminfo_version;
}

/**
 * CGNS:UdmInfo:CGNS出力時のUDMlibのバージョンを設定する.
 * @param version        CGNS:UdmInfo:CGNS出力時のUDMlibのバージョン
 */
void UdmModel::setUdminfoVersion(const std::string& version)
{
    this->udminfo_version = version;
}

/**
 * 出力CGNSファイルをクローズする.
 * @param index_file        CGNSファイルインデックス
 */
void UdmModel::closeWriteCgnsFile(int index_file)
{
    cg_close(index_file);

    return;
}

/**
 * DFI設定に従って、CGNSファイル, index.dfiを出力する.
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @param average_step            平均ステップ数
 * @param average_time            平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeModel(
                        int timeslice_step,
                        float timeslice_time,
                        int average_step,
                        float average_time)
{
    UdmError_t error = UDM_OK;

    if (this->config == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_VARIABLE, "config is null.");
    }

    // CGNSファイルを出力する
    error = this->writeCgnsModel(timeslice_step, timeslice_time);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 時系列情報：TimeSliceを追加する
    this->config->insertTimeSlice(
                        timeslice_step,
                        timeslice_time,
                        average_step,
                        average_time);

    // DFI:index.dfi出力
    error = this->writeDfi();
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}


/**
 * DFI設定に従って、CGNSファイル, index.dfiを出力する.
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeModel(int timeslice_step, float timeslice_time)
{
#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_START(__FUNCTION__);
#endif

    UdmError_t error = UDM_OK;

    if (this->config == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_VARIABLE, "config is null.");
    }

    // CGNSファイルを出力する
    error = this->writeCgnsModel(timeslice_step, timeslice_time);
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    // 時系列情報：TimeSliceを追加する
    this->config->insertTimeSlice(
                        timeslice_step,
                        timeslice_time);

    // DFI:index.dfi出力
    error = this->writeDfi();
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

#ifdef _UDM_PROFILER
    udm_mpi_barrier(this->getMpiComm());
    UDM_STOPWATCH_STOP(__FUNCTION__);
#endif

    return UDM_OK;
}


/**
 * DFI設定に従って、CGNSファイルを出力する.
 * @param timeslice_step        時系列ステップ数
 * @param timeslice_time        時系列ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeCgnsModel(int timeslice_step, float timeslice_time)
{
    int myrank = this->getMpiRankno();
    UdmError_t error = UDM_OK;
    std::vector<std::string> write_files;
    bool write_constgrid = false;
    bool write_constsolutions = false;
    bool initialize_cgns = false;
    bool initialize_linkcgns = false;
    UdmSize_t num_nodes = 0;
    int n;

    // DFI出力ディレクトリを作成する
    std::string output_dir;
    output_dir.clear();
    if (this->config->getDfiOutputDirectory(output_dir) == UDM_OK) {
        if (!output_dir.empty()) {
            if (udm_make_directories(output_dir.c_str()) != 0) {
                error = UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", output_dir.c_str());
            }
        }
    }
    // CGNS出力ディレクトリを作成する
    output_dir.clear();
    if (this->config->getCgnsOutputDirectory(output_dir) == UDM_OK) {
        if (!output_dir.empty()) {
            if (udm_make_directories(output_dir.c_str()) != 0) {
                error = UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", output_dir.c_str());
            }
        }
    }
    // CGNS時系列出力ディレクトリを作成する
    output_dir.clear();
    if (this->config->getCgnsTimeSliceDirectory(output_dir, timeslice_step) == UDM_OK) {
        if (!output_dir.empty()) {
            if (udm_make_directories(output_dir.c_str()) != 0) {
                error = UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", output_dir.c_str());
            }
        }
    }
    if (error != UDM_OK) return error;

    // 初回出力であるかチェックする.
    if (this->config->getNumCurrentTimeSlices() <= 0) {
        initialize_linkcgns = true;
        initialize_cgns = true;
        write_constsolutions = true;        // 初回のみ固定値物理量出力を行う
    }

    // 時系列構成のファイル出力であるかチェックする.
    if (!this->config->getFileinfoConfig()->isFileTimeSlice()) {
        // 時系列毎のファイル出力であるので、毎回初期化を行う.
        initialize_cgns = true;
    }

    // 固定値GridCoordinatesチェック
    if (this->getConfigFileinfo()->isFileGridConstant()) {
        write_constgrid = true;
    }

    // 出力GridCoordinatesチェック
    num_nodes = 0;
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        num_nodes += zone->getNumNodes();

        // 初期化を行う。
        zone->initializeWriteCgns();
    }
    if (num_nodes <= 0) {
        // 出力ノードが存在しないので、CGNSファイルを出力しない。
        UDM_WARNING_HANDLER(UDM_ERROR_CGNS_INVALID_GRIDCOORDINATES, "GridCoordinates(node) is empty.");
    }

    // CGNS:GridCoordinates出力
    if ((num_nodes > 0)
        && this->getConfigFileinfo()->isFileGridCoordinates()) {
        std::string gridfile;
        this->config->getCgnsGridFilePath(gridfile, myrank, timeslice_step);
        // 初期化
        if (initialize_cgns) {
            this->initializeCgnsFile(gridfile.c_str());
        }

        // CGNS:GridCoordinates出力
        error = this->writeCgnsGridCoordinatesFile(gridfile.c_str(), timeslice_step, timeslice_time, !write_constgrid);
        if (error == UDM_OK) {
            write_files.push_back(gridfile);
        }
    }
    if (error != UDM_OK) return error;

    // CGNS:FlowSolution出力
    if ((num_nodes > 0)
        && this->getConfigFileinfo()->isFileFlowSolution()) {
        std::string solutionfile;
        this->config->getCgnsSolutionFilePath(solutionfile, myrank, timeslice_step);
        // 初期化
        if (initialize_cgns) {
            this->initializeCgnsFile(solutionfile.c_str());
        }

        // CGNS:FlowSolution出力
        error = this->writeCgnsFlowSolutionFile(solutionfile.c_str(), timeslice_step, timeslice_time, write_constsolutions);
        if (error == UDM_OK) {
            write_files.push_back(solutionfile);
        }
    }
    if (error != UDM_OK) return error;

    // CGNS:GridCoordinates+CGNS:FlowSolution出力
    if ((num_nodes > 0)
        && this->getConfigFileinfo()->isFileIncludeGrid()) {
        std::string timeslicefile;
        this->config->getCgnsTimeSliceFilePath(timeslicefile, myrank, timeslice_step);
        // 初期化
        if (initialize_cgns) {
            this->initializeCgnsFile(timeslicefile.c_str());
        }
        // CGNS:GridCoordinates+CGNS:FlowSolution出力
        error = this->writeCgnsTimeSliceFile(timeslicefile.c_str(), timeslice_step, timeslice_time, !write_constgrid, write_constsolutions);
        if (error == UDM_OK) {
            write_files.push_back(timeslicefile);
        }
    }
    if (error != UDM_OK) return error;

    // リンクファイル出力
    if (num_nodes > 0) {
        std::string linkfile;
        this->config->getCgnsLinkFilePath(linkfile, myrank);

        // リンクファイルを出力するかチェックする
        bool write_linkfile = false;
        if (write_files.size() > 1) write_linkfile = true;
        if (write_files.size() == 1) {
            if (write_files[0] != linkfile) write_linkfile = true;
        }

        // CGNS:リンクファイル出力
        if (write_linkfile) {
            // 初期化
            if (initialize_linkcgns) {
                this->initializeCgnsFile(linkfile.c_str());
            }
            error = this->writeCgnsLinkFile(linkfile.c_str(), write_files, timeslice_step, timeslice_time, !write_constgrid);
        }
    }

    return error;
}

/**
 * index.dfiファイルを出力する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeDfi()
{
    UdmError_t error = UDM_OK;

    // ゾーン１固定とする.
    int zone_id = 1;
    UdmZone *zone = this->getZone(zone_id);
    if (zone != NULL) {
        // プロセス、ドメイン、MPI情報を更新する.
        error = zone->updateProcessConfigs();
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }

    // ランク０のみが出力する.
    if (this->getMpiRankno() != 0) return UDM_OK;

    // DFI:index.dfi出力
    std::string index_dfi;
    this->config->getWriteIndexDfiPath(index_dfi);
    error = this->config->writeDfi(index_dfi.c_str());
    if (error != UDM_OK) {
        return UDM_ERRORNO_HANDLER(error);
    }

    return UDM_OK;
}

/**
 * CGNSファイルを初期化する.
 * CGNSファイルをWRITEモードに開いて閉じる.
 * @param cgns_filename        CGNSファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::initializeCgnsFile(const char* cgns_filename)
{
    int open_mode = CG_MODE_WRITE;        // modes for cgns file
    int index_file;

    // CGNSファイル:open
    if (cg_open(cgns_filename, open_mode, &index_file) != CG_OK) {
        return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_OPENERROR, "filename=%s, cgns_error=%s", cgns_filename, cg_get_error());
    }
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}

/**
 * CGNS:UdmInfo:ranknoを取得する.
 * CGNS:UdmInfo:ranknoが未設定の場合はMPI:ranknoを取得する.
 * @return        ランク番号
 */
int UdmModel::getCgnsRankno() const
{
    int rankno = this->getUdminfoRankno();
    if (rankno < 0) {
        rankno = this->getMpiRankno();
    }
    if (rankno < 0) {
        rankno = 0;
    }

    return rankno;
}


/**
 * UdmRankConnectivityの接続面をCGNSファイル出力する.
 * UdmRankConnectivityの検証用関数.
 * @param cgns_filename        出力CGNSファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeRankConnectivity(const char* cgns_filename)
{
    int index_file,index_base;
    int n;

    // ゾーン出力前の初期化を行う。
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->initializeWriteCgns() != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : UdmZone::initializeWriteCgns(zoneid=%d)", n);
        }
    }

    // CGNS:Baseを出力する.
    if (this->writeCgnsBase(cgns_filename, index_file, index_base) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNS:Zoneを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeRankConnectivity(index_file, index_base) != UDM_OK) {
            this->closeWriteCgnsFile(index_file);
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // CGNSファイルをクローズする。
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}


/**
 * 仮想セルをCGNSファイル出力する.
 * 仮想セルの検証用関数.
 * @param cgns_filename        出力CGNSファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::writeVirtualCells(const char* cgns_filename)
{
    int index_file,index_base;
    int n;

    // ゾーン出力前の初期化を行う。
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->initializeWriteCgns() != UDM_OK) {
            return UDM_ERROR_HANDLER(UDM_ERROR_CGNS_INVALID_BASE, "failure : UdmZone::initializeWriteCgns(zoneid=%d)", n);
        }
    }

    // CGNS:Baseを出力する.
    if (this->writeCgnsBase(cgns_filename, index_file, index_base) != UDM_OK) {
        this->closeWriteCgnsFile(index_file);
        return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_BASE);
    }

    // CGNS:Zoneを出力する.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (zone->writeVirtualCells(index_file, index_base) != UDM_OK) {
            this->closeWriteCgnsFile(index_file);
            return UDM_ERRORNO_HANDLER(UDM_ERROR_CGNS_INVALID_ZONE);
        }
    }

    // CGNSファイルをクローズする。
    this->closeWriteCgnsFile(index_file);

    return UDM_OK;
}

/**
 * 仮想セルの転送を行う.
 * @param [out] virtual_nodes        転送節点（ノード）リスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::transferVirtualCells(std::vector<UdmNode*>& virtual_nodes)
{
    int n;
    UdmError_t error = UDM_OK;
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        error = zone->transferVirtualCells(virtual_nodes);
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }

    return UDM_OK;
}

/**
 * 仮想セルの転送を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::transferVirtualCells()
{
    int n;
    UdmError_t error = UDM_OK;
    std::vector<UdmNode*> virtual_nodes;
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        error = zone->transferVirtualCells(virtual_nodes);
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }
    return UDM_OK;
}

/**
 * 内部CGNSデータの検証を行う.
 * @return        true=検証OK
 */
bool UdmModel::validateCgns() const
{
    int n;
    bool validate = true;

    // CGNS:Zoneの検証を行う.
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        if (!zone->validateCgns()) {
            validate = false;
        }
    }
    return validate;
}

/**
 * 部品要素の作成を行う.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::createComponentCells()
{
    int n;
    UdmError_t error = UDM_OK;
    for (n=1; n<= this->getNumZones(); n++) {
        UdmZone* zone = this->getZone(n);
        error = zone->createComponentCells();
        if (error != UDM_OK) {
            return UDM_ERRORNO_HANDLER(error);
        }
    }
    return error;
}

/**
 * 出力ディレクトリを設定する.
 * @param path        出力ディレクトリ
 */
void UdmModel::setOutputPath(const char* path)
{
    if (this->config == NULL) return;
    this->config->setOutputPath(std::string(path));
    return;
}

/**
 * ゾーンの分割を行う。
 * @param zone_id        ゾーンID（１～）
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::partitionZone(int zone_id)
{
    if (this->partition == NULL) {
        // return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmLoadBalance is null.");
        this->partition = new UdmLoadBalance();
    }
    UdmZone *zone = this->getZone(zone_id);
    if (zone == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "UdmZone is null[zone_id=%d].", zone_id);
    }
    UdmError_t error = this->partition->partitionZone(zone);

    return error;
}

/**
 * ロードバランスクラスを取得する.
 * @return        ロードバランスクラス
 */
UdmLoadBalance* UdmModel::getLoadBalance()
{
    if (this->partition == NULL) {
        this->partition = new UdmLoadBalance();
    }
    return this->partition;
}

/**
 * 時系列情報を初期化する.
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmModel::initializeTimeSlice()
{
    if (this->config == NULL) return UDM_ERROR;
    UdmTimeSliceConfig *timeslice_config =this->config->getTimesliceConfig();
    if (timeslice_config == NULL) return UDM_ERROR;
    // 時系列情報を初期化する
    return timeslice_config->clearCurrentTimeSlices();
}


} /* namespace udm */
