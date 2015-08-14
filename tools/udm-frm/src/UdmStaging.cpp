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

/**
 * @file   UdmStaging.cpp
 * @brief  UdmStaging class 関数
 */

#include "UdmStaging.h"


namespace udm {
/**
 * コンストラクタ
 */
UdmStaging::UdmStaging()
{
    // 初期化を行う
    initialize();
}

/**
 * デストラクタ
 */
UdmStaging::~UdmStaging()
{
}

/**
 * 初期化を行う。
 */
void UdmStaging::initialize()
{
    this->index_dfi.clear();
    this->input_dir.clear();
    this->output_dir = "./";        // 出力ディレクトリのデフォルトは./とする
    this->num_procs=0;
    this->debug_trace = false;
    this->print_version = false;
    this->udmlib_file.clear();
    this->stepno = -1;
}


/**
 * index.dfiファイルを読み込む.
 * @return        true=success
 */
bool UdmStaging::readDfi()
{
    return this->readDfi(this->index_dfi.c_str());
}


/**
 * index.dfiファイルを読み込む.
 * @param index_dfi        入力index.dfi
 * @return        true=success
 */
bool UdmStaging::readDfi(const char* index_dfi)
{
    if (index_dfi == NULL) {
        printf("Error : empty index.dfi.\n");
        return false;
    }
    if (strlen(index_dfi) <= 0) {
        printf("Error : empty index.dfi.\n");
        return false;
    }

    // index.dfiファイルの読込
    if (this->dfi_config.readDfi(index_dfi) != UDM_OK) {
        printf("Error : can not read index.dfi.\n");
        return false;
    }

    // 入力ディレクトリの取得
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};

    // ファイル名分解, 拡張子チェック
    udm_splitpath(index_dfi, drive, folder, name, ext);
    this->input_dir = folder;

    return true;
}

/**
 * ファイルの振分けを行う.
 * @return        true=success
 */
bool UdmStaging::mappingFiles()
{
    return this->mappingFiles(this->num_procs);
}

/**
 * ファイルの振分けを行う.
 * @param num_procs            振分プロセス数
 * @return        true=success
 */
bool UdmStaging::mappingFiles(int num_procs)
{
    if (num_procs <= 0) {
        printf("Error : number of process is zero.");
        return false;
    }
    if (this->index_dfi.empty()) {
        printf("Error : index.dfi is empty.");
        return false;
    }
    const UdmFileInfoConfig *fileinfo_config = this->dfi_config.getFileinfoConfig();
    if (fileinfo_config == NULL) {
        printf("Error : UdmFileInfoConfig is null.");
        return false;
    }
    const UdmFilePathConfig *filepath_config = this->dfi_config.getFilepathConfig();
    if (filepath_config == NULL) {
        printf("Error : UdmFilePathConfig is null.");
        return false;
    }

    // CGNSファイル接頭文字
    std::string prefix;
    fileinfo_config->getPrefix(prefix);
    if (prefix.empty()) {
        printf("Error : FileInfo::prefix is empty.");
        return false;
    }

    // index.dfiのファイル名
    char indexdfi_name[128] = {0x00};
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    // ファイル名分解
    udm_splitpath(this->index_dfi.c_str(), drive, folder, name, ext);
    sprintf(indexdfi_name, "%s%s", name, ext);

    // DirectoryPath
    std::string field_directory;
    fileinfo_config->getDirectoryPath(field_directory);

    // TimeSliceDirectory
    bool timeslicedirectory = fileinfo_config->isTimeSliceDirectory();

    // proc.dfiファイル名
    std::string procdfi_name;
    filepath_config->getProcess(procdfi_name);
    if (procdfi_name.empty()) {
        printf("Error : FilePath::process[proc.dfi] is empty.");
        return false;
    }

    // udmlib.tpのファイル名
    char udmlibtp_name[128] = {0x00};
    char udmlibtp_drive[32]={0x00}, udmlibtp_folder[256]={0x00};
    // ファイル名分解
    udm_splitpath(this->udmlib_file.c_str(), udmlibtp_drive, udmlibtp_folder, name, ext);
    sprintf(udmlibtp_name, "%s%s", name, ext);

    // Process
    const UdmProcessConfig *process_config = this->dfi_config.getProcessConfig();
    if (process_config == NULL) {
        printf("Error : UdmProcessConfig is null.");
        return false;
    }
    int num_rankconfig = process_config->getNumRankConfig();
    if (num_rankconfig <= 0) {
        printf("Error : Number of rank is zero in proc.dfi.");
        return false;
    }
    // デバッグトレース
    if (this->isDebugTrace()) {
        printf("[udm-frm] File Mapping from %d to %d rank.\n", num_rankconfig, num_procs);
    }

    int n = 0;
    char input_dfi[512] = {0x00};
    char output_dfi[512] = {0x00};
    char output_rank[512] = {0x00};
    char rank_dir[512] = {0x00};

    // 出力ディレクトリの設定 : 絶対パスとする.
    char current_dir[1024] = {0x00};
    char staging_directory[1024] = {0x00};
    udm_getcwd(current_dir);
    if (udm_is_absolutepath(this->output_dir.c_str())) {
        strcpy(staging_directory, this->output_dir.c_str());
    }
    else {
        udm_connect_path(current_dir, this->output_dir.c_str(), staging_directory);
    }

    // 振分プロセス数にて実行時の読込ランク番号テーブルの取得
    for (n=0; n<num_procs; n++) {
        // 出力パス
        sprintf(rank_dir, UDMFRM_OUTPUT_RANKFORMAT, n);
        udm_connect_path(staging_directory, rank_dir, output_rank);

        // デバッグトレース
        if (this->isDebugTrace()) {
            printf("\n[udm-frm] output rank path = %s\n", output_rank);
        }

        std::vector<int> input_rank_ids;
        // 読込プロセス番号の取得
        int num_ranks = this->dfi_config.makeIoRankTable(num_procs, n, input_rank_ids);

        // cgnsファイルのコピーを行う。
        std::vector<int>::iterator itr;
        for (itr=input_rank_ids.begin(); itr!=input_rank_ids.end(); itr++) {
            int rankno = (*itr);
            if (this->stepno >= 0) {
                std::string read_cgns;
                this->dfi_config.getCgnsInputFilePath(read_cgns, rankno);
                // 対象ランク番号のCGNSファイルからステップ番号のデータを出力する。
                if (!this->writeCgnsFiles(
                                this->index_dfi.c_str(),
                                read_cgns.c_str(),
                                output_rank,
                                rankno,
                                this->stepno)) {
                    return false;
                }
            }
            else {
                // 対象ランク番号のCGNSファイルをコピーする。
                if (!this->copyCgnsFiles(
                                this->input_dir.c_str(),
                                output_rank,
                                prefix.c_str(),
                                rankno,
                                field_directory.c_str(),
                                timeslicedirectory)) {
                    return false;
                }
            }
        }

        if (this->stepno >= 0) {
            if (!this->writeDfiFiles(
                    this->index_dfi.c_str(),
                    output_rank,
                    n,
                    this->stepno)) {
                return false;
            }
        }
        else {
            // index.dfiのコピー
            udm_connect_path(output_rank, indexdfi_name, output_dfi);
            if (!this->copyFile(this->index_dfi.c_str(), output_dfi)) {
                return false;
            }

            // proc.dfiのコピー
            udm_connect_path(this->input_dir.c_str(), procdfi_name.c_str(), input_dfi);
            udm_connect_path(output_rank, procdfi_name.c_str(), output_dfi);
            if (!this->copyFile(input_dfi, output_dfi)) {
                return false;
            }
        }

        // udmlib.tp : with-udmlibに指定されたファイルをコピーする.
        if (!this->udmlib_file.empty()) {
            udm_connect_path(output_rank, udmlibtp_name, output_dfi);
            if (!this->copyFile(this->udmlib_file.c_str(), output_dfi)) {
                return false;
            }
        }
    }

    return true;
}

/**
 * UDM:設定クラスを取得する.
 * @return        UDM:設定クラス
 */
const UdmDfiConfig& UdmStaging::getDfiConfig() const
{
    return this->dfi_config;
}


/**
 * index.dfiファイル名を取得する.
 * @return        index.dfiファイル名
 */
const std::string& UdmStaging::getIndexDfi() const
{
    return this->index_dfi;
}

/**
 * index.dfiファイル名を設定する.
 * @param filename            index.dfiファイル名
 */
void UdmStaging::setIndexDfi(const std::string& filename)
{
    this->index_dfi = filename;
}

/**
 * 入力ディレクトリを取得する.
 * @return        入力ディレクトリ
 */
const std::string& UdmStaging::getInputDirectory() const
{
    return this->input_dir;
}

/**
 * 入力ディレクトリを設定する.
 * @param path            入力ディレクトリ
 */
void UdmStaging::setInputDirectory(const std::string& path)
{
    this->input_dir = path;
}

/**
 * 振分プロセス数を取得する.
 * @return        振分プロセス数
 */
int UdmStaging::getNumProcess() const
{
    return this->num_procs;
}

/**
 * 振分プロセス数を設定する.
 * @param num        振分プロセス数
 */
void UdmStaging::setNumProcess(int num)
{
    this->num_procs = num;
}

/**
 * 出力ディレクトリを取得する.
 * @return        出力ディレクトリ
 */
const std::string& UdmStaging::getOutputDirectory() const
{
    return this->output_dir;
}

/**
 * 出力ディレクトリを設定する.
 * @param path        出力ディレクトリ
 */
void UdmStaging::setOutputDirectory(const std::string& path)
{
    this->output_dir = path;
}

/**
 * ランク番号記述のCGNSファイルをコピーする.
 * @param input_dir        コピー元ディレクトリ
 * @param output_dir    コピー先ディレクトリ
 * @param prefix        ファイル接頭文字列
 * @param rankno        ランク番号
 * @param field_directory     フィールドディレクトリ
 * @param timeslicedirectory   時系列ディレクトリの有無
 * @return        true=success
 */
bool UdmStaging::copyCgnsFiles(
                        const char *input_dir,
                        const char *output_dir,
                        const char *prefix,
                        int rankno,
                        const char *field_directory,
                        bool timeslicedirectory) const
{
    if (input_dir == NULL || strlen(input_dir) <= 0) return false;
    if (output_dir == NULL || strlen(output_dir) <= 0) return false;
    if (prefix == NULL || strlen(prefix) <= 0) return false;
    if (rankno < 0) return false;

    int n;
    // コピー元ディレクトリのファイル一覧の取得
    std::vector<std::string> filelist;
    int num_files = this->getFileList(input_dir, filelist);
    for (n=0; n<num_files; n++) {
        std::string filename = filelist[n];

        // コピー対象のファイルであるかチェックする.
        if (!checkCgnsFile(filename.c_str(), prefix, rankno)) continue;

        // CGNSファイルをコピーする.
        char src_path[1024] = {0x00};
        char dest_path[1024] = {0x00};
        udm_connect_path(input_dir, filename.c_str(), src_path);
        udm_connect_path(output_dir, filename.c_str(), dest_path);
        if (!this->copyFile(src_path, dest_path)) {
            return false;
        }
    }

    // フィールドディレクトリ
    if (field_directory != NULL && strlen(field_directory) > 0) {
        char input_field[1024] = {0x00};
        char output_field[1024] = {0x00};
        udm_connect_path(input_dir, field_directory, input_field);
        udm_connect_path(output_dir, field_directory, output_field);
        if (!copyCgnsFiles(input_field, output_field,
                            prefix, rankno,
                            NULL, timeslicedirectory)) {
            printf("Error : can not copy input_dir[%s] to output_dir[%s].\n", input_field, output_field);
            return false;
        }
    }
    else if (timeslicedirectory) {
        // コピー元ディレクトリのディレクトリ一覧の取得
        std::vector<std::string> dirlist;
        int num_dirs = this->getDirectoryList(input_dir, dirlist);
        for (n=0; n<num_dirs; n++) {
            std::string dirname = dirlist[n];

            // 時系列のディレクトリであるかチェックする.
            if (!checkTimeSliceDirectory(dirname.c_str())) continue;

            char input_time[1024] = {0x00};
            char output_time[1024] = {0x00};
            udm_connect_path(input_dir, dirname.c_str(), input_time);
            udm_connect_path(output_dir, dirname.c_str(), output_time);
            if (!copyCgnsFiles(
                        input_time, output_time,
                        prefix, rankno,
                        field_directory, false)) {
                printf("Error : can not copy input_dir[%s] to output_dir[%s].\n", input_time, output_time);
                return false;
            }
        }
    }

    return true;
}

/**
 * コピー対象のファイルであるかチェックする.
 * @param filename            チェック対象CGNSファイル名
 * @param prefix        ファイル接頭文字列
 * @param rankno        ランク番号
 * @return        true=コピー対象のCGNSファイル
 */
bool UdmStaging::checkCgnsFile(const char *filename, const char *prefix, int rankno) const
{
    if (filename == NULL || strlen(filename) <= 0) return false;
    if (prefix == NULL || strlen(prefix) <= 0) return false;
    if (rankno < 0) return false;

    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};

    // ファイル名分解, 拡張子チェック
    udm_splitpath(filename, drive, folder, name, ext);
    if (strcmp(ext+1, "cgns") != 0) {
        return false;
    }

    // ファイル接頭文字列のチェック
    const char *prefix_pos = strstr(filename, prefix);
    if (filename != prefix_pos) {
        return false;
    }

    // ランク番号のチェック
    char rankid[32] = {0x00};
    sprintf(rankid, UDM_DFI_FORMAT_PROCESS, rankno);
    if (strstr(filename, rankid) == NULL) {
        return false;
    }

    return true;
}


/**
 * ディレクトリ配下のファイルの一覧を取得する.
 * ディレクトリは取得しない.
 * @param [in] dirname        一覧取得ディレクトリ
 * @param [out] filelist        取得ファイルリスト
 * @return        取得ファイル数, -1=エラー
 */
int UdmStaging::getFileList(
                const std::string& dirname,
                std::vector<std::string>& filelist) const
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char path[512] = {0x00};

    filelist.clear();
    if(( dp = opendir(dirname.c_str()) ) == NULL ){
        printf("Error : can not found directory[%s]\n", dirname.c_str());
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0) continue;
        if (strcmp(entry->d_name, "..") == 0) continue;
        udm_connect_path(dirname.c_str(), entry->d_name, path);
        stat(path, &statbuf);
        if(S_ISREG(statbuf.st_mode)){
            filelist.push_back(std::string(entry->d_name));
        }
        // if(S_ISDIR(statbuf.st_mode)) {} // ディレクトリ
     }

    closedir(dp);

    return filelist.size();
}


/**
 * ディレクトリ配下のディレクトリの一覧を取得する.
 * ファイルは取得しない.
 * @param [in] dirname        一覧取得ディレクトリ
 * @param [out] dirlist        取得ディレクトリリスト
 * @return        取得ディレクトリ数, -1=エラー
 */
int UdmStaging::getDirectoryList(
                const std::string& dirname,
                std::vector<std::string>& dirlist) const
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char path[512] = {0x00};

    dirlist.clear();
    if(( dp = opendir(dirname.c_str()) ) == NULL ){
        printf("Error : can not found directory[%s]\n", dirname.c_str());
        return -1;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0) continue;
        if (strcmp(entry->d_name, "..") == 0) continue;
        udm_connect_path(dirname.c_str(), entry->d_name, path);
        stat(path, &statbuf);
        if(S_ISDIR(statbuf.st_mode)){
            dirlist.push_back(std::string(entry->d_name));
        }
        // if(S_ISREG(statbuf.st_mode)) {} // ファイル
     }

    closedir(dp);

    return dirlist.size();
}

/**
 * ファイルが存在するかチェックする.
 * ディレクトリの場合は、falseを返す.
 * @param filename        チェックファイルパス
 * @return        true=存在する
 */
bool UdmStaging::existsFile(const char *filename) const
{
    if (filename == NULL) return false;
    if (strlen(filename) <= 0) return false;

    struct stat sb;
    return (stat(filename, &sb) == 0 && S_ISREG(sb.st_mode));
}

/**
 * ディレクトリが存在するかチェックする.
 * ファイルの場合は、falseを返す.
 * @param dirname        チェックディレクトリパス
 * @return        true=存在する
 */
bool UdmStaging::existsDirectory(const char *dirname) const
{
    if (dirname == NULL) return false;
    if (strlen(dirname) <= 0) return false;

    struct stat sb;
    return (stat(dirname, &sb) == 0 && S_ISDIR(sb.st_mode));
}

/**
 * デバッグトレース（コピーファイル表示）であるかチェックする.
 * @return        true=デバッグトレース（コピーファイル表示）
 */
bool UdmStaging::isDebugTrace() const
{
    return this->debug_trace;
}

/**
 * デバッグトレース（コピーファイル表示）を設定する.
 * @param debugTrace        デバッグトレース（コピーファイル表示）
 */
void UdmStaging::setDebugTrace(bool debug)
{
    this->debug_trace = debug;
}

/**
 * バージョン表示を行うかチェックする.
 * @return        true=バージョン表示を行う.
 */
bool UdmStaging::isPrintVersion() const
{
    return this->print_version;
}

/**
 * バージョン表示を設定する
 * @param print            バージョン表示
 */
void UdmStaging::setPrintVersion(bool print)
{
    this->print_version = print;
}

/**
 * 実行パラメータを表示する.
 */
void UdmStaging::print_info() const
{
    printf("input filename       %s\n", this->index_dfi.c_str());
    printf("output directory     %s\n", this->output_dir.c_str());
    printf("number of process    %d\n", this->num_procs);
    if (this->stepno >= 0) {
        printf("mapping stepno       %d\n", this->stepno);
    }
    if (!this->udmlib_file.empty()) {
        printf("with udmlib.tp       %s\n", this->udmlib_file.c_str());
    }

    return;
}

/**
 * 時系列のディレクトリであるかチェックする.
 * @param directory            チェック対象ディレクトリ
 * @return        true=時系列のディレクトリ
 */
bool UdmStaging::checkTimeSliceDirectory(const char* directory) const
{
    if (directory == NULL) return false;
    if (strlen(directory) != UDMFRM_NUM_TIMESLICE_DIRECTORY)  return false;

    int n = 0;
    int len = strlen(directory);
    for (n=0; n<len; n++) {
        if (directory[n] < '0') return false;
        if (directory[n] > '9') return false;
    }

    return true;
}

/**
 * コピーステップ番号を取得する.
 * @return        コピーステップ番号
 */
int UdmStaging::getStepno() const
{
    return this->stepno;
}

/**
 * コピーステップ番号を設定する.
 * @param stepno        コピーステップ番号
 */
void UdmStaging::setStepno(int stepno)
{
    this->stepno = stepno;
}

/**
 * コピーudmlib.tpファイル名を取得する.
 * @return        コピーudmlib.tpファイル名
 */
const std::string& UdmStaging::getUdmlibFile() const
{
    return this->udmlib_file;
}

/**
 * コピーudmlib.tpファイル名
 * @param udmlib_tp        udmlib.tpファイル名
 */
void UdmStaging::setUdmlibFile(const std::string& udmlib_tp)
{
    this->udmlib_file = udmlib_tp;
}

/**
 * CGNSファイルの時系列ステップ番号のデータのみを出力する.
 * @param index_dfi            読込index.dfi
 * @param read_cgns            読込CGNSファイル
 * @param output_dir        出力先ディレクトリ : 絶対パスとする
 * @param rankno            読込ランク番号
 * @param stepno            入出力時系列ステップ番号
 * @return            true=success
 */
bool UdmStaging::writeCgnsFiles(
                        const char *index_dfi,
                        const char* read_cgns,
                        const char* output_dir,
                        int rankno,
                        int stepno) const
{
    UdmError_t error = UDM_OK;
    UdmModel model;

    // index.dfiの読込
    error = model.readDfi(index_dfi);
    if (error != UDM_OK) {
        printf("Error : can not read index.dfi[=%s].", index_dfi);
        return false;
    }

    // 時系列情報の取得
    const UdmSliceConfig* slice_config = NULL;
    const UdmDfiConfig *config = model.getDfiConfig();
    if (config != NULL) {
        const UdmTimeSliceConfig *timeslice_config = config->getTimesliceConfig();
        if (timeslice_config != NULL) {
            slice_config = timeslice_config->getPreviousSliceConfigByStep(stepno);
        }
    }
    if (slice_config == NULL) {
        printf("Error : can not get TimeSlice from index.dfi[=%s,stepno=%d].", index_dfi, stepno);
        return false;
    }

    // プロセス数の取得
    int prosess_size = 0;
    const UdmProcessConfig *process_config = config->getProcessConfig();
    if (process_config != NULL) {
        prosess_size = process_config->getNumRankConfig();
    }
    if (prosess_size <= 0) {
        printf("Error : can not get Process from index.dfi[=%s].", index_dfi);
        return false;
    }


    // MPIランク番号の設定
    model.setMpiRankno(rankno);
    model.setMpiProcessSize(prosess_size);

    // CGNSファイルの読込
    error = model.readCgns(read_cgns, stepno);
    if (error != UDM_OK) {
        printf("Error : can not read CGNS file[=%s].", read_cgns);
        return false;
    }

    // 出力ディレクトリの設定
    model.setOutputPath(output_dir);

    // CGNSファイルの出力
    if (slice_config->isSetAverageStep() || slice_config->isSetAverageTime() ) {
        error = model.writeModel(slice_config->getStep(),
                                slice_config->getTime(),
                                slice_config->getAverageStep(),
                                slice_config->getAverageTime());
    }
    else {
        error = model.writeCgnsModel(slice_config->getStep(),
                                 slice_config->getTime());
    }

    return true;
}


/**
 * DFIファイルの時系列ステップ番号のデータを出力する.
 * @param index_dfi            読込index.dfi
 * @param output_dir        出力先ディレクトリ : 絶対パスとする
 * @param rankno            読込ランク番号
 * @param stepno            入出力時系列ステップ番号
 * @return            true=success
 */
bool UdmStaging::writeDfiFiles(
                        const char *index_dfi,
                        const char* output_dir,
                        int rankno,
                        int stepno) const
{
    UdmError_t error = UDM_OK;
    UdmDfiConfig config;
    const UdmSliceConfig* slice_config = NULL;

    error = config.readDfi(index_dfi);
    const UdmTimeSliceConfig *timeslice_config = config.getTimesliceConfig();
    if (timeslice_config != NULL) {
        slice_config = timeslice_config->getPreviousSliceConfigByStep(stepno);
    }
    if (slice_config == NULL) {
        printf("Error : can not get TimeSlice from index.dfi[=%s,stepno=%d].", index_dfi, stepno);
        return false;
    }

    // プロセス数の取得
    int prosess_size = 0;
    const UdmProcessConfig *process_config = config.getProcessConfig();
    if (process_config != NULL) {
        prosess_size = process_config->getNumRankConfig();
    }
    if (prosess_size <= 0) {
        printf("Error : can not get Process from index.dfi[=%s].", index_dfi);
        return false;
    }

    // 出力時系列の設定
    if (slice_config->isSetAverageStep() || slice_config->isSetAverageTime() ) {
        config.insertTimeSlice(slice_config->getStep(),
                                slice_config->getTime(),
                                slice_config->getAverageStep(),
                                slice_config->getAverageTime());
    }
    else {
        config.insertTimeSlice(slice_config->getStep(),
                                 slice_config->getTime());
    }

    // 出力ディレクトリの設定
    config.setOutputPath(output_dir);
    // DFI:index.dfi出力
    std::string write_index_dfi;
    config.getWriteIndexDfiPath(write_index_dfi);
    error = config.writeDfi(write_index_dfi.c_str());
    if (error != UDM_OK) {
        printf("Error : can not write index.dfi[%s].", write_index_dfi.c_str());
        return false;
    }

    return true;
}


/**
 * ファイルをコピーする.
 * コピー先ファイルパスのディレクトリが存在しない場合は作成する.
 * @param src_path            コピー元ファイルパス
 * @param dest_path            コピー先ファイルパス
 * @return            true=success
 */
bool UdmStaging::copyFile(const char* src_path, const char* dest_path) const
{
    if (src_path == NULL || strlen(src_path) <= 0) return false;
    if (dest_path == NULL || strlen(dest_path) <= 0) return false;

    // コピー元ファイルが存在するかチェックする.
    if (!this->existsFile(src_path)) {
        printf("Error : not exists src file[%s]\n", src_path);
        return false;
    }

    // コピー先ファイルパスを作成する.
    char drive[32]={0x00}, folder[256]={0x00}, name[128]={0x00}, ext[32]={0x00};
    udm_splitpath(dest_path, drive, folder, name, ext);
    udm_make_directories(folder);

    // ファイルコピーを行う.
    int result = udm_copyfile(src_path, dest_path);
    if (result != 0) {
        printf("Error : can not copy file [src=%s, dest=%s]\n", src_path, dest_path);
        return false;
    }

    // デバッグトレース
    if (this->isDebugTrace()) {
        printf("[udm-frm:cp] %s -> %s\n", src_path, dest_path);
    }
    return true;
}

} /* namespace udm */

