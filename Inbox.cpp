// Inbox.cpp
#include "Inbox.hpp"
#include <algorithm>

void Inbox::addMessage(InboxMessageType type, const std::string& title,
                       const std::string& body, const std::string& date) {
    InboxMessage msg;
    msg.type = type;
    msg.title = title;
    msg.body = body;
    msg.date = date;
    msg.read = false;
    msg.id = m_nextId++;
    m_messages.push_back(msg);
}

void Inbox::markAsRead(int id) {
    for (auto& msg : m_messages) {
        if (msg.id == id) {
            msg.read = true;
            return;
        }
    }
}

void Inbox::markAllRead() {
    for (auto& msg : m_messages) msg.read = true;
}

void Inbox::deleteMessage(int id) {
    m_messages.erase(
        std::remove_if(m_messages.begin(), m_messages.end(),
                       [id](const InboxMessage& m) { return m.id == id; }),
        m_messages.end());
}

std::vector<InboxMessage> Inbox::getUnreadMessages() const {
    std::vector<InboxMessage> unread;
    for (const auto& msg : m_messages)
        if (!msg.read) unread.push_back(msg);
    return unread;
}

std::vector<InboxMessage> Inbox::getAllMessages() const {
    return m_messages;
}

std::vector<InboxMessage> Inbox::getMessagesByType(InboxMessageType type) const {
    std::vector<InboxMessage> result;
    for (const auto& msg : m_messages)
        if (msg.type == type) result.push_back(msg);
    return result;
}

int Inbox::getUnreadCount() const {
    int count = 0;
    for (const auto& msg : m_messages)
        if (!msg.read) count++;
    return count;
}

json Inbox::toJson() const {
    json arr = json::array();
    for (const auto& msg : m_messages) {
        arr.push_back({
            {"type", static_cast<int>(msg.type)},
            {"title", msg.title},
            {"body", msg.body},
            {"date", msg.date},
            {"read", msg.read},
            {"id", msg.id}
        });
    }
    json j;
    j["messages"] = arr;
    j["nextId"] = m_nextId;
    return j;
}

void Inbox::fromJson(const json& j) {
    m_messages.clear();
    m_nextId = j.value("nextId", 1);
    if (j.contains("messages") && j["messages"].is_array()) {
        for (const auto& mj : j["messages"]) {
            InboxMessage msg;
            msg.type = static_cast<InboxMessageType>(mj.value("type", 0));
            msg.title = mj.value("title", "");
            msg.body = mj.value("body", "");
            msg.date = mj.value("date", "");
            msg.read = mj.value("read", false);
            msg.id = mj.value("id", 0);
            m_messages.push_back(msg);
        }
    }
}