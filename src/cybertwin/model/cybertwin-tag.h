#ifndef CYBERTWIN_TAG_H
#define CYBERTWIN_TAG_H

#include "cybertwin-common.h"

#include "ns3/header.h"
#include "ns3/tag.h"

namespace ns3
{

class CybertwinTag : public Tag
{
  public:
    CybertwinTag(CYBERTWINID_t cuid = 0);
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer) const override;
    void Deserialize(TagBuffer) override;
    void Print(std::ostream&) const override;
    std::string ToString() const;

    void SetCybertwin(CYBERTWINID_t);
    CYBERTWINID_t GetCybertwin() const;

  protected:
    CYBERTWINID_t m_cuid;
};

class CybertwinCreditTag : public CybertwinTag
{
  public:
    CybertwinCreditTag(CYBERTWINID_t cuid = 0, uint16_t credit = 0, CYBERTWINID_t peer = 0);
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer) const override;
    void Deserialize(TagBuffer) override;
    void Print(std::ostream&) const override;

    void SetCredit(uint16_t);
    uint16_t GetCredit() const;
    void SetPeer(CYBERTWINID_t);
    CYBERTWINID_t GetPeer() const;

  private:
    uint16_t m_credit;
    CYBERTWINID_t m_peer;
};

class CybertwinCertTag : public CybertwinTag
{
  public:
    CybertwinCertTag(CYBERTWINID_t cuid = 0,
                     uint16_t initialCredit = 0,
                     uint16_t ingressCredit = 0,
                     bool isUserRequired = false,
                     bool isCertValid = false,
                     CYBERTWINID_t m_usr = 0,
                     uint16_t m_usrInitialCredit = 0);
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer) const override;
    void Deserialize(TagBuffer) override;
    void Print(std::ostream&) const override;

    void SetInitialCredit(uint16_t);
    uint16_t GetInitialCredit() const;
    void SetIngressCredit(uint16_t);
    uint16_t GetIngressCredit() const;
    void SetIsUserRequired(bool);
    bool GetIsUserRequired() const;
    void SetIsValid(bool);
    bool GetIsValid() const;
    CYBERTWINID_t GetUser() const;
    uint16_t GetUserInitialCredit() const;

  private:
    uint16_t m_initialCredit;
    uint16_t m_ingressCredit;
    bool m_isUserRequired;
    bool m_isCertValid;

    CYBERTWINID_t m_usr;
    uint16_t m_usrInitialCredit;
};

class MultipathTagConn : public ns3::Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;
  
  void SetPathId (uint32_t pathId);
  void SetCuid (uint64_t cuid);
  void SetSenderKey (uint32_t senderKey);
  void SetRecverKey (uint32_t recverKey);
  void SetConnId (uint64_t connId);

  uint32_t GetPathId (void) const;
  uint64_t GetCuid (void) const;
  uint32_t GetSenderKey (void) const;
  uint32_t GetRecverKey (void) const;
  uint64_t GetConnId (void) const;

private:
  uint32_t m_pathId;
  uint64_t m_cuid;
  uint32_t m_senderKey;
  uint32_t m_recverKey;
  uint64_t m_connId;
};

} // namespace ns3

#endif