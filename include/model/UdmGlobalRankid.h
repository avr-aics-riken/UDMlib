/*
 * UDMlib - Unstructured Data Management Library
 *
 * Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
 * All rights reserved.
 *
 */
#ifndef _UDMGLOBALRANKID_H_
#define _UDMGLOBALRANKID_H_

/**
 * @file UdmGlobalRankid.h
 * グローバルIDクラスのヘッダファイル
 */

#include "UdmBase.h"
#include "utils/UdmSerialization.h"

namespace udm
{

class UdmEntity;

/**
 * グローバルIDクラス.
 * ランク番号とローカルIDを対として持つ。
 */
class UdmGlobalRankid
{
public:
    int                rankno;            ///< ランク番号（０～）
    UdmSize_t         localid;        ///< ローカルID (１～）
    UdmEntity *ref_entity;                ///< 関連オブジェクト

public:
    /**
     * コンストラクタ
     */
    UdmGlobalRankid()
    {
        this->rankno = -1;
        this->localid = 0;
    };

    /**
     * コンストラクタ
     * @param rankno        ランク番号（０～）
     * @param localid        ローカルID (１～）
     */
    UdmGlobalRankid(int rankno, UdmSize_t localid)
    {
        this->rankno = rankno;
        this->localid = localid;
    };

    /**
     * コンストラクタ
     * @param rankno        ランク番号（０～）
     * @param localid        ローカルID (１～）
     * @param entity;        関連オブジェクト
     */
    UdmGlobalRankid(int rankno, UdmSize_t localid, UdmEntity *entity)
    {
        this->rankno = rankno;
        this->localid = localid;
        this->ref_entity = entity;
    };

    /**
     * デストラクタ
     */
    virtual ~UdmGlobalRankid() {};

    /**
     * ランク番号とローカルIDが同じであるかチェックする.
     * @param rankno        ランク番号
     * @param localid        ローカルID
     * @return        true=ランク番号とローカルIDが同じ
     */
    bool equals(int rankno, UdmSize_t localid) const
    {
        if (this->rankno != rankno) return false;
        if (this->localid != localid) return false;
        return true;
    };

    /**
     * ランク番号とローカルIDが同じであるかチェックする.
     * @param dest        比較対象グローバルID
     * @return        true=ランク番号とローカルIDが同じ
     */
    bool equals(const UdmGlobalRankid &dest) const
    {
        if (this->rankno != dest.rankno) return false;
        if (this->localid != dest.localid) return false;
        return true;
    };

    /**
     * ランク番号とローカルIDがの大小関係をチェックする.
     * @param dest            比較対象グローバルID
     * @return            -1=thisが小さい,0=同じ,+1=thisが大きい
     */
    long long compare(const UdmGlobalRankid &dest) const
    {

        // 差分
        int diff_rank =  this->rankno - dest.rankno;
        long long diff_id =  (unsigned long long)this->localid - (unsigned long long)dest.localid;

        // ランク番号比較
        if (diff_rank != 0) return diff_rank;
        // ID比較
        if (diff_id != 0) return diff_id;

        return 0;
    };

    /**
     * 等値比較オペレータ
     * @param dest        比較対象グローバルID
     * @return        true=等しい
     */
    bool operator == (const UdmGlobalRankid& dest) const
    {
        return (this->equals(dest.rankno, dest.localid));
    };

    /**
     * 不等値比較オペレータ
     * @param dest        比較対象グローバルID
     * @return        true=等しい
     */
    bool operator != (const UdmGlobalRankid& dest) const
    {
        return !(this->equals(dest.rankno, dest.localid));
    };

    /**
     * 大小比較オペレータ
     * @param dest        比較対象グローバルID
     * @return        true=大きい
     */
    bool operator > (const UdmGlobalRankid& dest) const
    {
        return (this->compare(dest) > 0);
    };

    /**
     * 大小比較オペレータ
     * @param dest        比較対象グローバルID
     * @return        true=小さい
     */
    bool operator < (const UdmGlobalRankid& dest) const
    {
        return (this->compare(dest) < 0);
    };

    /**
     * グローバルID（ランク番号とローカルID）を設定する.
     * @param rankno        ランク番号
     * @param localid        ローカルID
     */
    void setGlobalids(int rankno, UdmSize_t localid)
    {
        this->rankno = rankno;
        this->localid = localid;
    }

    /**
     * グローバルID（ランク番号とローカルID）を取得する.
     * @param [out] rankno        ランク番号
     * @param [out] localid        ローカルID
     */
    void getGlobalids(int &rankno, UdmSize_t &localid) const
    {
        rankno = this->rankno;
        localid = this->localid;
    }

    /**
     * グローバルIDを文字列として出力する.
     * @param buf        出力文字列
     */
    void toString(std::string &buf) const
    {
        char stream[64] = {0x00};
        sprintf(stream, "%d[%d]", this->localid, this->rankno);
        buf += std::string(stream);
        return;
    }

    /**
     * 関連オブジェクトを取得する.
     * @return        関連オブジェクト
     */
    UdmEntity *getReferenceEntity() const
    {
        return this->ref_entity;
    }

    /**
     * 関連オブジェクトを設定する.
     * @param entity        関連オブジェクト
     */
    void setReferenceEntity(UdmEntity * entity)
    {
        this->ref_entity = entity;
    }
};


/**
 * グローバルIDペアクラス.
 * グローバルID（ランク番号とローカルID）を新旧、this-mpiの２つ持つクラス
 */
class UdmGlobalRankidPair
{
private:
    UdmGlobalRankid src_globalid;            ///< 自プロセスグローバルID,新グローバルID
    UdmGlobalRankid dest_globalid;            ///< 接続先グローバルID,旧グローバルID

public:

    /**
     * コンストラクタ
     */
    UdmGlobalRankidPair() { };

    /**
     * コンストラクタ
     * @param src        自プロセスグローバルID,新グローバルID
     * @param dest       接続先グローバルID,旧グローバルID
     */
    UdmGlobalRankidPair(UdmGlobalRankid src, UdmGlobalRankid dest)
    {
        this->src_globalid = src;
        this->dest_globalid = dest;
    };


    /**
     * コンストラクタ
     * @param src_rankno        新,自プロセス:ランク番号
     * @param src_localid        新,自プロセス:ローカルID
     * @param dest_rankno        旧,接続先:ランク番号
     * @param dest_localid        旧,接続先:ローカルID
     */
    UdmGlobalRankidPair(int src_rankno, UdmSize_t src_localid, int dest_rankno, UdmSize_t dest_localid)
    {
        this->src_globalid.setGlobalids(src_rankno, src_localid);
        this->dest_globalid.setGlobalids(dest_rankno, dest_localid);
    };

    /**
     * デストラクタ
     */
    virtual ~UdmGlobalRankidPair() {};

    /**
     * src/destのランク番号とローカルIDが同じであるかチェックする.
     * @param dest_pair        比較対象グローバルID
     * @return        true=グローバルIDがsrc/dest共同じ
     */
    bool equals(const UdmGlobalRankidPair& dest_pair) const
    {
        if (this->src_globalid != dest_pair.src_globalid) return false;
        if (this->dest_globalid != dest_pair.dest_globalid) return false;
        return true;
    };

    /**
     * ランク番号とローカルIDが新,自プロセスグローバルIDと同じであるかチェックする.
     * @param rankno        ランク番号
     * @param localid        ローカルID
     * @return        true=新,自プロセスグローバルIDとランク番号とローカルIDが同じ
     */
    bool equalsSrc(int rankno, UdmSize_t localid) const
    {
        if (this->src_globalid.equals(rankno, localid)) return true;
        return false;
    };

    /**
     * ランク番号とローカルIDが旧,接続先グローバルIDと同じであるかチェックする.
     * @param rankno        ランク番号
     * @param localid        ローカルID
     * @return        true=旧,接続先グローバルIDとランク番号とローカルIDが同じ
     */
    bool equalsDest(int rankno, UdmSize_t localid) const
    {
        if (this->dest_globalid.equals(rankno, localid)) return true;
        return false;
    };

    /**
     * 等値比較オペレータ
     * @param dest_pair        比較対象src/destグローバルID
     * @return        true=等しい
     */
    bool operator == (const UdmGlobalRankidPair& dest_pair) const
    {
        return (this->equals(dest_pair));
    };

    /**
     * 不等値比較オペレータ
     * @param dest_pair        比較対象グローバルID
     * @return        true=等しい
     */
    bool operator != (const UdmGlobalRankidPair& dest_pair) const
    {
        return !(this->equals(dest_pair));
    };


    /**
     * ランク番号とローカルIDがsrc/destのどちらかと一致しているかチェックする.
     * @param rankno        ランク番号
     * @param localid        ローカルID
     * @return        true=ランク番号とローカルIDがsrc/destのどちらかと一致
     */
    bool match(int rankno, UdmSize_t localid) const
    {
        if (this->src_globalid.equals(rankno, localid)) return true;
        if (this->dest_globalid.equals(rankno, localid)) return true;
        return false;
    };

    /**
     * 自プロセスのランク番号,ローカルIDと接続先のランク番号,ローカルIDが相互に一致しているかチェックする.
     * @param dest_pair       接続先情報
     * @return        true=自プロセスと接続先のランク番号,ローカルIDが相互に一致
     */
    bool equalsConnectivity(const UdmGlobalRankidPair &dest_pair) const
    {
        // 自プロセスと接続先が相互に一致しているか？
        if (this->src_globalid != dest_pair.dest_globalid) return false;
        if (this->dest_globalid != dest_pair.src_globalid) return false;
        return true;
    };

    /**
     * 自プロセス・新グローバルIDを取得する.
     * @return            自プロセス・新グローバルID
     */
    const UdmGlobalRankid& getSrcGlobalRankid() const
    {
        return this->src_globalid;
    }

    /**
     * 接続先ランク・旧グローバルIDを取得する.
     * @return            接続先ランク・旧グローバルID
     */
    const UdmGlobalRankid& getDestGlobalRankid() const
    {
        return this->dest_globalid;
    }

    /**
     * 自プロセス・新グローバルIDのランク番号を取得する.
     * @return            自プロセス・新グローバルIDのランク番号
     */
    int getSrcRankno() const
    {
        return this->src_globalid.rankno;
    }

    /**
     * 接続先ランク・旧グローバルIDのランク番号を取得する.
     * @return            接続先・旧グローバルIDのランク番号
     */
    int getDestRankno() const
    {
        return this->dest_globalid.rankno;
    }

    /**
     * 自プロセス・新グローバルIDのローカルIDを取得する.
     * @return            自プロセス・新グローバルIDのローカルID
     */
    UdmSize_t getSrcLocalid() const
    {
        return this->src_globalid.localid;
    }

    /**
     * 接続先ランク・旧グローバルIDのローカルIDを取得する.
     * @return            接続先・旧グローバルIDのローカルID
     */
    int getDestLocalid() const
    {
        return this->dest_globalid.localid;
    }


    /**
     * オペレータ：内部境界送受信情報が小さいかチェックする.
     * @param src        比較対象内部境界送受信情報
     * @return        true=小さい（this < src)
     */
    bool operator < (const UdmGlobalRankidPair &dest_pair)
    {
        return (compare(dest_pair) < 0);
    };

    /**
     * オペレータ：内部境界送受信情報が大きいかチェックする.
     * @param src        比較対象内部境界送受信情報
     * @return        true=大きい（this > src)
     */
    bool operator > (const UdmGlobalRankidPair &dest_pair)
    {
        return (compare(dest_pair) > 0);
    };

    /**
     * ランク番号とローカルIDがの大小関係をチェックする.
     * @param dest            比較対象グローバルIDペア
     * @return            -1=thisが小さい,0=同じ,+1=thisが大きい
     */
    long long compare(const UdmGlobalRankidPair &dest_pair) const
    {
        // 差分
        int diff_src_rank =  this->src_globalid.rankno - dest_pair.src_globalid.rankno;
        long long diff_src_id =  (unsigned long long)this->src_globalid.localid - (unsigned long long)dest_pair.src_globalid.localid;
        int diff_dest_rank =  this->dest_globalid.rankno - dest_pair.dest_globalid.rankno;
        long long diff_dest_id =  (unsigned long long)this->dest_globalid.localid - (unsigned long long)dest_pair.dest_globalid.localid;

        if (diff_src_rank != 0) return diff_src_rank;
        if (diff_src_id != 0) return diff_src_id;
        if (diff_dest_rank != 0) return diff_dest_rank;
        if (diff_dest_id != 0) return diff_dest_id;

        return 0;
    };

    /**
     * グローバルIDペアを文字列として出力する.
     * @param buf        出力文字列
     */
    void toString(std::string &buf) const
    {
        char stream[64] = {0x00};

        std::string src_buf;
        this->src_globalid.toString(src_buf);
        std::string dest_buf;
        this->dest_globalid.toString(dest_buf);
        sprintf(stream, "%s->%s", src_buf.c_str(), dest_buf.c_str());
        buf += std::string(stream);

        return;
    }
};


/**
 * グローバルIDリストクラス.
 * 重複を許可しないグローバルIDのリストクラス
 */
class UdmGlobalRankidList : public UdmISerializable
{
private:

    /**
     * グローバルIDリスト
     */
    std::vector<UdmGlobalRankid>  globalid_list;

public:
    UdmGlobalRankidList();
    UdmGlobalRankidList(UdmSize_t size);
    virtual ~UdmGlobalRankidList();
    UdmError_t addGlobalRankid(const UdmGlobalRankid &globalid);
    UdmError_t addGlobalRankid(int rankno, UdmSize_t localid);
    UdmError_t addGlobalRankid(int rankno, UdmSize_t localid, UdmEntity *entity);
    UdmError_t addGlobalRankids(const UdmGlobalRankidList &globalids);
    bool existsGlobalRankid(const UdmGlobalRankid &globalid) const;
    bool existsGlobalRankid(int rankno, UdmSize_t localid) const;
    void clear();
    UdmError_t removeGlobalRankid(const UdmGlobalRankid &globalid);
    UdmError_t removeGlobalRankid(int rankno, UdmSize_t localid);
    UdmEntity* getReferenceEntity(const int rankno, const UdmSize_t localid) const;
    void removeReferenceEntity(int rankno, UdmSize_t localid, UdmEntity* entity);
    UdmSize_t getNumGlobalRankids() const;
    UdmSize_t size() const;
    UdmError_t getGlobalRankid(UdmSize_t id, int &rankno, UdmSize_t &localid) const;
    UdmError_t getGlobalRankid(UdmSize_t id, UdmGlobalRankid &globalid) const;
    UdmSerializeArchive& serialize(UdmSerializeArchive& archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive& archive);
    void eraseInvalidGlobalRankids(int invalid_rankno);
    UdmError_t updateGlobalRankid(
                    int old_rankno, UdmSize_t old_localid,
                    int new_rankno, UdmSize_t new_localid);
    UdmError_t cloneGlobalRankidList(const UdmGlobalRankidList& src);
    UdmError_t addPreviousRankInfo(int rankno, UdmSize_t localid);
    void reserve(UdmSize_t size);
    bool equals(const UdmGlobalRankidList &dest_list) const;

    // for debug
    void toString(std::string &buf) const;
    // memory size
    size_t getMemSize() const;

private:
    bool findGlobalRankid(
                const UdmGlobalRankid &src_rank,
                std::vector<UdmGlobalRankid>::const_iterator &insert_itr) const;
};

/**
 * グローバルIDペアリストクラス.
 */
class UdmGlobalRankidPairList
{
private:
    /**
     * グローバルIDペアリスト
     */
    std::vector<UdmGlobalRankidPair>  pair_list;

public:
    UdmGlobalRankidPairList();
    UdmGlobalRankidPairList(UdmSize_t size);
    virtual ~UdmGlobalRankidPairList();
    UdmError_t addGlobalRankidPair(const UdmGlobalRankidPair &global_pair);
    UdmError_t addGlobalRankidPair(int src_rankno, UdmSize_t src_localid, int dest_rankno, UdmSize_t dest_localid);
    UdmError_t addGlobalRankidPair(const UdmGlobalRankid &src_globalid, const UdmGlobalRankid &dest_globalid);
    UdmError_t addGlobalRankidPairs(const UdmGlobalRankidPairList &add_list);
    bool existsGlobalRankidPair(const UdmGlobalRankidPair &global_pair) const;
    bool existsGlobalRankidPair(int src_rankno, UdmSize_t src_localid, int dest_rankno, UdmSize_t dest_localid) const;
    bool existsGlobalRankidPair(const UdmGlobalRankid &src_globalid, const UdmGlobalRankid &dest_globalid) const;
    void clear();
    UdmError_t removeGlobalRankidPair(const UdmGlobalRankidPair &global_pair);
    UdmError_t removeGlobalRankidPair(int src_rankno, UdmSize_t src_localid, int dest_rankno, UdmSize_t dest_localid);
    UdmError_t removeGlobalRankidPair(const UdmGlobalRankid &src_globalid, const UdmGlobalRankid &dest_globalid);
    UdmSize_t getNumGlobalRankidPairs() const;
    UdmSize_t size() const;
    UdmError_t getGlobalRankidPair(UdmSize_t id, int &src_rankno, UdmSize_t &src_localid, int &dest_rankno, UdmSize_t &dest_localid) const;
    UdmError_t getGlobalRankidPair(UdmSize_t id, UdmGlobalRankidPair &global_pair) const;
    UdmError_t getGlobalRankidPair(UdmSize_t id, UdmGlobalRankid &src_globalid, UdmGlobalRankid &dest_globalid) const;
    UdmSerializeArchive& serialize(UdmSerializeArchive& archive) const;
    UdmSerializeArchive& deserialize(UdmSerializeArchive& archive);
    void toString(std::string &buf) const;
    std::vector<UdmGlobalRankidPair>::iterator begin();
    std::vector<UdmGlobalRankidPair>::iterator end();
    UdmError_t pushbackGlobalRankidPair(const UdmGlobalRankidPair &global_pair);
    UdmSize_t getCommSize() const;
    void reserve(UdmSize_t size);

private:
    bool findGlobalRankidPair(
                const UdmGlobalRankidPair &src_pair,
                std::vector<UdmGlobalRankidPair>::const_iterator &insert_itr) const;

};

} /* namespace udm */

#endif /* _UDMGLOBALRANKID_H_ */
