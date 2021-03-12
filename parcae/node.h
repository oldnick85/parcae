#ifndef NODE_H
#define NODE_H

#include <memory>
#include <unordered_set>
#include <cassert>
#include <vector>

#include "types.h"

class CParcaeNode;
using CParcaeNodePtr = std::shared_ptr<CParcaeNode>;
/**
 * @brief CParcaeNode - узел в дереве выполнения
 */
class CParcaeNode
{
public:
    CParcaeNode() = default;
    /**
     * @brief CParcaeNode - конструктор с явной параметризацией
     * @param[in] thread_name - имя потока
     * @param[in] m - номер этапа
     * @param[in] threads_ready - список имён готовых к работе потоков на данный момент
     */
    CParcaeNode(const std::string &thread_name, const uint m, const std::vector<std::string> &threads_ready)
        : m_thread_name(thread_name)
        , m_milestone(m)
        , m_threads_ready(threads_ready)
    {

    }
    /**
     * @brief CheckDeadEnd - проверить узел на наличие альтернатив
     * @remark По итогу проверки будет установлен флаг в классе
     */
    void CheckDeadEnd()
    {
        PARCAE_LOG("%s %s\n", __FUNCTION__, Print().c_str());
        bool all_dead_end = true;
        for (const auto &th_name : m_threads_ready)
        {
            if (const auto &next = FindNext(th_name))
            {
                if (not next->IsDeadEnd())
                {
                    all_dead_end = false;
                    break;
                }
            }
            else
            {
                all_dead_end = false;
                break;
            }
        }
        if (all_dead_end)
            SetDeadEnd();

        if ((not IsRoot()) and (m_dead_end))
            m_prev->CheckDeadEnd();
    }
    /**
     * @brief SetReadyThreads - установить список потоков, готовых к работе
     * @param threads_ready - список потоков, готовых к работе
     */
    void SetReadyThreads(const std::vector<std::string> &threads_ready) {m_threads_ready = threads_ready;}
    /**
     * @brief SetDeadEnd - установить узел как безальтернативный
     */
    void SetDeadEnd()
    {
        PARCAE_LOG("%s %s\n", __FUNCTION__, Print().c_str());
        m_dead_end = true;
    }
    /**
     * @brief IsDeadEnd - проверить узел на безальтернативность
     * @return безальтернативность узла
     */
    bool IsDeadEnd() const {return m_dead_end;}
    /**
     * @brief HookOn - привязать к предку
     * @param prev - предыдущий узел
     */
    void HookOn(CParcaeNodePtr prev)
    {
        if (not prev)
            return;
        m_prev = prev;
    }
    /**
     * @brief FindNext - найти потомка с указанным именем потока и номером этапа
     * @param[in] thread_name - имя потока
     * @param[in] milestone - номер этапа
     * @return потомок с указанным именем потока и номером этапа или nullptr, если не найден
     */
    CParcaeNodePtr FindNext(const std::string thread_name, const uint milestone) const
    {
        for (const auto p : m_next)
        {
            if ((p->m_thread_name == thread_name) and (p->m_milestone == milestone))
                return p;
        }
        return nullptr;
    }
    /**
     * @brief FindNext - найти потомка с указанным именем потока
     * @param[in] thread_name - имя потока
     * @return потомок с указанным именем потока или nullptr, если не найден
     */
    CParcaeNodePtr FindNext(const std::string thread_name) const
    {
        for (const auto &p : m_next)
        {
            if (p->m_thread_name == thread_name)
                return p;
        }
        return nullptr;
    }
    /**
     * @brief AddNext - добавить потомка
     * @param next - следующий узел
     */
    void AddNext(CParcaeNodePtr next)
    {
        if (not next)
            return;
        m_next.emplace(next);
    }
    /**
     * @brief Clear - очистить все привязки этого узла и всех последующих
     * @remark В результате все потомки удалятся, если на них больше нет ссылок
     */
    void Clear()
    {
        m_prev = nullptr;
        for (auto &next : m_next)
        {
            next->Clear();
        }
        m_next.clear();
    }
    /**
     * @brief IsRoot - проверить корень это или нет
     * @return является ли узел корневым
     */
    bool IsRoot() const {return (m_prev == nullptr);}
    /**
     * @brief IsEnd - проверить конец это или нет
     * @return является ли узел конечным
     */
    bool IsEnd() const {return (m_next.empty());}
    /**
     * @brief PrintShort - получить краткое строковое описание узла
     * @return краткое строковое описание узла
     */
    std::string PrintShort() const
    {
        std::string milestone_str;
        if (IsRoot())
        {
            milestone_str = "ROOT";
        }
        else
        {
            char s[128];
            snprintf(s, sizeof(s), "%s:%u", m_thread_name.c_str(), m_milestone);
            milestone_str = s;
        }
        return (std::string("-") + (m_dead_end ? "[" : "") + milestone_str + (m_dead_end ? "]" : ""));
    }
    /**
     * @brief Print - получить строковое описание узла
     * @return строковое описание узла
     */
    std::string Print() const
    {
        if (IsRoot())
            return "ROOT";
        char s[128];
        snprintf(s, sizeof(s), "%s%s:%u%s", m_dead_end ? "[" : "", m_thread_name.c_str(), m_milestone, m_dead_end ? "]" : "");
        std::string str = s;
        for (const auto &th_run : m_threads_ready)
        {
            str += "+";
            str += th_run;
        }
        return str;
    }
    /**
     * @brief PrintPrevious - получить строковое представление восходящей цепочки
     * @return строковое представление восходящей цепочки
     */
    std::string PrintPrevious() const
    {
        std::string str;
        if (m_prev)
            str = m_prev->PrintPrevious();
        str += PrintShort();
        return str;
    }
    /**
     * @brief PrintTree - получить строковое представление дерева в формате JSON
     * @return строковое представление дерева в формате JSON
     */
    std::string PrintTree() const
    {
        if (m_next.empty())
            return "{" + PrintJSON() + "}";
        std::string str;
        str = "{";
        str += PrintJSON();
        if (not m_next.empty())
        {
            str += ", \"next\" : [";
            std::string delimeter = "";
            for (const auto &next : m_next)
            {
                str += delimeter;
                str += next->PrintTree();
                delimeter = ",";
            }
            str += "]";
        }
        str += "}";
        return str;
    }
    /**
     * @brief PrintDOT - получить строковое представление дерева в формате DOT
     * @return строковое представление дерева в формате DOT
     */
    std::string PrintDOT() const
    {
        std::string str_vertexes;
        std::string str_edges;
        PrintDOT(str_vertexes, str_edges, 0);
        return "digraph G {\n" + str_vertexes + str_edges + "\n}";
    }
    /**
     * @brief ThreadName - получить имя потока
     * @return имя потока
     */
    std::string ThreadName() const
    {
        return m_thread_name;
    }
private:
    std::string PrintJSON() const
    {
        char s[128];
        snprintf(s, sizeof(s), "\"thread\" : \"%s\", \"milestone\" : %u, \"dead_end\" : \"%s\"",
                 m_thread_name.c_str(), m_milestone, m_dead_end ? "true" : "false");
        return s;
    }

    std::string PrintDOT_Vertex(const uint level) const
    {
        if (IsRoot())
            return "ROOT";
        char str[256];
        snprintf(str, sizeof(str), "%s%uL%u", m_thread_name.c_str(), m_milestone, level);
        return str;
    }

    std::string PrintDOT_Vertex() const
    {
        if (IsRoot())
            return "ROOT";
        char str[256];
        snprintf(str, sizeof(str), "%s%u", m_thread_name.c_str(), m_milestone);
        return str;
    }

    void PrintDOT(std::string &str_vertexes, std::string &str_edges, const uint level) const
    {
        str_vertexes += PrintDOT_Vertex(level);
        str_vertexes += " [";
        str_vertexes += std::string("shape=") + (m_dead_end ? "box" : "diamond");
        str_vertexes += ", label=\"" + PrintDOT_Vertex() + "\"";
        str_vertexes += "]\n";
        for (const auto &next : m_next)
            str_edges += PrintDOT_Vertex(level) + " -> " + next->PrintDOT_Vertex(level+1) + "\n";
        for (const auto &next : m_next)
            next->PrintDOT(str_vertexes, str_edges, level+1);
    }

    using NextNodes_t = std::unordered_set<std::shared_ptr<CParcaeNode>>;
    NextNodes_t         m_next;
    CParcaeNodePtr      m_prev = nullptr;
    std::string         m_thread_name;
    uint                m_milestone = 0;
    bool                m_dead_end = false;
    std::vector<std::string> m_threads_ready;
};

#endif // NODE_H
