#ifndef _CCN_INTEREST_HEADER_H_
#define _CCN_INTEREST_HEADER_H_

#include "ns3/header.h"
#include "ns3/ipv4-address.h"

#include <stdint.h>
#include <string>

namespace ns3
{
/**
 * \ingroup ccn
 * \brief Packet header for CCN Interest packets
*/
class CCNHeader : public Header
{
    public:
        /**
         * \brief Constructor
         * 
         * Creates a null header
         */
        CCNHeader();
        ~CCNHeader() override;

        static const uint32_t m_name_length = 20;
        static const uint32_t m_type_length = 1;

        enum MessageType
        {
            INTEREST = 0,
            DATA = 1
        };

        /**
         * \brief Get the type of the message
         * \return 0 for Interest, 1 for Data
         */
        uint8_t GetMessageType() const;

        /**
         * \brief Set the type of the message
         * \param type 0 for Interest, 1 for Data
         */
        void SetMessageType(uint8_t type);

        /**
         * \brief Set the content name
         */
        void SetContentName(std::string content_name);

        /**
         * \brief Get the content name
         */
        std::string GetContentName() const;

        static TypeId GetTypeId();
        TypeId GetInstanceTypeId() const override;
        void Print(std::ostream& os) const override;
        uint32_t GetSerializedSize() const override;
        void Serialize(Buffer::Iterator start) const override;
        uint32_t Deserialize(Buffer::Iterator start) override;
    
    private:
        std::string m_content_name;
        uint8_t m_type;     // 0: Interest, 1: Data
        uint16_t m_selector;
        uint16_t m_nonce;
};

} // namespace ns3



#endif