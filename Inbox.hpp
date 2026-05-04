// Inbox.hpp
#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "json.hpp"
using json = nlohmann::json;

enum class InboxMessageType {
    TRANSFER_OFFER,
    TRANSFER_ACCEPTED,
    TRANSFER_REJECTED,
    CONTRACT_EXPIRY,
    ACADEMY_INTAKE,
    INJURY_REPORT,
    BOARD_FEEDBACK,
    COMPETITION_DRAW,
    AWARD_NOMINATION,
    SCOUT_REPORT_READY,
    MATCH_REPORT,
    GENERAL
};

struct InboxMessage {
    InboxMessageType type;
    std::string title;
    std::string body;
    std::string date;          // "YYYY-MM-DD"
    bool read = false;
    int id = 0;                // unique message ID
};

class Inbox {
private:
    std::vector<InboxMessage> m_messages;
    int m_nextId = 1;

public:
    void addMessage(InboxMessageType type, const std::string& title,
                    const std::string& body, const std::string& date);
    void markAsRead(int id);
    void markAllRead();
    void deleteMessage(int id);
    std::vector<InboxMessage> getUnreadMessages() const;
    std::vector<InboxMessage> getAllMessages() const;
    std::vector<InboxMessage> getMessagesByType(InboxMessageType type) const;
    int getUnreadCount() const;

    json toJson() const;
    void fromJson(const json& j);
};