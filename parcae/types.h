#ifndef TYPES_H
#define TYPES_H

#undef PARCAE_LOG
//#define PARCAE_LOG(...) printf(__VA_ARGS__)
#define PARCAE_LOG(...) {}

using uint = unsigned int;

/**
 * @brief CThread - поток исполнения
 */
class CThread
{
public:
    /**
     * @brief CThread - конструктор с явной параметризацией
     * @param[in] name - имя потока
     */
    CThread(const std::string &name)
        : m_name(name)
    {

    }
    /**
     * @brief Lock - заблокировать поток
     */
    void Lock() const
    {
        m_running = false;
        m_thread_mutex.lock();
        m_running = true;
    }
    /**
     * @brief Unlock - разблокировать поток
     */
    void Unlock() const
    {
        m_thread_mutex.unlock();
        m_running = true;
    }
    /**
     * @brief IsRunning - предикат выполнения
     * @return выполняется поток или нет
     */
    bool IsRunning() const
    {
        return m_running;
    }
    /**
     * @brief SetReady - установить готовность потока к работе
     * @param[in] ready - готовность потока к работе
     */
    void SetReady(const bool ready) {m_ready = ready;}
    /**
     * @brief IsReady - проверить готовность потока к работе
     * @return готовность потока к работе
     */
    bool IsReady() const {return m_ready;}
    /**
     * @brief Name - получить имя потока
     * @return имя потока
     */
    std::string Name() const {return m_name;}

private:
    bool                m_ready = false;
    std::string         m_name;
    mutable bool        m_running = true;
    mutable std::mutex  m_thread_mutex;
};

/**
 * @brief CThreads - менеджер потоков
 */
class CThreads
{
public:
    /**
     * @brief SetNotReady - установить всем потокам состояние неготовности
     */
    void SetNotReady()
    {
        for (auto &th : m_threads)
            th.second.SetReady(false);
    }
    /**
     * @brief GetThread - получить поток по имени
     * @param[in] thread_name - имя потока
     * @return ссылка на класс потока
     */
    CThread& GetThread(const std::string &thread_name)
    {
        auto th_it = m_threads.find(thread_name);
        if (th_it != m_threads.end())
        {
            return th_it->second;
        }
        else
        {
            m_threads.emplace(thread_name, thread_name);
            return m_threads.find(thread_name)->second;
        }
    }
    /**
     * @brief SetNotReady - установить неготовность потока по имени
     * @param[in] thread_name - имя потока
     */
    void SetNotReady(const std::string &thread_name)
    {
        auto th_it = m_threads.find(thread_name);
        if (th_it != m_threads.end())
        {
            th_it->second.SetReady(false);
        }
    }
    /**
     * @brief SetReady - установить готовность потока по имени
     * @param[in] thread_name - имя потока
     */
    void SetReady(const std::string &thread_name)
    {
        auto th_it = m_threads.find(thread_name);
        if (th_it != m_threads.end())
        {
            th_it->second.SetReady(true);
        }
    }
    /**
     * @brief IsReady - проверить готовность потока по имени
     * @param[in] thread_name - имя потока
     */
    bool IsReady(const std::string &thread_name) const
    {
        auto th_it = m_threads.find(thread_name);
        if (th_it != m_threads.end())
        {
            return th_it->second.IsReady();
        }
        return false;
    }
    /**
     * @brief Clear - очистить список потоков
     */
    void Clear()
    {
        m_threads.clear();
    }
    /**
     * @brief Lock - заблокировать поток
     * @param[in] thread_name - имя потока
     */
    void Lock(const std::string &thread_name)
    {
        PARCAE_LOG("CThreads::Lock %s\n", thread_name.c_str());
        auto th_it = m_threads.find(thread_name);
        if (th_it != m_threads.end())
        {
            th_it->second.Lock();
        }
        else
        {
            PARCAE_LOG("ERROR Lock %s\n", thread_name.c_str());
        }
    }
    /**
     * @brief Unlock - разблокировать поток
     * @param[in] thread_name - имя потока
     */
    void Unlock(const std::string &thread_name)
    {
        PARCAE_LOG("CThreads::Unlock %s\n", thread_name.c_str());
        auto th_it = m_threads.find(thread_name);
        if (th_it != m_threads.end())
        {
            th_it->second.Unlock();
        }
        else
        {
            PARCAE_LOG("ERROR Unlock %s\n", thread_name.c_str());
        }
    }
    /**
     * @brief AllReady - проверить, что все потоки готовы
     * @return все потоки готовы
     */
    bool AllReady() const
    {
        const bool b = std::all_of(m_threads.cbegin(), m_threads.cend(),
                        [](const std::pair<const std::string, CThread> &t) { return t.second.IsReady(); });
        return b;
    }
    /**
     * @brief GetRunning - получить список запущенных потоков
     * @return список запущенных потоков
     */
    std::vector<std::string> GetRunning() const
    {
        std::vector<std::string> threads_running;
        for (const auto &th : m_threads)
        {
            if (th.second.IsRunning())
                threads_running.push_back(th.second.Name());
        }
        return threads_running;
    }
    /**
     * @brief GetReady получить список готовых к работе потоков
     * @return список готовых к работе потоков
     */
    std::vector<std::string> GetReady() const
    {
        std::vector<std::string> threads_ready;
        for (const auto &th : m_threads)
        {
            if (th.second.IsReady())
                threads_ready.push_back(th.second.Name());
        }
        return threads_ready;
    }

private:
    std::unordered_map<std::string, CThread>    m_threads;
};



#endif // TYPES_H
