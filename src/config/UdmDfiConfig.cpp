/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/*
 * @file  UdmDfiConfig.cpp
 * index.dfi,proc.dfiの入出力クラスのソースファイル
 */
#include "config/UdmDfiConfig.h"

namespace udm {

/**
 * コンストラクタ
 */
UdmDfiConfig::UdmDfiConfig() : UdmConfigBase(NULL), fileinfo(NULL), filepath(NULL), timeslice(NULL),
                            unitlist(NULL), flowsolutionlist(NULL)
{
    this->initialize();
}

/**
 * デストラクタ
 */
UdmDfiConfig::~UdmDfiConfig()
{
    if (this->fileinfo != NULL) delete this->fileinfo;        ///< index.dfi/FileInfoデータ
    if (this->filepath != NULL) delete this->filepath;        ///< index.dfi/FilePathデータ
    if (this->timeslice != NULL) delete this->timeslice;        ///< index.dfi/TimeSliceデータ
    if (this->unitlist != NULL) delete this->unitlist;        ///< index.dfi/UnitListデータ
    if (this->flowsolutionlist != NULL) delete this->flowsolutionlist;    ///< index.dfi/FlowSolutionデータ
    if (this->domain != NULL) delete this->domain;            ///< proc.dfi/Domainデータ
    if (this->mpi != NULL) delete this->mpi;            ///< proc.dfi/MPIデータ
    if (this->process != NULL) delete this->process;            ///< proc.dfi/Processデータ
}

/**
 * 初期化を行う.
 */
void UdmDfiConfig::initialize()
{

    this->fileinfo = new UdmFileInfoConfig();        ///< index.dfi/FileInfoデータ
    this->filepath = new UdmFilePathConfig();        ///< index.dfi/FilePathデータ
    this->timeslice = new UdmTimeSliceConfig();        ///< index.dfi/TimeSliceデータ
    this->unitlist = new UdmUnitListConfig();        ///< index.dfi/UnitListデータ
    this->flowsolutionlist = new UdmFlowSolutionListConfig();    ///< index.dfi/FlowSolutionデータ
    this->domain = new UdmDomainConfig();        ///< proc.dfi/Domainデータ
    this->mpi = new UdmMpiConfig();           ///< proc.dfi/MPIデータ
    this->process = new UdmProcessConfig();           ///< proc.dfi/Processデータ
    /// index.dfiのデフォルトファイル名を設定する.
    this->filename_input_dfi = UDM_DFI_FILENAME_INDEX;
    /// 出力パスのデフォルトパスを設定する.
    this->output_path = UDM_DFI_OUTPUT_PATH;
}

/**
 * ローカルファイルからindex.dfiファイルを読みこむ.
 * @param filename        index.dfiファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::readDfi(const char* filename)
{
    UdmError_t ret;
    int ierror = 0;
    FILE*fp = NULL;
    if( !(fp=fopen(filename, "rb")) ) {
        return UDM_ERROR_HANDLER(UDM_ERROR_READ_INDEXFILE_OPENERROR, "Can't open file. (%s)\n", filename);
    }
    fclose(fp);

    // input.dfiファイル名
    this->filename_input_dfi = filename;

    // TextParser
    TextParser *parser = new TextParser();
    TextParserError parser_ret = parser->read(filename);
    if (parser_ret != TP_NO_ERROR) {
        delete parser;
        return UDM_ERROR_HANDLER(UDM_ERROR_TEXTPARSER, "failure : TextParser::read(%s).", filename);
    }

    ierror = 0;
    // FileInfo
    this->fileinfo->setTextParser(parser);
    ret = this->fileinfo->read();
    if (ret != UDM_OK) {
        ierror++;
        UDM_ERROR_HANDLER(ret, "failure : UdmFileInfoConfig::read() [filename=%s].", filename);
    }

    // FilePath
    this->filepath->setTextParser(parser);
    ret = this->filepath->read();
    if (ret != UDM_OK) {
        ierror++;
        UDM_ERROR_HANDLER(ret, "failure : UdmFilePathConfig::read() [filename=%s].", filename);
    }

    // UnitList
    this->unitlist->setTextParser(parser);
    ret = this->unitlist->read();
    if (ret != UDM_OK && ret != UDM_WARNING_DFI_NOTFOUND_NODE) {
        ierror++;
        UDM_ERROR_HANDLER(ret, "failure : UdmUnitListConfig::read() [filename=%s].", filename);
    }

    // TimeSlice
    this->timeslice->setTextParser(parser);
    ret = timeslice->read();
    if (ret != UDM_OK && ret != UDM_WARNING_DFI_NOTFOUND_NODE) {
        ierror++;
        UDM_ERROR_HANDLER(ret, "failure : UdmTimeSliceConfig::read() [filename=%s].", filename);
    }

    // FlowSolutionList
    this->flowsolutionlist->setTextParser(parser);
    ret = this->flowsolutionlist->read();
    if (ret != UDM_OK && ret != UDM_WARNING_DFI_NOTFOUND_NODE) {
        this->flowsolutionlist->clear();
        UDM_ERROR_HANDLER(ret, "failure : UdmFlowSolutionListConfig::read() [filename=%s].", filename);
        ierror++;
    }

    delete parser;
    parser = NULL;

    // proc.dfiの読込み
    std::string procdfi_name;
    if (this->getReadProcDfiPath(procdfi_name) == UDM_OK) {
        ret = this->readProcDfi(procdfi_name.c_str());
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    if (ierror > 0) {
        return UDM_ERROR;
    }
    return UDM_OK;
}


/**
 * ローカルファイルからproc.dfiファイルを読みこむ.
 * @param filename        proc.dfiファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::readProcDfi(const char* filename)
{
    UdmError_t ret = UDM_OK;
    int ierror = 0;
    FILE*fp = NULL;
    if( !(fp=fopen(filename, "rb")) ) {
        return UDM_ERROR_HANDLER(UDM_ERROR_READ_PROCFILE_OPENERROR, "can not open file.(%s).", filename);
    }
    fclose(fp);

    // TextParser
    TextParser *parser = new TextParser();
    TextParserError parser_ret = parser->read(filename);
    if (parser_ret != TP_NO_ERROR) {
        delete parser;
        return UDM_ERROR_HANDLER(UDM_ERROR_TEXTPARSER, "failure : TextParser::read(%s).", filename);
    }

    ierror = 0;
    // Domain
    this->domain->setTextParser(parser);
    ret = this->domain->read();
    if (ret != UDM_OK) {
        UDM_ERROR_HANDLER(ret, "failure : UdmDomainConfig::read() [filename=%s].", filename);
        ierror++;
    }

    // MPI
    this->mpi->setTextParser(parser);
    ret = this->mpi->read();
    if (ret != UDM_OK) {
        UDM_ERROR_HANDLER(ret, "failure : UdmMpiConfig::read() [filename=%s].", filename);
        ierror++;
    }

    // Process
    this->process->setTextParser(parser);
    ret = this->process->read();
    if (ret != UDM_OK) {
        UDM_ERROR_HANDLER(ret, "failure : UdmProcessConfig::read() [filename=%s].", filename);
        ierror++;
    }

    delete parser;
    parser = NULL;

    if (ierror > 0) {
        return UDM_ERROR;
    }
    return UDM_OK;
}


/**
 * ローカルファイルにindex.dfiファイルを書き込む.
 * @param filename        index.dfiファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::writeDfi(const char* filename)
{
    int ierror;
    UdmError_t ret;
    FILE* fp = NULL;
    char dirname[256] = {0x00};

    if (filename == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_INVALID_PARAMETERS, "filename is null.");
    }
    udm_get_dirname(filename, dirname);
    if (strlen(dirname) > 0) {
        if (udm_make_directories(dirname) != 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", dirname);
        }
    }

    if( !(fp = fopen(filename, "w")) ) {
        return UDM_ERROR_WRITE_INDEXFILE_OPENERROR;
    }

    ierror = 0;
    // ヘッダー出力
    this->writeText(fp, 0, UDM_CGNS_TEXT_UDMINFO_DESCRIPTION);

    // FileInfo
    if (this->fileinfo != NULL) {
        ret = this->fileinfo->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    // FilePath
    if (this->filepath != NULL) {
        ret = this->filepath->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    // UnitList
    if (this->unitlist != NULL) {
        ret = this->unitlist->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    // TimeSlice
    if (this->timeslice != NULL) {
        ret = this->timeslice->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    // FlowSolutionList
    if (this->flowsolutionlist != NULL) {
        ret = this->flowsolutionlist->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }
    fclose(fp);

    // proc.dfiの出力
    std::string procdfi_name;
    if (this->getWriteProcDfiPath(procdfi_name) == UDM_OK) {
        ret = this->writeProcDfi(procdfi_name.c_str());
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    if (ierror > 0) {
        return UDM_ERROR;
    }
    return UDM_OK;
}


/**
 * ローカルファイルにproc.dfiファイルを書き込む.
 * 出力proc.dfiファイル名の指定がない場合は、FilePath/Process要素(proc.dfi設定)により出力を行う.
 * @param filename        出力proc.dfiファイル
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::writeProcDfi(const char* filename)
{
    std::string write_filename;
    int ierror;
    UdmError_t ret;
    FILE* fp = NULL;
    char dirname[256] = {0x00};

    if (filename == NULL || strlen(filename) <= 0) {
        // proc.dfiの読込み
        if (this->getWriteProcDfiPath(write_filename) != UDM_OK) {
            return UDM_ERROR_WRITE_PROCFILE_OPENERROR;
        }
    }
    else {
        write_filename = filename;
    }

    udm_get_dirname(write_filename.c_str(), dirname);
    if (strlen(dirname) > 0) {
        if (udm_make_directories(dirname) != 0) {
            return UDM_ERROR_HANDLER(UDM_ERROR, "make directory(%s).", dirname);
        }
    }

    if( !(fp = fopen(write_filename.c_str(), "w")) ) {
        return UDM_ERROR_WRITE_PROCFILE_OPENERROR;
    }

    ierror = 0;

    // ヘッダー出力
    this->writeText(fp, 0, UDM_CGNS_TEXT_UDMINFO_DESCRIPTION);

    // Domain
    if (this->domain != NULL) {
        ret = this->domain->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    // MPI
    if (this->mpi != NULL) {
        ret = this->mpi->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    // Process
    if (this->process != NULL) {
        ret = this->process->write(fp, 0);
        if (ret != UDM_OK) {
            ierror++;
        }
    }

    fclose(fp);

    if (ierror > 0) {
        return UDM_ERROR;
    }
    return UDM_OK;
}


/**
 * index.dfi/FileInfoデータを取得する.
 * @return        index.dfi/FileInfoデータ
 */
UdmFileInfoConfig* UdmDfiConfig::getFileinfoConfig() const
{
    return this->fileinfo;
}

/**
 * index.dfi/FilePathデータを取得する.
 * @return    index.dfi/FilePathデータ
 */
UdmFilePathConfig* UdmDfiConfig::getFilepathConfig() const
{
    return this->filepath;
}

/**
 * index.dfi/FlowSolutionデータを取得する:const.
 * @return        index.dfi/FlowSolutionデータ
 */
UdmFlowSolutionListConfig* UdmDfiConfig::getFlowSolutionListConfig() const
{
    return this->flowsolutionlist;
}


/**
 * index.dfi/TimeSliceデータを取得する.
 * @return        index.dfi/TimeSliceデータ
 */
UdmTimeSliceConfig* UdmDfiConfig::getTimesliceConfig() const
{
    return this->timeslice;
}

/**
 * index.dfi/UnitListデータを取得する.
 * @return        index.dfi/UnitListデータ
 */
UdmUnitListConfig* UdmDfiConfig::getUnitListConfig() const
{
    return this->unitlist;
}

/**
 * proc.dfi/Domainデータを取得する.
 * @return        proc.dfi/Domainデータ
 */
UdmDomainConfig* UdmDfiConfig::getDomainConfig() const
{
    return this->domain;
}

/**
 * proc.dfi/MPIデータを取得する.
 * @return        proc.dfi/MPIデータ
 */
UdmMpiConfig* UdmDfiConfig::getMpiConfig() const
{
    return this->mpi;
}

/**
 * proc.dfi/Processデータを取得する.
 * @return        proc.dfi/Processデータ
 */
UdmProcessConfig* UdmDfiConfig::getProcessConfig() const
{
    return this->process;
}

/**
 * input.dfiファイル名を取得する.
 * read(filename)に渡されたファイル名を返す.
 * @param [out] filepath        input.dfiファイル名
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getIndexDfiPath(std::string &filepath) const
{
    filepath = this->filename_input_dfi;
    return UDM_OK;
}

/**
 * input.dfiファイルのフォルダを取得する.
 * read(filename)に渡されたファイルのフォルダ名を返す.
 * @param [out] folder        input.dfiフォルダ名
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getIndexDfiFolder(std::string &folder) const
{
    if (this->filename_input_dfi.empty()) {
        return UDM_OK;
    }
    // char input_dir[this->filename_input_dfi.length()+1];
    char input_dir[512] = {0x00};
    udm_get_dirname(this->filename_input_dfi.c_str(), input_dir);
    folder = input_dir;

    return UDM_OK;
}


/**
 * 入力CGNSファイルパスを取得する.
 * @param [out]  input_path            入力CGNSファイルパス
 * @param [in] rank_no            MPIランク番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getCgnsInputFilePath(std::string& input_path, int rank_no)
{
    // 入力CGNSファイル名の作成
    std::string filename;
    if (this->fileinfo->getCgnsLinkFile(filename, rank_no) != UDM_OK) {
        return UDM_ERROR;
    }

    // input.dfiからの相対パスを返す。
    std::string input_dir;
    this->getIndexDfiFolder(input_dir);

    // ファイル名を付加する
    char path[512] = {0x00};
    udm_connect_path(input_dir.c_str(), filename.c_str(), path);
    input_path = path;

    return UDM_OK;
}


/**
 * リンクCGNSファイルパスを取得する.
 * @param [out]  file_path            リンクCGNSファイルパス
 * @param [in] rank_no            MPIランク番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getCgnsLinkFilePath(std::string& file_path, int rank_no)
{
    // 入力CGNSファイル名の作成
    std::string filename;
    if (this->fileinfo->getCgnsLinkFile(filename, rank_no) != UDM_OK) {
        return UDM_ERROR;
    }

    // 出力パスを追加する.
    this->connectOutputDirectory(filename, file_path);

    return UDM_OK;
}

/**
 * CGNS:GridCoordinatesとFlowSolutionを同時出力するCGNSファイル名を取得する.
 * CGNS:GridCoordinatesとFlowSolutionを別出力する場合は、エラーを返す。
 * @param [out] file_path        出力CGNSファイル名
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getCgnsTimeSliceFilePath(std::string& file_path, int rank_no, int timeslice_step, bool mpiexec)
{

    // 出力CGNSファイル名の作成
    std::string filename;
    if (this->fileinfo->existsFileCompositionType(Udm_AppendStep)) {
        if (this->fileinfo->generateCgnsFileName(filename, rank_no) != UDM_OK) {
            return UDM_ERROR;
        }
    }
    else {
        if (this->fileinfo->generateCgnsTimeSliceName(filename, rank_no, timeslice_step) != UDM_OK) {
            return UDM_ERROR;
        }
    }

    // 出力パスを追加する.
    this->connectOutputDirectory(filename, file_path);

    return UDM_OK;
}

/**
 * CGNS:GridCoordinatesのみ出力するCGNSファイルパスを取得する.
 * CGNSファイル構成タイプのAppendStep|EachStepとGridConstant|GridTimeSliceから判断してファイル名を作成する.
 * @param [out] file_path        出力CGNSファイルパス
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getCgnsGridFilePath(
                        std::string& file_path,
                        int rank_no,
                        int timeslice_step)
{
    // 出力CGNSファイル名の作成
    std::string filename;

    if (this->fileinfo->existsFileCompositionType(Udm_GridConstant)) {
        if (this->fileinfo->generateCgnsGridName(filename, rank_no) != UDM_OK) {
            return UDM_ERROR;
        }
    }
    else if (this->fileinfo->existsFileCompositionType(Udm_AppendStep)) {
        if (this->fileinfo->generateCgnsGridName(filename, rank_no) != UDM_OK) {
            return UDM_ERROR;
        }
    }
    else {
        if (this->fileinfo->generateCgnsTimeSliceGridName(filename, rank_no, timeslice_step) != UDM_OK) {
            return UDM_ERROR;
        }
    }

    // 出力パスを追加する.
    this->connectOutputDirectory(filename, file_path);

    return UDM_OK;
}

/**
 * CGNS:FlowSolutionのみ出力するCGNSファイルパスを取得する.
 * CGNSファイル構成タイプのAppendStep|EachStepから判断してファイル名を作成する.
 * @param [out] file_path        出力CGNSファイルパス
 * @param [in] rank_no            MPIランク番号
 * @param [in] timeslice_step    ステップ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getCgnsSolutionFilePath(
                        std::string& file_path,
                        int rank_no,
                        int timeslice_step)
{
    // 出力CGNSファイル名の作成
    std::string filename;
    if (this->fileinfo->existsFileCompositionType(Udm_AppendStep)) {
        if (this->fileinfo->generateCgnsSolutionName(filename, rank_no) != UDM_OK) {
            return UDM_ERROR;
        }
    }
    else {
        if (this->fileinfo->generateCgnsTimeSliceSolutionName(filename, rank_no, timeslice_step) != UDM_OK) {
            return UDM_ERROR;
        }
    }

    // 出力パスを追加する.
    this->connectOutputDirectory(filename, file_path);

    return UDM_OK;
}

/**
 * DFIラベルパスのパラメータの設定値を取得する.
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getDfiValue(const std::string &label_path, std::string &value) const
{
    std::vector<std::string> labels;
    std::string label;
    int count;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return UDM_ERROR;
    label = labels[0];

    // FileInfo
    if (!strcasecmp(label.c_str(), UDM_DFI_FILEINFO)) {
        if (this->fileinfo == NULL) return UDM_ERROR;
        return this->fileinfo->getDfiValue(label_path, value);
    }

    // FilePath
    if (!strcasecmp(label.c_str(), UDM_DFI_FILEPATH)) {
        if (this->filepath == NULL) return UDM_ERROR;
        return this->filepath->getDfiValue(label_path, value);
    }

    // UnitList
    if (!strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        if (this->unitlist == NULL) return UDM_ERROR;
        return this->unitlist->getDfiValue(label_path, value);
    }

    // TimeSlice
    if (!strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        if (this->timeslice == NULL) return UDM_ERROR;
        return this->timeslice->getDfiValue(label_path, value);
    }

    // FlowSolutionList
    if (!strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        if (this->flowsolutionlist == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "flowsolutionlist is null");
        }
        return this->flowsolutionlist->getDfiValue(label_path, value);
    }

    // Domain
    if (!strcasecmp(label.c_str(), UDM_DFI_DOMAIN)) {
        if (this->domain == NULL) return UDM_ERROR;
        return this->domain->getDfiValue(label_path, value);
    }

    // MPI
    if (!strcasecmp(label.c_str(), UDM_DFI_MPI)) {
        if (this->mpi == NULL) return UDM_ERROR;
        return this->mpi->getDfiValue(label_path, value);
    }

    // Process
    if (!strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        if (this->process == NULL) return UDM_ERROR;
        return this->process->getDfiValue(label_path, value);
    }

    return UDM_ERROR;
}


/**
 * DFIラベルパスのパラメータを設定する.
 * @param [in] label_path        DFIラベルパス
 * @param [in] value                設定値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::setDfiValue(const std::string &label_path, const std::string &value)
{
    std::vector<std::string> labels;
    std::string label;
    int count;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return UDM_ERROR;
    label = labels[0];

    // FileInfo
    if (!strcasecmp(label.c_str(), UDM_DFI_FILEINFO)) {
        return this->fileinfo->setDfiValue(label_path, value);
    }

    // FilePath
    if (!strcasecmp(label.c_str(), UDM_DFI_FILEPATH)) {
        return this->filepath->setDfiValue(label_path, value);
    }

    // UnitList
    if (!strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        return this->unitlist->setDfiValue(label_path, value);
    }

    // TimeSlice
    if (!strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        return this->timeslice->setDfiValue(label_path, value);
    }

    // FlowSolutionList
    if (!strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        return this->flowsolutionlist->setDfiValue(label_path, value);
    }

    // Domain
    if (!strcasecmp(label.c_str(), UDM_DFI_DOMAIN)) {
        return this->domain->setDfiValue(label_path, value);
    }

    // MPI
    if (!strcasecmp(label.c_str(), UDM_DFI_MPI)) {
        return this->mpi->setDfiValue(label_path, value);
    }

    // Process
    if (!strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        return this->process->setDfiValue(label_path, value);
    }

    return UDM_ERROR;
}

/**
 * DFIラベルパスのパラメータの設定値数を取得する.
 * 複数の設定値のあるパラメータの場合は設定値数を返す。
 * １つのみ設定値のパラメータの場合は1を返す。
 * 設定値されていないパラメータの場合は0を返す。
 * 子ノードが存在している場合は、ノード数又はパラメータ数を返す.
 * @param [in]  label_path        DFIラベルパス
 * @param [out] value                設定値
 * @return        パラメータの設定値数
 */
int UdmDfiConfig::getNumDfiValue(const std::string &label_path) const
{
    std::vector<std::string> labels;
    std::string label;
    int count;

    // ラベルの取得
    count = this->split(label_path, '/', labels);
    if (count <= 0) return UDM_ERROR;
    label = labels[0];

    // FileInfo
    if (!strcasecmp(label.c_str(), UDM_DFI_FILEINFO)) {
        if (this->fileinfo == NULL) return UDM_ERROR;
        return this->fileinfo->getNumDfiValue(label_path);
    }

    // FilePath
    if (!strcasecmp(label.c_str(), UDM_DFI_FILEPATH)) {
        if (this->filepath == NULL) return UDM_ERROR;
        return this->filepath->getNumDfiValue(label_path);
    }

    // UnitList
    if (!strcasecmp(label.c_str(), UDM_DFI_UNITLIST)) {
        if (this->unitlist == NULL) return UDM_ERROR;
        return this->unitlist->getNumDfiValue(label_path);
    }

    // TimeSlice
    if (!strcasecmp(label.c_str(), UDM_DFI_TIMESLICE)) {
        if (this->timeslice == NULL) return UDM_ERROR;
        return this->timeslice->getNumDfiValue(label_path);
    }

    // FlowSolutionList
    if (!strcasecmp(label.c_str(), UDM_DFI_FLOWSOLUTIONLIST)) {
        if (this->flowsolutionlist == NULL) {
            return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "flowsolutionlist is null");
        }
        return this->flowsolutionlist->getNumDfiValue(label_path);
    }

    // Domain
    if (!strcasecmp(label.c_str(), UDM_DFI_DOMAIN)) {
        if (this->domain == NULL) return UDM_ERROR;
        return this->domain->getNumDfiValue(label_path);
    }

    // MPI
    if (!strcasecmp(label.c_str(), UDM_DFI_MPI)) {
        if (this->mpi == NULL) return UDM_ERROR;
        return this->mpi->getNumDfiValue(label);
    }

    // Process
    if (!strcasecmp(label.c_str(), UDM_DFI_PROCESS)) {
        if (this->process == NULL) return UDM_ERROR;
        return this->process->getNumDfiValue(label);
    }

    return 0;
}


/**
 * 単位系を設定する：基準値.
 * 単位系名称が追加済みの場合は、上書きを行う.
 * 単位系名称が未設定の場合は、追加を行う.
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::setUnitList(const std::string &unit_name, const std::string &unit, double reference)
{
    return this->unitlist->setUnitConfig(unit_name, unit, reference);
}


/**
 * 単位系を設定する：基準値,差分値.
 * 単位系名称が追加済みの場合は、上書きを行う.
 * 単位系名称が未設定の場合は、追加を行う.
 * @param unit_name        単位系名称
 * @param unit            単位
 * @param reference        基準値
 * @param difference        差分値
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::setUnitList(const std::string &unit_name, const std::string &unit, double reference, double difference)
{
    return this->unitlist->setUnitConfig(unit_name, unit, reference, difference);
}

/**
 * 単位系名称リストを取得する.
 * @param [out] labels        単位系名称リスト
 * @return    単位系名称リスト数
 */
unsigned int UdmDfiConfig::getUnitNameList(std::vector<std::string> &unit_names) const
{
    if (this->unitlist == NULL) return 0;
    return this->unitlist->getUnitNameList(unit_names);
}

/**
 * 物理量名称リストを取得する.
 * @param [out] solution_names        物理量名称リスト
 * @return    物理量名称リスト数
 */
unsigned  int UdmDfiConfig::getSolutionNameList(std::vector<std::string> &solution_names) const
{
    if (this->flowsolutionlist == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "flowsolutionlist is null");
    }
    return this->flowsolutionlist->getSolutionNameList(solution_names);
}

/**
 * 物理量情報を設定する.
 * 物理量変数名称が存在している場合は、上書きする.
 * 存在していない場合は、追加する.
 * @param solution_name        物理量変数名称
 * @param grid_location        物理量の定義位置
 * @param data_type            データ型
 * @param vector_type        ベクトル型
 * @param nvector_size        N成分数
 * @param constant_flag        物理データ固定値フラグ
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::setSolutionConfig(
                const std::string     &solution_name,
                UdmGridLocation_t    grid_location,
                UdmDataType_t        data_type,
                UdmVectorType_t        vector_type,
                int                    nvector_size,
                bool                constant_flag)
{
    if (this->flowsolutionlist == NULL) {
        return UDM_ERROR_HANDLER(UDM_ERROR_NULL_VARIABLE, "flowsolutionlist is null");
    }
    return this->flowsolutionlist->setSolutionFieldInfo(
                                solution_name,
                                grid_location,
                                data_type,
                                vector_type,
                                nvector_size,
                                constant_flag);
}


/**
 * 時系列データ(TimeSlice)を追加する.
 * @param step        ステップ数
 * @param time        ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::insertTimeSlice(UdmSize_t step, double time)
{
    return this->timeslice->setCurrentTimeSlice(step, time);
}


/**
 * 時系列データ(TimeSlice)を追加する.
 * @param step        ステップ数
 * @param time        ステップ時間
 * @param average_step        平均ステップ数
 * @param average_time        平均ステップ時間
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::insertTimeSlice(UdmSize_t step, double time, UdmSize_t average_step, double average_time)
{
    return this->timeslice->setCurrentTimeSlice(step, time, average_step, average_time);
}

/**
 * 現在実行中の時系列データ(TimeSlice)数を取得する.
 * @return            現在実行中の時系列データ(TimeSlice)数
 */
UdmSize_t UdmDfiConfig::getNumCurrentTimeSlices() const
{
    return this->timeslice->getNumCurrentTimeSlices();
}


/**
 * /Process/Rankを挿入する.
 * ランク番号が既に存在する場合は上書きする.
 * ランク番号が存在しない場合は、追加する.
 * @param rankid        ランク番号
 * @param vertex_size    プロセスの領域のノード数
 * @param cell_size        プロセスの領域のセル（要素）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::setProcessRank(int rankid, UdmSize_t vertex_size, UdmSize_t cell_size)
{
    return this->process->setRankConfig(rankid, vertex_size, cell_size);

}


/**
 * プロセス領域情報リストをクリアする.
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
void UdmDfiConfig::clearProcessRank()
{
    this->process->clearRankConfig();
    return;
}

/**
 * 自ランク番号の読込ランク番号リストを取得する.
 * proc.dfi/MPI/NumberOfRankを読込ランク数に振り分ける.
 * @param [in]  total_ranks        読込ランク数
 * @param [in]  myrank            自ランク番号
 * @param [out] input_rank_ids    読込ランク番号リスト
 * @return        読込ランク番号リスト数
 */
int UdmDfiConfig::makeIoRankTable(int total_ranks, int myrank, std::vector<int>& input_rank_ids)
{
    int i = 0, n = 0;
    UdmSize_t my_vertex_size = 0, my_cell_size = 0;
    UdmSize_t vertex_size = 0, cell_size = 0;
    int numberofrank = 0;
    int num_valid_ranks = 0;
    std::vector<int> valid_ranks;

    if (total_ranks <= 0) return 0;
    if (total_ranks <= myrank) return 0;
    if (this->getMpiConfig() == NULL) return 0;
    if (this->getProcessConfig() == NULL) return 0;
    if (this->getProcessConfig()->getNumRankConfig() == 0) return 0;

    // 出力節点（ノード）数、要素（セル）数を取得する.
    this->getProcessConfig()->getRankConfig(myrank, my_vertex_size, my_cell_size);
    // 出力プロセス数を取得する.
    numberofrank = this->getProcessConfig()->getNumRankConfig();
    // 有効プロセス数を取得する.
    for (i=0; i<numberofrank; i++) {
        this->getProcessConfig()->getRankConfig(i, vertex_size, cell_size);
        if (vertex_size > 0) {
            valid_ranks.push_back(i);
        }
    }
    num_valid_ranks = valid_ranks.size();

    if (numberofrank <= total_ranks) {
        if (numberofrank > myrank) {
            if (my_cell_size > 0) {
                input_rank_ids.push_back(myrank);
                return input_rank_ids.size();
            }
            else {
                return 0;
            }
        }
        else {
            return 0;
        }
    }

    input_rank_ids.clear();
    // 読込プロセス番号を振り分ける
    int *input_rankids = NULL;
    int alloc_size = num_valid_ranks;
    if (total_ranks > 1 && num_valid_ranks > (total_ranks-1)) {
        alloc_size = num_valid_ranks/(total_ranks-1);
    }
    else if (num_valid_ranks <= total_ranks) {
        alloc_size = 1;
    }
    input_rankids = new int[alloc_size];

    // すべてのプロセスの読込先を取得する
    for (n=0; n<total_ranks; n++) {
        memset(input_rankids, 0x00, sizeof(int)*alloc_size);
        // 順次振分
        int num_inputs = udm_divide_serialids(num_valid_ranks, total_ranks, n, input_rankids);
        // int num_inputs = udm_divide_parallelids(num_valid_ranks, total_ranks, n, input_rankids);
        if (n==myrank) {
            for (i=0; i<num_inputs; i++) {
                int id = input_rankids[i];
                input_rank_ids.push_back(valid_ranks[id]);
            }
        }
        // 入出力ランク番号テーブルを作成する.
        for (i=0; i<num_inputs; i++) {
            int id = input_rankids[i];
            int rankno = valid_ranks[id];
            // 入出力ランク番号テーブル={出力ランク番号, 入力ランク番号}
            this->io_rank_table.insert(std::make_pair(rankno, n));
        }
    }

    delete []input_rankids;
    return input_rank_ids.size();
}


/**
 * ドメイン情報：Domainを設定する.
 * @param dimmention        非構造格子モデルの次元数
 * @param vertex_size        全体の節点（ノード）数
 * @param cell_size            全体の要素（セル）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::setDomainConfig(
                        int dimmention,
                        UdmSize_t vertex_size,
                        UdmSize_t cell_size)
{
    // 非構造格子モデルの次元数
    this->getDomainConfig()->setCellDimension(dimmention);
    // 全体のノード数
    this->getDomainConfig()->setVertexSize(vertex_size);
    // 全体のセル（要素）数
    this->getDomainConfig()->setCellSize(cell_size);

    return UDM_OK;
}

/**
 * 並列情報：MPIを設定する.
 * @param num_process        MPIプロセス数
 * @param num_group            MPIグループ数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::setMpiConfig(int num_process, int num_group)
{
    this->getMpiConfig()->setNumberRank(num_process);
    this->getMpiConfig()->setNumberGroup(num_group);

    return UDM_OK;
}

/**
 * プロセス情報：Processを設定する.
 * @param num_process        MPIプロセス数
 * @param vertex_sizes        MPIプロセス毎の節点（ノード）数
 * @param cell_sizes        MPIプロセス毎の要素（セル）数
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::setProcessConfig(
                    int num_process,
                    const UdmSize_t* vertex_sizes,
                    const UdmSize_t* cell_sizes)
{
    // プロセス領域情報リストをクリアする.
    this->clearProcessRank();

    int n = 0;
    for (n=0; n<num_process; n++) {
        this->setProcessRank(n, vertex_sizes[n], cell_sizes[n]);
    }
    return UDM_OK;
}

/**
 * 出力ディレクトリを取得する.
 * @return        出力ディレクトリ
 */
const std::string& UdmDfiConfig::getOutputPath() const
{
    return this->output_path;
}

/**
 * 出力ディレクトリを設定する.
 * @param path        出力ディレクトリ
 */
void UdmDfiConfig::setOutputPath(const std::string path)
{
    this->output_path = path;
}

/**
 * ファイル名に出力ディレクトリを付加して出力パスを作成する.
 * 読込index.dfiのパス + 出力ディレクトリ + ファイル名とする.
 * ファイル名が絶対パスの場合は、出力ディレクトリは付加しない。
 * @param [in]  file_name                ファイル名
 * @param [out] output_path                    出力パス
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::connectOutputDirectory(
                        const std::string file_name,
                        std::string &connect_path) const
{
    if (file_name.empty()) {
        return UDM_ERROR;
    }

    // ファイル名が絶対パスであるかチェックする.
    if (udm_is_absolutepath(file_name.c_str())) {
        // ファイル名が絶対パスの場合は、出力ディレクトリは付加しない。
        connect_path = file_name;
        return UDM_OK;
    }

    char path[512] = {0x00};
    // 出力ディレクトリが絶対パスであるかチェックする.
    if (!this->output_path.empty()) {
        if (udm_is_absolutepath(this->output_path.c_str())) {
            // 出力ディレクトリが絶対パスであるので、出力ディレクトリ + ファイル名とする.
            udm_connect_path(this->output_path.c_str(), file_name.c_str(), path);
            connect_path = path;
            return UDM_OK;
        }
    }

    // input.dfiからの相対パスを返す。
    std::string input_dir;
    this->getIndexDfiFolder(input_dir);
    // 出力ディレクトリを付加する
    char out_dir[512] = {0x00};
    if (!this->output_path.empty()) {
        udm_connect_path(input_dir.c_str(), this->output_path.c_str(), out_dir);
        input_dir = out_dir;
    }
    // ファイル名を付加する
    udm_connect_path(input_dir.c_str(), file_name.c_str(), path);
    connect_path = path;

    return UDM_OK;
}

/**
 * DFIの出力ディレクトリを取得する.
 * 読込index.dfiのパス + 出力ディレクトリとする.
 * @param [out] dfi_outputdir               出力ディレクトリ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::getDfiOutputDirectory(std::string &dfi_outputdir) const
{
    // 出力ディレクトリが絶対パスであるかチェックする.
    if (!this->output_path.empty()) {
        if (udm_is_absolutepath(this->output_path.c_str())) {
            // 出力ディレクトリが絶対パスであるので、出力ディレクトリを返す
            dfi_outputdir = this->output_path;
            return UDM_OK;
        }
    }

    // input.dfiからの相対パスを返す。
    std::string input_dir;
    this->getIndexDfiFolder(input_dir);
    // 出力ディレクトリを付加する
    char path[512] = {0x00};
    if (!this->output_path.empty()) {
        udm_connect_path(input_dir.c_str(), this->output_path.c_str(), path);
        input_dir = path;
    }
    dfi_outputdir = input_dir;

    return UDM_OK;
}

/**
 * DFIの出力ディレクトリを取得する.
 * 読込index.dfiのパス + 出力ディレクトリとする.
 * @param [in]  file_name                ファイル名
 * @param [out] dfi_outputdir               出力ディレクトリ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::getDfiOutputDirectory(char *dfi_outputdir) const
{
    // DFI出力ディレクトリ
    std::string dfi_path;
    if (this->getDfiOutputDirectory(dfi_path) != UDM_OK) {
        return UDM_ERROR;
    }

    strcpy(dfi_outputdir, dfi_path.c_str());

    return UDM_OK;
}

/**
 * CGNSファイルの出力ディレクトリを取得する.
 * 読込index.dfiのパス + 出力ディレクトリ + フィールドディレクトリ
 * @param [out] out_path        CGNSファイルの出力ディレクトリ
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::getCgnsOutputDirectory(std::string &out_path) const
{
    // DFI出力ディレクトリ
    std::string dfi_path;
    if (this->getDfiOutputDirectory(dfi_path) != UDM_OK) {
        return UDM_ERROR;
    }

    // フィールドディレクトリ
    std::string field_dir;
    this->getFileinfoConfig()->getDirectoryPath(field_dir);
    if (field_dir.empty()) {
        out_path = dfi_path;
        return UDM_OK;
    }
    else if (udm_is_absolutepath(field_dir.c_str())) {
        // 絶対パスであるので、フィールドディレクトリを返す
        out_path = field_dir;
        return UDM_OK;
    }

    // 読込index.dfiのパス + 出力ディレクトリ + フィールドディレクトリ
    char path[512] = {0x00};
    udm_connect_path(dfi_path.c_str(), field_dir.c_str(), path);
    out_path = path;

    return UDM_OK;
}

/**
 * 時系列CGNSファイルの出力ディレクとを取得する.
 * 読込index.dfiのパス + 出力ディレクトリ + フィールドディレクトリ + %08d(=時系列ステップ番号)
 * 時刻ディレクトリ作成オプション(TimeSliceDirectory=on)の場合は、時系列ステップ番号ディレクトリを付加する.
 * 時刻ディレクトリ作成オプション(TimeSliceDirectory=off)の場合は、時系列ステップ番号ディレクトリを付加しない（getCgnsOutputDirectoryと同じ）.
 * @param [out] out_path                時系列CGNSファイルの出力ディレク
 * @param [in] timeslice_step        出力時系列ステップ番号
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmDfiConfig::getCgnsTimeSliceDirectory(std::string &out_path, UdmSize_t timeslice_step) const
{
    // CGNS出力ディレクトリを取得する.
    this->getCgnsOutputDirectory(out_path);

    // 時刻ディレクトリ作成オプションをチェックする.
    if (!this->getFileinfoConfig()->isTimeSliceDirectory()) {
        return UDM_OK;
    }

    char time_path[128] = {0x00};
    // 時刻ディレクトリを作成する
    sprintf(time_path, UDM_DFI_FORMAT_TIMESLICE, timeslice_step);

    char path[512] = {0x00};
    udm_connect_path(out_path.c_str(), time_path, path);
    out_path = path;

    return UDM_OK;
}


/**
 * 出力index.dfiファイルパスを取得する.
 * input.dfiファイルパスと/FilePath/Processのproc.dfiファイル名からproc.dfiファイルパスを取得する.
 * 未設定の場合は、index.dfiとする.
 * @param [out]  filepath        出力index.dfiファイルパス
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getWriteIndexDfiPath(std::string &filepath) const
{
    std::string indexdfi;
    if (this->getIndexDfiPath(indexdfi) != UDM_OK) {
        return UDM_ERROR;
    }
    if (indexdfi.empty()) {
        indexdfi = UDM_DFI_FILENAME_INDEX;
    }
    char filename[256] = {0x00};
    udm_get_filename(indexdfi.c_str(), filename);

    // 出力パスを追加する.
    this->connectOutputDirectory(filename, filepath);

    return UDM_OK;
}

/**
 * 出力proc.dfiファイルパスを取得する.
 * input.dfiファイルパスと/FilePath/Processのproc.dfiファイル名からproc.dfiファイルパスを取得する.
 * 未設定の場合は、proc.dfiとする.
 * @param [out]  filepath        出力proc.dfiファイルパス
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getWriteProcDfiPath(std::string &filepath) const
{
    std::string procdfi;
    if (this->filepath->getProcess(procdfi) != UDM_OK) {
        return UDM_ERROR;
    }
    if (procdfi.empty()) {
        procdfi = UDM_DFI_FILENAME_PROC;
    }

    // 絶対パスの場合は、procdfiを返す
    if (udm_is_absolutepath(procdfi.c_str())) {
        filepath = procdfi;
        return UDM_OK;
    }

    // 出力パスを追加する.
    std::string path;
    this->connectOutputDirectory(procdfi, filepath);

    return UDM_OK;
}

/**
 * 読込proc.dfiファイルパスを取得する.
 * input.dfiファイルパスと/FilePath/Processのproc.dfiファイル名からproc.dfiファイルパスを取得する.
 * proc.dfiファイル名が未設定の場合は、"proc.dfi"とする.
 * @param [out]  filepath        proc.dfiファイルパス
 * @return        エラー番号 : UDM_OK | UDM_ERROR,etc
 */
UdmError_t UdmDfiConfig::getReadProcDfiPath(std::string &filepath) const
{
    std::string procdfi;
    if (this->filepath->getProcess(procdfi) != UDM_OK) {
        return UDM_ERROR;
    }
    if (procdfi.empty()) {
        procdfi = UDM_DFI_FILENAME_PROC;
    }

    // 絶対パスの場合は、procdfiを返す
    if (udm_is_absolutepath(procdfi.c_str())) {
        filepath = procdfi;
        return UDM_OK;
    }

    // input.dfiからの相対パスを返す。
    std::string input_dir;
    this->getIndexDfiFolder(input_dir);
    char path[512] = {0x00};
    udm_connect_path(input_dir.c_str(), procdfi.c_str(), path);
    filepath = path;

    return UDM_OK;
}


/**
 * 出力ランク番号から入力ランク番号を取得する.
 * 入出力ランク番号テーブルからNxM読込を行う場合の入力ランク番号を取得する.
 * @param output_rankno        出力ランク番号
 * @return            入力ランク番号 : 存在しない場合は、-1を返す.
 */
int UdmDfiConfig::getInputRankno(int output_rankno) const
{
    if (this->io_rank_table.size() <= 0) return -1;

    std::map<int, int>::const_iterator  find_itr;
    find_itr = this->io_rank_table.find(output_rankno);
    if (find_itr == this->io_rank_table.end()) {
        return -1;
    }

    return find_itr->second;
}

/**
 * 入出力ランク番号テーブルをクリアする.
 */
void UdmDfiConfig::clearIoRankTable()
{
    this->io_rank_table.clear();
    return;
}

/**
 * 入出力ランク番号テーブルが存在するかチェックする.
 * @return        true=入出力ランク番号テーブルが存在する
 */
bool UdmDfiConfig::existsIoRankTable() const
{
    return (this->io_rank_table.size() > 0);
}

/**
 * ベースファイル名を設定する.
 * @param prefix        ベースファイル名
 */
void UdmDfiConfig::setFileInfoPrefix(const std::string& prefix)
{
    if (this->fileinfo == NULL) return;
    this->fileinfo->setPrefix(prefix);

    return;
}

} /* namespace udm */
