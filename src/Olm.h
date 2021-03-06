#pragma once

#include <boost/optional.hpp>

#include <memory>
#include <mtx.hpp>
#include <mtxclient/crypto/client.hpp>

constexpr auto OLM_ALGO = "m.olm.v1.curve25519-aes-sha2";

namespace olm {

struct OlmMessage
{
        std::string sender_key;
        std::string sender;

        using RecipientKey = std::string;
        std::map<RecipientKey, mtx::events::msg::OlmCipherContent> ciphertext;
};

inline void
from_json(const nlohmann::json &obj, OlmMessage &msg)
{
        if (obj.at("type") != "m.room.encrypted")
                throw std::invalid_argument("invalid type for olm message");

        if (obj.at("content").at("algorithm") != OLM_ALGO)
                throw std::invalid_argument("invalid algorithm for olm message");

        msg.sender     = obj.at("sender");
        msg.sender_key = obj.at("content").at("sender_key");
        msg.ciphertext = obj.at("content")
                           .at("ciphertext")
                           .get<std::map<std::string, mtx::events::msg::OlmCipherContent>>();
}

mtx::crypto::OlmClient *
client();

void
handle_to_device_messages(const std::vector<nlohmann::json> &msgs);

nlohmann::json
try_olm_decryption(const std::string &sender_key,
                   const mtx::events::msg::OlmCipherContent &content);

void
handle_olm_message(const OlmMessage &msg);

//! Establish a new inbound megolm session with the decrypted payload from olm.
void
create_inbound_megolm_session(const std::string &sender,
                              const std::string &sender_key,
                              const nlohmann::json &payload);

void
handle_pre_key_olm_message(const std::string &sender,
                           const std::string &sender_key,
                           const mtx::events::msg::OlmCipherContent &content);

mtx::events::msg::Encrypted
encrypt_group_message(const std::string &room_id,
                      const std::string &device_id,
                      const std::string &body);

void
mark_keys_as_published();

//! Request the encryption keys from sender's device for the given event.
void
request_keys(const std::string &room_id, const std::string &event_id);

void
send_key_request_for(const std::string &room_id,
                     const mtx::events::EncryptedEvent<mtx::events::msg::Encrypted> &);

void
handle_key_request_message(const mtx::events::msg::KeyRequest &);

void
send_megolm_key_to_device(const std::string &user_id,
                          const std::string &device_id,
                          const json &payload);

} // namespace olm
