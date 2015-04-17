/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */

/**
 * @file UdmGlobalRankid.cpp
 * グローバルIDリストクラスのソースファイル
 */

#include "UdmBase.h"
#include "model/UdmGlobalRankid.h"
#include "model/UdmEntity.h"

namespace udm
{

/**
 * コンストラクタ
 */
UdmGlobalRankidList::UdmGlobalRankidList()
{
    this->globalid_list.clear();
};

/**
 * コンストラクタ
 * @param    size        確保サイズ
 */
UdmGlobalRankidList::UdmGlobalRankidList(UdmSize_t size)
{
    this->globalid_list.clear();
    this->reserve(size);
}

/**
 * デストラクタ
 */
UdmGlobalRankidList::~UdmGlobalRankidList()
{
    this->globalid_list.clear();
};

/**
 * グローバルIDを追加する.
 * @param globalid        グローバルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::addGlobalRankid(const UdmGlobalRankid &globalid)
{
    std::vector<UdmGlobalRankid>::const_iterator found_itr;
    bool found = this->findGlobalRankid(globalid, found_itr);
    if (found) {
        return UDM_ERROR;
    }
    if (found_itr == this->globalid_list.end()) {
        this->globalid_list.push_back(globalid);
    }
    else {
        std::vector<UdmGlobalRankid>::iterator insert_itr;
        insert_itr = this->globalid_list.begin() + (found_itr - this->globalid_list.begin());
        this->globalid_list.insert(insert_itr, globalid);
    }
    return UDM_OK;
};

/**
 * グローバルIDを追加する.
 * @param rankno        ランク番号
 * @param localid        ローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::addGlobalRankid(int rankno, UdmSize_t localid)
{
    return this->addGlobalRankid(UdmGlobalRankid(rankno, localid));
};

UdmError_t UdmGlobalRankidList::addPreviousRankInfo(int rankno, UdmSize_t localid)
{
    UdmGlobalRankid src_rank = UdmGlobalRankid(rankno, localid);

    std::vector<UdmGlobalRankid>::iterator insert_itr = this->globalid_list.end();
    bool found = true;
    // bool found = this->findGlobalRankid(globalid, found_itr);

    printf("addPreviousRankInfo : %d, %ld, %d\n", rankno, localid, this->globalid_list.size());
    if (!found) {
        if (this->globalid_list.size() <= 0) {
            insert_itr = this->globalid_list.end();
            found = false;
        }
        if (found == true) {
            UdmSize_t left = 0;
            UdmSize_t right = this->globalid_list.size();
            UdmSize_t mid;
            UdmSize_t size = this->globalid_list.size();
            UdmGlobalRankid first = this->globalid_list[left];
            UdmGlobalRankid last = this->globalid_list[right-1];
            UdmGlobalRankid mid_rank;
            UdmSize_t first_id = first.localid;
            UdmSize_t last_id = last.localid;
            UdmSize_t src_id = src_rank.localid;

            if (first  > src_rank) {
                insert_itr = this->globalid_list.begin();
                found = false;
            }
            if (last < src_rank) {
                insert_itr = this->globalid_list.end();
                found = false;
            }
            if (found == true) {
                found = false;
#if 0
                // id位置で検索
                if (src_id>=first_id && src_id-first_id < size) {
                    UdmGlobalRankid list_rank = this->globalid_list[src_id-first_id];
                    insert_itr = this->globalid_list.begin() + (src_id-first_id);
                    if (list_rank == src_rank) {
                        found = true;
                    }
                    else if (list_rank > src_rank) {
                        right = std::distance(this->globalid_list.begin(), insert_itr);
                    }
                    else {
                        left = std::distance(this->globalid_list.begin(), insert_itr);
                    }
                }
#endif
                if (found == false) {
                    long long diff = 0;
                    while (left <= right) {
                        mid = (left + right) / 2;
                        if (mid >= size) {
                            break;
                        }
                        mid_rank = this->globalid_list[mid];
                        diff = mid_rank.compare(src_rank);
                        if (diff == 0) {
                            insert_itr = this->globalid_list.begin() + mid;
                            found = true;
                            break;
                        }
                        else if (diff < 0) {
                            left = mid + 1;
                        }
                        else {
                            right = mid - 1;
                        }
                    }
                    if (found == false) {
                        if (diff < 0) {
                            insert_itr = this->globalid_list.begin() + mid + 1;
                        }
                        else {
                            insert_itr = this->globalid_list.begin() + mid;
                        }
                    }
                }
            }
        }
    }

//    if (found) {
//        return UDM_ERROR;
//    }
    /*
    if (insert_itr == this->globalid_list.end()) {
        this->globalid_list.push_back(src_rank);
    }
    else {
        std::vector<UdmGlobalRankid>::iterator itr;
        itr = this->globalid_list.begin() + (insert_itr - this->globalid_list.begin());
        this->globalid_list.insert(itr, src_rank);
    }
*/
    this->globalid_list.push_back(src_rank);
    return UDM_OK;
};


/**
 * グローバルIDリストを追加する.
 * @param globalids       追加グローバルIDリスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::addGlobalRankids(const UdmGlobalRankidList &globalids)
{
    if (globalids.size() <= 0) return UDM_ERROR;

    std::vector<UdmGlobalRankid>::const_iterator itr;
    for (itr=globalids.globalid_list.begin(); itr!=globalids.globalid_list.end(); itr++) {
        addGlobalRankid(*itr);
    }
    return UDM_OK;
};

/**
 * グローバルID, 関連オブジェクトを追加する.
 * @param rankno        ランク番号
 * @param localid        ローカルID
 * @param entity        関連オブジェクト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::addGlobalRankid(int rankno, UdmSize_t localid, UdmEntity* entity)
{
    return this->addGlobalRankid(UdmGlobalRankid(rankno, localid, entity));
}


/**
 * グローバルIDが存在するかチェックする.
 * @param globalid        チェック対象グローバルID
 * @return            true=チェック対象グローバルIDが存在する.
 */
bool UdmGlobalRankidList::existsGlobalRankid(const UdmGlobalRankid &globalid) const
{
    std::vector<UdmGlobalRankid>::const_iterator insert_itr;
    bool found = this->findGlobalRankid(globalid, insert_itr);
    return found;
};

/**
 * グローバルIDが存在するかチェックする.
 * @param rankno        ランク番号
 * @param localid        ローカルID
 * @return            true=チェック対象グローバルIDが存在する.
 */
bool UdmGlobalRankidList::existsGlobalRankid(int rankno, UdmSize_t localid) const
{
    return this->existsGlobalRankid(UdmGlobalRankid(rankno, localid));
};

/**
 * グローバルIDリストをクリアする.
 */
void UdmGlobalRankidList::clear()
{
    this->globalid_list.clear();
};

/**
 * グローバルIDリストからグローバルIDを削除を行う.
 * @param globalid        削除グローバルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::removeGlobalRankid(const UdmGlobalRankid &globalid)
{
    std::vector<UdmGlobalRankid>::const_iterator found_itr;
    bool found = this->findGlobalRankid(globalid, found_itr);
    if (!found) {
        return UDM_ERROR;
    }

    std::vector<UdmGlobalRankid>::iterator insert_itr;
    insert_itr = this->globalid_list.begin() + (found_itr - this->globalid_list.begin());
    this->globalid_list.erase(insert_itr);

    return UDM_OK;
}

/**
 * グローバルIDリストからグローバルIDを削除を行う.
 * @param rankno        ランク番号
 * @param localid        ローカルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::removeGlobalRankid(int rankno, UdmSize_t localid)
{
    return removeGlobalRankid(UdmGlobalRankid(rankno, localid));
};

/**
 * グローバルID数を取得する.
 * @return        グローバルID数
 */
UdmSize_t UdmGlobalRankidList::getNumGlobalRankids() const
{
    return this->globalid_list.size();
};

/**
 * グローバルID数を取得する.
 * @return        グローバルID数
 */
UdmSize_t UdmGlobalRankidList::size() const
{
    return this->getNumGlobalRankids();
};


/**
 * グローバルIDを取得する.
 * @param id        id  : 1～getNumGlobalids()
 * @param [out] rankno      ランク番号
 * @param [out] localid      ID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::getGlobalRankid(UdmSize_t id, int &rankno, UdmSize_t &localid) const
{
    if (id <= 0) return UDM_ERROR;
    if (id > this->getNumGlobalRankids()) return UDM_ERROR;

    rankno = this->globalid_list[id-1].rankno;
    localid = this->globalid_list[id-1].localid;

    return UDM_OK;
};

/**
 * グローバルIDを取得する.
 * @param [in] id        id  : 1～getNumGlobalids()
 * @param [out] globalid      グローバルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::getGlobalRankid(UdmSize_t id, UdmGlobalRankid &globalid) const
{
    if (id <= 0) return UDM_ERROR;
    if (id > this->getNumGlobalRankids()) return UDM_ERROR;

    globalid = this->globalid_list[id-1];
    return UDM_OK;
};

/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGlobalRankidList::serialize(UdmSerializeArchive& archive) const
{
    UdmSize_t n = 0, num_globalids = 0;
    int rankno;
    UdmSize_t localid;

    // グローバルIDリスト
    num_globalids = this->getNumGlobalRankids();
    archive << num_globalids;
    for (n=1; n<=num_globalids; n++) {
        this->getGlobalRankid(n, rankno, localid);
        archive << rankno << localid;
    }

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGlobalRankidList::deserialize(UdmSerializeArchive& archive)
{
    UdmSize_t n = 0, num_globalids = 0;
    int rankno;
    UdmSize_t localid;

    // ストリームステータスチェック
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }

    // グローバルIDリスト
    archive >> num_globalids;
    for (n=1; n<=num_globalids; n++) {
        archive >> rankno;
        archive >> localid;
        this->addGlobalRankid(rankno, localid);
    }

    return archive;
}


/**
 * 不正データを削除する.
 * @param  invalid_rankno        削除ランク番号
 */
void UdmGlobalRankidList::eraseInvalidGlobalRankids(int invalid_rankno)
{
/*************************
    std::vector<UdmGlobalRankid>::reverse_iterator rev_itr;
    for (rev_itr=this->globalid_list.rbegin(); rev_itr!=this->globalid_list.rend(); rev_itr++) {
        if ((*rev_itr).localid <= 0 || (*rev_itr).rankno < 0) {
            // 不正グローバルIDの削除
            this->globalid_list.erase( --(rev_itr.base()));
            continue;
        }
        if ((*rev_itr).rankno == invalid_rankno) {
            // MPI接続情報の削除
            this->globalid_list.erase( --(rev_itr.base()));
            continue;
        }
    }
***************************/

    std::vector<UdmGlobalRankid>::iterator itr;
    for (itr=this->globalid_list.begin(); itr!=this->globalid_list.end(); ) {
        if ((*itr).localid <= 0 || (*itr).rankno < 0) {
            // 不正グローバルIDの削除
            itr = this->globalid_list.erase( itr);
            continue;
        }
        if ((*itr).rankno == invalid_rankno) {
            // MPI接続情報の削除
            itr = this->globalid_list.erase( itr );
            continue;
        }
        itr++;
    }
    return;
}

/**
 * グローバルIDを更新する
 * @param old_rankno        以前のランク番号
 * @param old_localid        以前のID
 * @param new_rankno        新しいランク番号
 * @param new_localid        新しいID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::updateGlobalRankid(
        int old_rankno, UdmSize_t old_localid,
        int new_rankno, UdmSize_t new_localid)
{
    std::vector<UdmGlobalRankid>::iterator itr;
    for (itr=this->globalid_list.begin(); itr!=this->globalid_list.end(); itr++) {
        if (itr->equals(old_rankno, old_localid)) {
            if (this->existsGlobalRankid(new_rankno, new_localid)) {
                // 存在するので更新しない。削除する
                this->globalid_list.erase(itr);
                return UDM_OK;
            }
            itr->rankno = new_rankno;
            itr->localid = new_localid;
            return UDM_OK;
        }
    }
    return UDM_ERROR;
};

/**
 * グローバルIDリストをコピーする
 * @param src        コピー元グローバルIDリスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidList::cloneGlobalRankidList(const UdmGlobalRankidList& src)
{
    // グローバルIDリストクリア
    this->clear();

    std::vector<UdmGlobalRankid>::const_iterator itr;
    for (itr=src.globalid_list.begin(); itr!=src.globalid_list.end(); itr++) {
        UdmGlobalRankid info = (*itr);
        this->addGlobalRankid(info.rankno, info.localid);
    }

    return UDM_OK;
}

/**
 * グローバルIDリストを文字列として出力する.
 * @param buf        出力文字列
 */
void UdmGlobalRankidList::toString(std::string &buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;
    std::string info_buf;
    std::vector<UdmGlobalRankid>::const_iterator itr;
    for (itr=this->globalid_list.begin(); itr!=this->globalid_list.end(); itr++) {
        UdmGlobalRankid info = (*itr);
        info_buf.clear();
        info.toString(info_buf);
        if (itr != this->globalid_list.begin()) {
            stream << ", ";
        }
        stream << info_buf;
    }

    buf += stream.str();
#endif
    return;
}


/**
 * グローバルIDリストから一致するグローバルIDを検索する.
 * 昇順に並んでいるグローバルIDリストから一致するイテレータを返す。
 * 一致するグローバルIDが存在しない場合は、挿入位置のイテレータを返す。
 * @param [in]  src                検索グローバルID
 * @param [out] insert_itr        一致、挿入位置のイテレータ
 * @return        true=一致するグローバルIDが存在する.
 */
bool UdmGlobalRankidList::findGlobalRankid(
                const UdmGlobalRankid &src_rank,
                std::vector<UdmGlobalRankid>::const_iterator &insert_itr) const
{
    if (this->globalid_list.size() <= 0) {
        insert_itr = this->globalid_list.end();
        return false;
    }

    UdmSize_t left = 0;
    UdmSize_t right = this->globalid_list.size();
    UdmSize_t mid;
    UdmSize_t size = this->globalid_list.size();
    UdmGlobalRankid first = this->globalid_list[left];
    UdmGlobalRankid last = this->globalid_list[right-1];
    UdmGlobalRankid mid_rank;
    UdmSize_t first_id = first.localid;
    UdmSize_t last_id = last.localid;
    UdmSize_t src_id = src_rank.localid;

    if (first  > src_rank) {
        insert_itr = this->globalid_list.begin();
        return false;
    }
    if (last < src_rank) {
        insert_itr = this->globalid_list.end();
        return false;
    }

    // id位置で検索
    if (src_id>=first_id && src_id-first_id < size) {
        UdmGlobalRankid list_rank = this->globalid_list[src_id-first_id];
        insert_itr = this->globalid_list.begin() + (src_id-first_id);
        if (list_rank == src_rank) {
            return true;
        }
        else if (list_rank > src_rank) {
            right = std::distance(this->globalid_list.begin(), insert_itr);
        }
        else {
            left = std::distance(this->globalid_list.begin(), insert_itr);
        }
    }
    long long diff = 0;
    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_rank = this->globalid_list[mid];
        diff = mid_rank.compare(src_rank);
        if (diff == 0) {
            insert_itr = this->globalid_list.begin() + mid;
            return true;
        }
        else if (diff < 0) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    if (diff < 0) {
        insert_itr = this->globalid_list.begin() + mid + 1;
    }
    else {
        insert_itr = this->globalid_list.begin() + mid;
    }

    return false;
};

/**
 * グローバルIDの関連オブジェクトを取得する
 * @param [in] rankno        ランク番号
 * @param [in] localid        ローカルID
 * @param [out] entity        関連オブジェクト
 * @return        true=ランク番号、ローカルIDが存在する.
 */
UdmEntity* UdmGlobalRankidList::getReferenceEntity(
                const int rankno,
                const UdmSize_t localid) const
{
    std::vector<UdmGlobalRankid>::const_iterator found_itr;
    bool found = this->findGlobalRankid(UdmGlobalRankid(rankno, localid), found_itr);
    if (!found) {
        return false;
    }
    UdmEntity* entity = found_itr->getReferenceEntity();

    return entity;
}

/**
 * グローバルID, 関連オブジェクトを削除する.
 * @param rankno        ランク番号
 * @param localid        ローカルID
 * @param entity        関連オブジェクト
 */
void UdmGlobalRankidList::removeReferenceEntity(int rankno, UdmSize_t localid, UdmEntity* entity)
{
    std::vector<UdmGlobalRankid>::const_iterator found_itr;
    bool found = this->findGlobalRankid(UdmGlobalRankid(rankno, localid), found_itr);
    if (!found) {
        return;
    }

    std::vector<UdmGlobalRankid>::const_iterator dest_itr;
    for (dest_itr=found_itr; dest_itr!=this->globalid_list.end(); dest_itr++) {
        if (!dest_itr->equals(rankno, localid)) {
            break;
        }
        UdmEntity* del_entity = dest_itr->getReferenceEntity();
        if (del_entity != entity) {
            break;
        }
    }

    std::vector<UdmGlobalRankid>::iterator erase_itr;
    for (dest_itr=dest_itr-1; dest_itr!=this->globalid_list.begin(); dest_itr--) {
        UdmEntity* del_entity = dest_itr->getReferenceEntity();
        if (dest_itr->equals(rankno, localid) && del_entity == entity) {
            erase_itr = this->globalid_list.begin() + (dest_itr - this->globalid_list.begin());
            this->globalid_list.erase(erase_itr);
        }
        else {
            break;
        }
    }

    return;
}

/**
 * グローバルIDリストの容量を確保する.
 * @param size        確保容量サイズ
 */
void UdmGlobalRankidList::reserve(UdmSize_t size)
{
    this->globalid_list.reserve(size);
    return;
}

/**
 * グローバルIDリストのグローバルIDが一致しているかチェックする.
 * @param dest_list        比較対象グローバルIDリスト
 * @return        true=一致
 */
bool UdmGlobalRankidList::equals(const UdmGlobalRankidList& dest_list) const
{
    UdmSize_t n, src_len, dest_len;

    src_len = this->getNumGlobalRankids();
    dest_len = dest_list.getNumGlobalRankids();
    if (src_len != dest_len) return false;

    for (n=1; n<=src_len; n++) {
        UdmGlobalRankid src_rankid;
        UdmGlobalRankid dest_rankid;
        this->getGlobalRankid(n, src_rankid);
        dest_list.getGlobalRankid(n, dest_rankid);
        if (src_rankid != dest_rankid) {
            return false;
        }
    }

    return true;
}


/**
 * メモリサイズを取得する.
 * @return        メモリサイズ
 */
size_t UdmGlobalRankidList::getMemSize() const
{
    UdmSize_t size = sizeof(*this);
#ifdef _DEBUG
    printf("globalid_list size=%ld [count=%d] [capacity=%d] [offset=%ld] \n",
                        sizeof(this->globalid_list),
                        this->globalid_list.size(),
                        this->globalid_list.capacity(),
                        offsetof(UdmGlobalRankidList, globalid_list));
    printf("UdmGlobalRankid size=%ld \n", sizeof(UdmGlobalRankid));
#endif
    return size;
}

/* ******************************************************** */
/* UdmGlobalRankidPairList                                  */
/* ******************************************************** */

/**
 * コンストラクタ
 */
UdmGlobalRankidPairList::UdmGlobalRankidPairList()
{
    this->pair_list.clear();
}

/**
 * コンストラクタ
 * @param    size        確保サイズ
 */
UdmGlobalRankidPairList::UdmGlobalRankidPairList(UdmSize_t size)
{
    this->pair_list.clear();
    this->reserve(size);
}

/**
 * デストラクタ
 */
UdmGlobalRankidPairList::~UdmGlobalRankidPairList()
{
    this->pair_list.clear();
}

/**
 * グローバルIDペアリストに追加する.
 * @param global_pair        追加グローバルIDペアリスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::addGlobalRankidPair(const UdmGlobalRankidPair& global_pair)
{
    std::vector<UdmGlobalRankidPair>::const_iterator found_itr;
    bool found = this->findGlobalRankidPair(global_pair, found_itr);
    if (found) {
        return UDM_ERROR;
    }
    if (found_itr == this->pair_list.end()) {
        this->pair_list.push_back(global_pair);
    }
    else {
        std::vector<UdmGlobalRankidPair>::iterator insert_itr;
        insert_itr = this->pair_list.begin() + (found_itr - this->pair_list.begin());
        this->pair_list.insert(insert_itr, global_pair);
    }
    return UDM_OK;
}


/**
 * グローバルIDペアリストに追加する.
 * @param src_rankno        自(新)ランク番号
 * @param src_localid        自(新)ID
 * @param dest_rankno        接続先(旧)ランク番号
 * @param dest_localid        接続先(旧)ID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::addGlobalRankidPair(
                int src_rankno,
                UdmSize_t src_localid,
                int dest_rankno,
                UdmSize_t dest_localid)
{
    return this->addGlobalRankidPair(
                UdmGlobalRankidPair(
                        src_rankno, src_localid,
                        dest_rankno, dest_localid));
}

/**
 * グローバルIDペアリストに追加する.
 * @param src_globalid        自(新)グローバルID
 * @param dest_globalid        接続先(旧)グローバルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::addGlobalRankidPair(
                const UdmGlobalRankid& src_globalid,
                const UdmGlobalRankid& dest_globalid)
{
    return this->addGlobalRankidPair(UdmGlobalRankidPair(src_globalid, dest_globalid));
}

/**
 * グローバルIDペアが存在するかチェックする.
 * @param global_pair        チェックグローバルIDペア
 * @return            true=存在する.
 */
bool UdmGlobalRankidPairList::existsGlobalRankidPair(
        const UdmGlobalRankidPair& global_pair) const
{
    std::vector<UdmGlobalRankidPair>::const_iterator found_itr;
    bool found = this->findGlobalRankidPair(global_pair, found_itr);
    return found;
}

/**
 * グローバルIDペアが存在するかチェックする.
 * @param src_rankno        自(新)ランク番号
 * @param src_localid        自(新)ID
 * @param dest_rankno        接続先(旧)ランク番号
 * @param dest_localid        接続先(旧)ID
 * @return            true=存在する.
 */
bool UdmGlobalRankidPairList::existsGlobalRankidPair(
                int src_rankno, UdmSize_t src_localid,
                int dest_rankno, UdmSize_t dest_localid) const
{
    return this->existsGlobalRankidPair(
                UdmGlobalRankidPair(
                        src_rankno, src_localid,
                        dest_rankno, dest_localid));
}

/**
 * グローバルIDペアが存在するかチェックする.
 * @param src_globalid        自(新)グローバルID
 * @param dest_globalid        接続先(旧)グローバルID
 * @return            true=存在する.
 */
bool UdmGlobalRankidPairList::existsGlobalRankidPair(
                const UdmGlobalRankid& src_globalid,
                const UdmGlobalRankid& dest_globalid) const
{
    return this->existsGlobalRankidPair(
                UdmGlobalRankidPair(src_globalid, dest_globalid));
}

/**
 * グローバルIDペアリストをクリアする.
 */
void UdmGlobalRankidPairList::clear()
{
    this->pair_list.clear();
}

/**
 * グローバルIDペアを削除する.
 * @param global_pair        グローバルIDペア
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::removeGlobalRankidPair(
                const UdmGlobalRankidPair& global_pair)
{
    std::vector<UdmGlobalRankidPair>::const_iterator found_itr;
    bool found = this->findGlobalRankidPair(global_pair, found_itr);
    if (!found) {
        return UDM_ERROR;
    }

    std::vector<UdmGlobalRankidPair>::iterator insert_itr;
    insert_itr = this->pair_list.begin() + (found_itr - this->pair_list.begin());
    this->pair_list.erase(insert_itr);

    return UDM_OK;
}

/**
 * グローバルIDペアを削除する.
 * @param src_rankno        自(新)ランク番号
 * @param src_localid        自(新)ID
 * @param dest_rankno        接続先(旧)ランク番号
 * @param dest_localid        接続先(旧)ID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::removeGlobalRankidPair(
                int src_rankno, UdmSize_t src_localid,
                int dest_rankno, UdmSize_t dest_localid)
{
    return this->removeGlobalRankidPair(
                UdmGlobalRankidPair(
                        src_rankno, src_localid,
                        dest_rankno, dest_localid));
}

/**
 * グローバルIDペアを削除する.
 * @param src_globalid        自(新)グローバルID
 * @param dest_globalid        接続先(旧)グローバルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::removeGlobalRankidPair(
                const UdmGlobalRankid& src_globalid,
                const UdmGlobalRankid& dest_globalid)
{
    return this->removeGlobalRankidPair(
                UdmGlobalRankidPair(src_globalid, dest_globalid));
}

/**
 * グローバルIDペアリストサイズを取得する.
 * @return        グローバルIDペアリストサイズ
 */
UdmSize_t UdmGlobalRankidPairList::getNumGlobalRankidPairs() const
{
    return this->size();
}

/**
 * グローバルIDペアリストサイズを取得する.
 * @return        グローバルIDペアリストサイズ
 */
UdmSize_t UdmGlobalRankidPairList::size() const
{
    return this->pair_list.size();
}

/**
 * グローバルIDペアを取得する.
 * @param id                グローバルIDペアリストID（１～）
 * @param [out] src_rankno        自(新)ランク番号
 * @param [out] src_localid        自(新)ID
 * @param [out] dest_rankno        接続先(旧)ランク番号
 * @param [out] dest_localid        接続先(旧)ID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::getGlobalRankidPair(
                UdmSize_t id,
                int& src_rankno, UdmSize_t& src_localid,
                int& dest_rankno, UdmSize_t& dest_localid) const
{
    if (id <= 0) return UDM_ERROR;
    if (id > this->getNumGlobalRankidPairs()) return UDM_ERROR;

    this->pair_list[id-1].getSrcGlobalRankid().getGlobalids(src_rankno, src_localid);
    this->pair_list[id-1].getDestGlobalRankid().getGlobalids(dest_rankno, dest_localid);

    return UDM_OK;
}

/**
 * グローバルIDペアを取得する.
 * @param id                グローバルIDペアリストID（１～）
 * @param [out] global_pair        グローバルIDペア
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::getGlobalRankidPair(
                UdmSize_t id,
                UdmGlobalRankidPair& global_pair) const
{
    if (id <= 0) return UDM_ERROR;
    if (id > this->getNumGlobalRankidPairs()) return UDM_ERROR;

    global_pair = this->pair_list[id-1];

    return UDM_OK;
}

/**
 * グローバルIDペアを取得する.
 * @param id                グローバルIDペアリストID（１～）
 * @param [out] src_globalid        自(新)グローバルID
 * @param [out] dest_globalid        接続先(旧)グローバルID
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::getGlobalRankidPair(
                UdmSize_t id,
                UdmGlobalRankid &src_globalid,
                UdmGlobalRankid &dest_globalid) const
{
    if (id <= 0) return UDM_ERROR;
    if (id > this->getNumGlobalRankidPairs()) return UDM_ERROR;

    src_globalid = this->pair_list[id-1].getSrcGlobalRankid();
    dest_globalid = this->pair_list[id-1].getDestGlobalRankid();

    return UDM_OK;
}

/**
 * シリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGlobalRankidPairList::serialize(
                UdmSerializeArchive& archive) const
{
    UdmSize_t n = 0, num_pairs = 0;
    int src_rankno;
    UdmSize_t src_localid;
    int dest_rankno;
    UdmSize_t dest_localid;

    // グローバルIDペアリスト
    num_pairs = this->getNumGlobalRankidPairs();
    archive << num_pairs;
    for (n=1; n<=num_pairs; n++) {
        this->getGlobalRankidPair(n, src_rankno, src_localid, dest_rankno, dest_localid);
        archive << src_rankno << src_localid;
        archive << dest_rankno << dest_localid;
    }

    return archive;
}

/**
 * デシリアライズを行う.
 * @param archive        シリアライズ・デシリアライズクラス
 */
UdmSerializeArchive& UdmGlobalRankidPairList::deserialize(
                UdmSerializeArchive& archive)
{
    UdmSize_t n = 0, num_pairs = 0;
    int src_rankno;
    UdmSize_t src_localid;
    int dest_rankno;
    UdmSize_t dest_localid;

    // ストリームステータスチェック
    if (archive.rdstate() != std::ios_base::goodbit) {
        UDM_ERROR_HANDLER(UDM_ERROR_DESERIALIZE, "rdstate is not goodbit.");
        return archive;
    }

    // グローバルIDリスト
    archive >> num_pairs;
    for (n=1; n<=num_pairs; n++) {
        archive >> src_rankno;
        archive >> src_localid;
        archive >> dest_rankno;
        archive >> dest_localid;
        this->addGlobalRankidPair(src_rankno, src_localid, dest_rankno, dest_localid);
    }

    return archive;
}

/**
 * グローバルIDペアリストを文字列として出力する.
 * @param buf        出力文字列
 */
void UdmGlobalRankidPairList::toString(std::string& buf) const
{
#ifdef _DEBUG_TRACE
    std::stringstream stream;
    std::string info_buf;
    std::vector<UdmGlobalRankidPair>::const_iterator itr;
    for (itr=this->pair_list.begin(); itr!=this->pair_list.end(); itr++) {
        UdmGlobalRankidPair pair = (*itr);
        info_buf.clear();
        pair.toString(info_buf);
        if (itr != this->pair_list.begin()) {
            stream << ", ";
        }
        stream << info_buf;
    }

    buf += stream.str();
#endif
    return;
}

/**
 * グローバルIDペアリストから一致するグローバルIDを検索する.
 * 昇順に並んでいるグローバルIDペアリストから一致するイテレータを返す。
 * 一致するグローバルIDペアが存在しない場合は、挿入位置のイテレータを返す。
 * @param [in]  src_pair                検索グローバルIDペア
 * @param [out] insert_itr        一致、挿入位置のイテレータ
 * @return        true=一致するグローバルIDペアが存在する.
 */
bool UdmGlobalRankidPairList::findGlobalRankidPair(
                const UdmGlobalRankidPair& src_pair,
                std::vector<UdmGlobalRankidPair>::const_iterator& insert_itr) const
{
    if (this->pair_list.size() <= 0) {
        insert_itr = this->pair_list.end();
        return false;
    }

    UdmSize_t left = 0;
    UdmSize_t right = this->pair_list.size();
    UdmSize_t mid;
    UdmSize_t size = this->pair_list.size();
    UdmGlobalRankidPair first = this->pair_list[left];
    UdmGlobalRankidPair last = this->pair_list[right-1];
    UdmGlobalRankidPair mid_pair;
    UdmSize_t first_id = first.getSrcLocalid();
    UdmSize_t last_id = last.getSrcLocalid();
    UdmSize_t src_id = src_pair.getSrcLocalid();

    if (first  > src_pair) {
        insert_itr = this->pair_list.begin();
        return false;
    }
    if (last < src_pair) {
        insert_itr = this->pair_list.end();
        return false;
    }

    // id位置で検索
    if (src_id>=first_id && src_id-first_id < size) {
        UdmGlobalRankidPair list_pair = this->pair_list[src_id-first_id];
        insert_itr = this->pair_list.begin() + (src_id-first_id);
        if (list_pair == src_pair) {
            return true;
        }
        else if (list_pair > src_pair) {
            right = std::distance(this->pair_list.begin(), insert_itr);
        }
        else {
            left = std::distance(this->pair_list.begin(), insert_itr);
        }
    }
    long long diff = 0;
    while (left <= right) {
        mid = (left + right) / 2;
        if (mid >= size) {
            break;
        }
        mid_pair = this->pair_list[mid];
        diff = mid_pair.compare(src_pair);
        if (diff == 0) {
            insert_itr = this->pair_list.begin() + mid;
            return true;
        }
        else if (diff < 0) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    if (diff < 0) {
        insert_itr = this->pair_list.begin() + mid + 1;
    }
    else {
        insert_itr = this->pair_list.begin() + mid;
    }

    return false;
}

/**
 * グローバルIDペアリストの開始イテレータを取得する.
 * @return        グローバルIDペアリスト開始イテレータ
 */
std::vector<UdmGlobalRankidPair>::iterator
UdmGlobalRankidPairList::begin()
{
    return this->pair_list.begin();
}

/**
 * グローバルIDペアリストの終了イテレータを取得する.
 * @return        グローバルIDペアリスト終了イテレータ
 */
std::vector<UdmGlobalRankidPair>::iterator
UdmGlobalRankidPairList::end()
{
    return this->pair_list.end();
}

/**
 * グローバルIDペアリストを追加する.
 * @param add_list        追加グローバルIDペアリスト
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::addGlobalRankidPairs(
                const UdmGlobalRankidPairList& add_list)
{
    if (add_list.size() <= 0) return UDM_ERROR;

    UdmSize_t n;
    UdmSize_t add_size = add_list.size();
    for (n=1; n<=add_size; n++) {
        UdmGlobalRankidPair pair;
        add_list.getGlobalRankidPair(n, pair);
        this->addGlobalRankidPair(pair);
    }
    return UDM_OK;
}

/**
 * 末尾に追加する。
 * 重複チェックは行わない。
 * @param global_pair        追加グローバルIDペア
 * @return        エラー番号 : UDM_OK | UDM_ERROR
 */
UdmError_t UdmGlobalRankidPairList::pushbackGlobalRankidPair(
                    const UdmGlobalRankidPair& global_pair)
{
    this->pair_list.push_back(global_pair);
    return UDM_OK;
}

/**
 * グローバルIDリストのMPI通信サイズを算出する
 * @return        MPI通信サイズ（byte)
 */
UdmSize_t UdmGlobalRankidPairList::getCommSize() const
{
    UdmSize_t size= this->getNumGlobalRankidPairs();
    // 新,自プロセス:ランク番号+ID + 旧,接続先:ランク番号+ID
    return size * (sizeof(int) + sizeof(UdmSize_t)) * 2;
}


/**
 * グローバルIDリストの容量を確保する.
 * @param size        確保容量サイズ
 */
void UdmGlobalRankidPairList::reserve(UdmSize_t size)
{
    this->pair_list.reserve(size);
    return;
}

} /* namespace udm */
